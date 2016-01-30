#include "bootpack.h"
#include <stdio.h>
#include <string.h>

static SuperFS* superfs;


int test_find_inode(const char *filename) {
  struct one_inode *inode = get_inode_by_path(filename);

  return inode->ino;
}

// unsigned char *test_find_block(const char *filename) {
//   struct one_inode *inode = get_inode_by_path(filename);
//
//   return inode->entry->file.block_group->blocks[0]->block_data;
// }
unsigned char *test_find_block(FileDescriptor *fd) {
  struct one_inode *inode = fd->inode;

  return inode->entry->file.block_group->blocks[0]->block_data;
}

unsigned int test_find_blockno(const char *filename) {
  struct one_inode *inode = get_inode_by_path(filename);

  return inode->entry->file.block_group->blocks[0]->block_no;
}

struct one_inode *test_get_root_inode() {
  return superfs->root_inode;
}


/**
 * @brief "create" procedure for block device
 *
 * @param dev_no (0)
 * @param block_count: fs size become (block_count * BLOCK_DATA_LEN) bytes
 * @return 0: success, error if others
 * @result created SuperFS is assigned to `superfs` (statically defined in fs.c)
 */
int make_superfs(unsigned char dev_no, unsigned int block_count) {
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  unsigned int i;

  superfs = (SuperFS *)memman_alloc(memman, sizeof(SuperFS));
  // NOTE: block buffer の初期化は不要
  superfs->dev_no = dev_no;

  superfs->master_block_buffer = (struct Block *)memman_alloc(memman, sizeof(struct Block));
  if (superfs->master_block_buffer == NULL) {
    return 1; // error
  }

  superfs->master_block_buffer->block_no  = 0; // first block id
  superfs->master_block_buffer->loaded    = 0;
  superfs->master_block_buffer->free      = 0;
  superfs->master_block_buffer->dirty     = 0;
  superfs->master_block_buffer->prev      = NULL;
  superfs->master_block_buffer->next      = NULL;

  superfs->root_inode = (struct one_inode *)memman_alloc(memman, sizeof(struct one_inode));
  if (superfs->root_inode == NULL) {
    return 2; // error
  }

  superfs->root_inode->size  = 0;
  superfs->root_inode->mode  = ONE_INODE_TYPE_DIR;
  superfs->root_inode->entry = (FSEntry *)memman_alloc(memman, sizeof(FSEntry));
  if (superfs->root_inode->entry == NULL) {
    return 3; // error
  }

  for (i = 1; i < DIR_MAX_FILES_NUM; i++) {
    superfs->root_inode->entry->dir.d_files[i] = ONE_INODE_INO_INVALID; // initial value (== nothing)
  }

  superfs->root_inode->ino        = 0; // root ino
  superfs->root_inode->prev  = NULL;
  superfs->root_inode->next  = NULL;
  strcpy(superfs->root_inode->entry->common.name, "/");

  superfs->root_direntry = &(superfs->root_inode->entry->dir);
  superfs->root_direntry->d_files[0] = 0; // root itself

  strcpy(superfs->root_direntry->d_name, "/");

  // disk の zerofill
  unsigned char *data = (unsigned char *)memman_alloc(memman, SECTOR_SIZE);
  if (data == NULL) {
    return 4; // error
  }
  memset(data, 0, SECTOR_SIZE);

  unsigned int status;
  // disk を 0 - block_count * BLOCK_DATA_LEN まで zerofill する
  for (i = 0; i < block_count * BLOCK_DATA_LEN_SECTORS; i++) {
    if ((status = ide_write(i, 1, data))) {
      return 100 + status; // error: failed data while writing to disk
    }
  }

  return 0;
}


/**
 * @brief "create" procedure for block device
 *
 * @param abs_path
 * @param is_file: 1 if File, 0 if Directory
 * @return 0: success, error if others
 */
int syscall_create(char *abs_path, unsigned char is_file, unsigned int *return_ino) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  unsigned int ino = get_unused_one_inode_number(0);

  *return_ino = ONE_INODE_INO_INVALID;

  if (ino == ONE_INODE_INO_INVALID) {
    // inode number に空きがない
    return 1; // error
  }


  char *name    = get_filename(abs_path);
  if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) {
    return 7; // error: user can't make special file
  }

  char *dirname = get_dirname(abs_path);

  struct one_inode *parent_inode = get_inode_by_path(dirname);

  if (parent_inode == NULL) {
    return 4; // error: parent one-inode is not found
  }

  if (!(parent_inode->mode & ONE_INODE_TYPE_DIR)) {
    return 5; // error: parent one-inode が Directory でない
  }

  if (parent_inode->entry->dir.d_files[DIR_MAX_FILES_NUM-1] != ONE_INODE_INO_INVALID) {
    return 6; // error: parent one-inode の DirEntry の d_files に空きがない
  }

  struct one_inode *new_inode = (struct one_inode *)memman_alloc(memman, sizeof(struct one_inode));

  if (new_inode == NULL) {
    // one_inode を保存するメモリの確保に失敗
    return 2; // error
  }

  new_inode->ino = ino;
  new_inode->size = 0;
  new_inode->mode = is_file ? 0 : 0x8000;

  new_inode->entry = (FSEntry *)memman_alloc(memman, sizeof(FSEntry));
  if (new_inode->entry == NULL) {
    return 3; // error
  }

  new_inode->entry->common.ino = ino;
  strcpy(new_inode->entry->common.name, name);

  if (is_file) {
    new_inode->entry->file.parent_ino = parent_inode->ino;
    new_inode->entry->file.links_count = 1;
  } else { // Directory
    new_inode->entry->dir.d_files[0] = parent_inode->ino; // TODO 0 is root_inode inono.

    int i;
    for (i = 1; i < DIR_MAX_FILES_NUM; i++) {
      new_inode->entry->dir.d_files[i] = ONE_INODE_INO_INVALID; // initial value (== nothing)
    }
  }

  // new_inode を root_inode->next->... によって辿ることができるリストに追加
  struct one_inode *prev_inode = get_prev_inode_by_no(new_inode->ino);
  new_inode->next = prev_inode->next;
  new_inode->prev = prev_inode;
  prev_inode->next->prev = new_inode;
  prev_inode->next       = new_inode;

  // parent one-inode の d_files に new_inode->inode を追加
  int i = 1;
  do {
    if (parent_inode->entry->dir.d_files[i] == ONE_INODE_INO_INVALID) {
      parent_inode->entry->dir.d_files[i] = new_inode->ino;
      break;
    }
  } while (++i < DIR_MAX_FILES_NUM);

  *return_ino = new_inode->ino;

  return 0;
}


/**
* @brief get `previous` inode by inode no
* @param ino inode no
* @return one_inode
*/
struct one_inode* get_prev_inode_by_no(unsigned int ino) {
    struct one_inode* inode = superfs->root_inode;

    while(inode->next && inode->next->ino < ino) {
        inode = inode->next;
    }

    return inode;
}


/**
* @brief get `previous` block by block no
* @param block_no: block no
* @return block
*/
Block* get_prev_block_by_no(unsigned int block_no) {
    Block* block = superfs->master_block_buffer;

    while(block->next && block->next->block_no < block_no) {
        block = block->next;
    }

    return block;
}


/**
 * @brief "remove" file or directory located `abs_path`
 * @param abs_path
 * @param is_recursive: 1 if recursive, 0 if not recursive
 * @return 0: success, error if others
 *
 * @note abs_path なファイルやディレクトリをブロックごと削除する
 *       is_recursize: 0 のときにabs_pathがディレクトリを指すときはエラー
 */
int syscall_remove(char *abs_path, unsigned char is_recursive) {

  struct one_inode *inode = get_inode_by_path(abs_path);
  if (inode == NULL) {
    return 1; // error: one-inode not found
  }

  unsigned int parent_ino;
  if (inode->mode & ONE_INODE_TYPE_DIR) {
    parent_ino = inode->entry->dir.d_files[0];
  } else {
    parent_ino = inode->entry->file.parent_ino;
  }

  struct one_inode *parent_inode = get_inode_by_no(parent_ino);
  if (parent_inode == NULL) {
    return 2; // error: parent one-inode not found
  } else if (!(parent_inode->mode & ONE_INODE_TYPE_DIR)) {
    return 3; // error: parent is not directory
  }

  unsigned int status, ino = inode->ino;
  if((status = remove_fsentry_by_ino(ino, is_recursive))) {
    return status;
  }

  // d_files[] から `ino` を削除する (前に詰める)
  unsigned int i = 1;
  do {
    if (parent_inode->entry->dir.d_files[i] == ino) {
      break;
    }
  } while(++i < DIR_MAX_FILES_NUM);

  if (i == DIR_MAX_FILES_NUM) {
    return 4; // parent one-inode's d_files doesn't include `ino`
  }

  do {
    parent_inode->entry->dir.d_files[i] = parent_inode->entry->dir.d_files[i+1];
  } while(++i < DIR_MAX_FILES_NUM-1);

  parent_inode->entry->dir.d_files[DIR_MAX_FILES_NUM-1] = ONE_INODE_INO_INVALID;

  return 0;
}

/**
 * @brief "remove" file or directory of one-inode(`ino`)
 * @param ino
 * @param is_recursive: 1 if recursive, 0 if not recursive
 * @return 0: success, error if others
 *
 * @note ino なファイルやディレクトリをブロックごと削除する
 *       is_recursive: 0 のときに ino が指すone_inodeがディレクトリの場合はエラー
 */
int remove_fsentry_by_ino(unsigned int ino, unsigned char is_recursive) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  unsigned int i, status;

  struct one_inode *inode = get_inode_by_no(ino);
  if (inode == NULL) {
    return 1; // error: one-inode not found
  }

  if (inode->mode & ONE_INODE_TYPE_DIR) { // directory
    if (!is_recursive) {
      return 4;
    }

    for (i = 1; i < DIR_MAX_FILES_NUM; i++) {
      if (inode->entry->dir.d_files[i] == ONE_INODE_INO_INVALID) {
        break;
      }

      if ((status = remove_fsentry_by_ino(inode->entry->dir.d_files[i], is_recursive))) {
        return status;
      }
    }
  } else { // file
    if ((status = free_block_group(inode->entry->file.block_group))) {
      return status;
    }
  }

  memman_free(memman, inode->entry, sizeof(FSEntry));

  if (inode->prev == NULL) {
    return 5; // error: prev one-inode not found
  }

  if (inode->next) {
    inode->prev->next = inode->next;
    inode->next->prev = inode->prev;
  } else {
    inode->prev->next = NULL;
  }

  memman_free(memman, inode, sizeof(struct one_inode));

  return 0;
}


/**
 * @brief create FileDescriptor and returns
 * @param abs_path: absolute path
 * @param mode: FS_MODE_*
 */
FileDescriptor *syscall_open(char *abs_path, unsigned int mode) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct one_inode *inode = get_inode_by_path(abs_path);
  if (inode == NULL) {
    return NULL;
  }
  FileDescriptor *fd = (FileDescriptor *)memman_alloc(memman, sizeof(FileDescriptor));

  fd->inode = inode;
  fd->mode = mode;
  fd->pos = 0;

  return fd;
}


/**
 * @brief close FileDescriptor and return status
 * @param fd: FileDescriptor
 */
int syscall_close(FileDescriptor *fd) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

  if (memman_free(memman, fd, sizeof(FileDescriptor)) != 0) {
    return 1; // ERROR
  }

  return 0;
}



/**
 * @brief write data using FileDescriptor
 * @param fd: FileDescriptor
 * @param bytes: number of bytes to write
 * @param src: source data (must bigger than `bytes`)
 */
// int syscall_write(char *abs_path, unsigned char *data, unsigned int data_size, unsigned char is_overwrite, FileEntry **return_file_entry) {
int syscall_write(FileDescriptor *fd, int bytes, char *src) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  unsigned int i;
  int status;

  // struct one_inode *inode = get_inode_by_path(abs_path);
  struct one_inode *inode = fd->inode;
  if (inode->mode & ONE_INODE_TYPE_DIR) {
    return 1; // `abs_path` points directory
  }

  if (bytes == 0) {
    return 0; // nothing to write
  }

  // 必要な Block の数の計算
  unsigned int block_count = bytes / BLOCK_DATA_LEN;
  if ((bytes % BLOCK_DATA_LEN) != 0) {
    block_count++;
  }


  // 必要な数 (block_count) の block_no を確保できるか確認
  {
    unsigned int block_no = 0;
    for (i = 0; i < block_count; i++) {
      block_no = get_unused_blockno(block_no);
      if (block_no == BLOCK_NO_INVALID) {
        return 2; // error: insufficient block-no space
      }
    }
  }

  BlockGroup *first_block_group = (BlockGroup *)memman_alloc(memman, sizeof(BlockGroup));
  if (first_block_group == NULL) {
    return 3; // error: failed to alloc block group
  }

  // 現在の書き込み対象の block group
  BlockGroup *block_group = first_block_group;
  // initialize block_group->(blocks[1..BLOCK_GROUP_BLOCKS_NUM-1], next)
  for (i = 0; i < BLOCK_GROUP_BLOCKS_NUM; i++) {
    block_group->blocks[i] = NULL;
  }
  block_group->next = NULL;

  unsigned int current_block = 0, block_group_idx = 0, block_count_current = 0;

  do {
    // block group が埋まっていた場合、新しい block group を確保する
    if (block_group_idx == BLOCK_GROUP_BLOCKS_NUM) {
      BlockGroup *new_block_group = (BlockGroup *)memman_alloc(memman, sizeof(BlockGroup));
      if (new_block_group == NULL) {
        // 今までに確保していた block group / block を開放
        free_block_group(first_block_group);

        return 3; // error: failed to alloc block group
      }

      block_group->next = new_block_group;
      block_group = new_block_group;

      // initialize block_group->(blocks[1..BLOCK_GROUP_BLOCKS_NUM-1], next)
      for (i = 0; i < BLOCK_GROUP_BLOCKS_NUM; i++) {
        block_group->blocks[i] = NULL;
      }
      block_group->next = NULL;
      block_group_idx = 0;
    }

    unsigned int block_no = get_unused_blockno(0);
    if (block_no == BLOCK_NO_INVALID) {
      return 2;
    }

    Block *new_block = (Block *)memman_alloc(memman, sizeof(Block));

    if (new_block == NULL) {
      return 4; // error: failed to alloc block
    }

    // block->(next, prev) の更新
    Block *prev_block = get_prev_block_by_no(block_no);
    if (prev_block == NULL) {
      return 6; // error: prev-block not found (unexpected)
    }

    if (prev_block->next) {
      new_block->next = prev_block->next;
      new_block->next->prev = new_block;
    }

    prev_block->next = new_block;
    new_block->prev  = prev_block;

    // data の書き込み
    new_block->block_data = (unsigned char *)memman_alloc(memman, BLOCK_DATA_LEN);

    if (new_block->block_data == NULL) {
      return 5; // error: failed to alloc block_data
    }

    memset(new_block->block_data, 0, BLOCK_DATA_LEN); // zerofill
    unsigned int size = bytes - (current_block * BLOCK_DATA_LEN);
    if (BLOCK_DATA_LEN < size) {
      size = BLOCK_DATA_LEN;
    }

    memcpy(new_block->block_data, &src[current_block++ * BLOCK_DATA_LEN], size);

    new_block->block_no = block_no;
    new_block->free     = 0;
    new_block->loaded   = 1;
    new_block->dirty    = 1;

    // HDD上のアドレス（block_no * BLOCK_DATA_LEN_SECTORS）にdataを BLOCK_DATA_LEN_SECTORS セクタ分 書き込む
    if ((status = ide_write(block_no * BLOCK_DATA_LEN_SECTORS, BLOCK_DATA_LEN_SECTORS, new_block->block_data))) {
      return 100 + status; // error: failed data while writing to disk
    }

    memman_free(memman, new_block->block_data, BLOCK_DATA_LEN);
    new_block->block_data = NULL;
    new_block->free   = 0;
    new_block->loaded = 0;
    new_block->dirty  = 0;

    // block_group の更新
    block_group->blocks[block_group_idx++] = new_block;
  } while (++block_count_current < block_count);


  inode->entry->file.block_group = first_block_group;

  return 0;
}


/**
 * @brief "read" file located `abs_path` and store into block(s)
 *
 * @param abs_path
 * @param size: size bytes
 * @param offset: offset bytes
 * @param *return_file_entry: store (FileEntry *) of `abs_path` if success
 * @return 0: success, error if others
 */
// NOTE: 1ブロック以上読み込んだ場合の動作は未確認
// int syscall_read(char *abs_path, unsigned int size, unsigned int offset, FileEntry **return_file_entry) {
int syscall_read(FileDescriptor *fd, int bytes, int offset, char *result) {
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  unsigned int block_no;
  int status;

  if (bytes == 0) {
    return 0; // nothing to read, return
  }

  // struct one_inode *inode = get_inode_by_path(abs_path);
  struct one_inode *inode = fd->inode;
  if (inode == NULL) {
    return 1; // error
  }

  if (inode->mode & ONE_INODE_TYPE_DIR) {
    return 2; // `abs_path` points directory
  }

  // _idx: block_group の中で何番目に位置しているか
  unsigned int first_block_idx = 0, last_block_idx;
  if (offset != 0) {
    first_block_idx = offset / BLOCK_DATA_LEN;
  }

  last_block_idx = (bytes + offset) / BLOCK_DATA_LEN;

  unsigned int block_group_idx = 0;
  BlockGroup *block_group = inode->entry->file.block_group;

  if (block_group == NULL) {
    return 3; // error
  }

  // first_block_idx を含む block_group まで next をたどる
  while ((block_group_idx++ + 1) * BLOCK_GROUP_BLOCKS_NUM - 1 < first_block_idx) {
    block_group = block_group->next;

    if (block_group == NULL) {
      return 4; // error
    }
  }

  unsigned int block_idx = first_block_idx;
  do {
    unsigned char *data = (unsigned char *)memman_alloc(memman, BLOCK_DATA_LEN);
    if (data == NULL) {
      return 6; // error
    }

    Block *block = inode->entry->file.block_group->blocks[block_idx % 16];
    if (block == NULL) {
      return 7; // unexpected error
    }

    block_no = block->block_no;

    // HDD上のアドレス（block_no * BLOCK_DATA_LEN_SECTORS）からdataを BLOCK_DATA_LEN_SECTORS セクタ分 読み込む
    if ((status = ide_read(block_no * BLOCK_DATA_LEN_SECTORS, BLOCK_DATA_LEN_SECTORS, data))) {
      return 100 + status; // error
    }

    block->block_data = data;
    block->loaded = 1;
    block->free   = 0;
    block->dirty  = 0;

    if (++block_idx <= last_block_idx) {
      if ((block_idx % BLOCK_GROUP_BLOCKS_NUM) == 0) {
        // 次の Block が 次の BlockGroup にあるとき
        block_group = block_group->next;

        if (block_group == NULL) {
          return 5; // error
        }
      }
    }
  } while (block_idx <= last_block_idx);

  return 0;
}


/**
 * @brief "get" array of (FSEntry *) located `abs_path`
 *
 * @param abs_path
 * @param return_entry: head address of (array of (FSEntry *))
 * @param return_size: length of (array of (FSEntry *))
 * @return 0: success, error if others
 */
int syscall_get_fsentry(FileDescriptor *fd, FSEntry **return_entry, int *return_size) {
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

  *return_entry = NULL;

  // struct one_inode *inode = get_inode_by_path(abs_path);
  struct one_inode *inode = fd->inode;
  if (inode == NULL) {
    return 1; // error: one-inode not found
  }

  // file
  if (!(inode->mode & ONE_INODE_TYPE_DIR)) {
    *return_entry = (FSEntry **)memman_alloc(memman, sizeof(FSEntry *));

    if (return_entry == NULL) {
      return 2;
    }

    return_entry[0] = inode->entry;
    *return_size = 1;
  } else {
    *return_size = 2; // itself, ".."

    // d_files に含まれる 子ファイル/ディレクトリ を数える
    unsigned int child_count = 0;
    do {
      if (inode->entry->dir.d_files[child_count+1] == ONE_INODE_INO_INVALID) {
        break;
      }
    } while(++child_count < DIR_MAX_FILES_NUM - 1);

    *return_size += child_count;

    *return_entry = (FSEntry **)memman_alloc(memman, *return_size * sizeof(FSEntry *));

    if (return_entry == NULL) {
      return 2;
    }

    return_entry[0] = inode->entry;

    unsigned int i;
    for (i = 0; i <= child_count; i++) {
      struct one_inode *child = get_inode_by_no(inode->entry->dir.d_files[i]);
      if (child == NULL) {
        // return 3;
        continue;
      }

      return_entry[i+1] = child->entry;
    }
  }

  return 0;
}


/**
 * @brief return unused one-inode number
 * @return ONE_INODE_INO_INVALID if error, others are unused one_inode number
 */
unsigned int get_unused_one_inode_number(unsigned int greater_than) {
  struct one_inode *inode = superfs->root_inode;

  // Error
  if ((ONE_INODE_INO_MAX-1) <= greater_than) {
    return ONE_INODE_INO_INVALID;
  }

  // 最後の inode の ino が割当上限でない
  while (inode->next != NULL) {

    // inode->ino が greater_than 以上であり、かつ
    // inode が連続していない場合は、その間にある ino を返す
    if (greater_than <= inode->ino &&
        1 < (inode->next->ino - inode->ino)) {
      return inode->ino + 1;
    }

    inode = inode->next;
  }

  // この時点で、 inode はリスト末尾の one_inode を指す

  // もし、 inode->ino が割当上限でなければ、末尾の inode->ino +1 を返す
  if (inode->ino != (ONE_INODE_INO_MAX-1)) {
    // greater_than より 大きい値 を返す
    if (greater_than < inode->ino) {
      return inode->ino + 1;
    } else {
      return greater_than + 1;
    }
  }

  return ONE_INODE_INO_INVALID; // not found unused ino greater than ino
}


/**
 * @brief return unused block number
 * @param from: find unused block with block-no from `from` (including from)
 * @return BLOCK_NO_INVALID if error, others are unused block number
 */
unsigned int get_unused_blockno(int from) {
  Block *block = superfs->master_block_buffer;

  if (from == 0) {
    from = 1; // fix
  }

  if (from == BLOCK_NO_MAX || from == BLOCK_NO_INVALID) {
    return BLOCK_NO_INVALID;
  }

  while (block->next != NULL && block->next->block_no < from) {
    block = block->next;
  }

  // assert((block->next == NULL) || (from <= block->next->block_no));
  // - block->next == NULL (from <= block_no となる block が存在しない)
  // - from <= block->next->block_no (from は使われていない)
  if (block->next == NULL || from < block->next->block_no) {
    return from;
  }

  block = block->next;
  // assert(block->block_no == from);

  while (block->next != NULL) {
    // block が連続していない場合は、その間にある block_no を返す
    if (1 < (block->next->block_no - block->block_no)) {
      return block->block_no + 1;
    }

    block = block->next;
  }

  // この時点で、 block はリスト末尾の Block を指す

  // もし、 block->block_no が割当上限でなければ、末尾の block->block_no +1 を返す
  if (block->block_no != (BLOCK_NO_MAX-1) &&
     (block->block_no + 1) != BLOCK_NO_INVALID) {
      return block->block_no + 1;
  }

  return BLOCK_NO_INVALID;
}


/**
 * @brief free Block
 * @note this function can't free master_block_buffer (block_no: 0)
 * @return 0: success, error if others
 */
int free_block(Block *block) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

  if (block->block_no == 0) {
    return 1; // error
  }

  if (block->prev == NULL && block->next != NULL) {
    return 2; // error
  }

  if (block->next) {
    block->next->prev = block->prev;
    block->prev->next = block->next;
  } else {
    if (block->prev) {
      block->prev->next = NULL;
    }
  }

  memman_free(memman, block->block_data, BLOCK_DATA_LEN);
  memman_free(memman, block, sizeof(Block));

  return 0;
}


/**
 * @brief free Blocks cointained by BlockGroup and BlockGroups following by block_group
 * @return 0: success, error if others
 */
int free_block_group(BlockGroup *block_group) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  BlockGroup *block_group_tmp = block_group, *next_block_group;
  unsigned int i, ret;

  // BlockGroup の前から、 block_group->blocks[0..BLOCK_GROUP_BLOCKS_NUM], block_group を開放していく
  do {
    for (i = 0; i < BLOCK_GROUP_BLOCKS_NUM; i++) {
      if (block_group_tmp->blocks[i]) {
        ret = free_block(block_group_tmp->blocks[i]);

        if (ret) {
          return ret; // error during free block;
        }
      } else {
        break;
      }
    }

    next_block_group = block_group_tmp->next;
    memman_free(memman, block_group_tmp, sizeof(BlockGroup));

  } while ((block_group_tmp = next_block_group));

  return 0;
}


/**
 * @brief get inode by inode no
 * @param ino inode no
 * @return NULL if error, others are one-inode
 */
struct one_inode* get_inode_by_no(unsigned int ino) {
  struct one_inode* inode;

  inode = superfs->root_inode;
  while ((inode = inode->next)) {
    if (inode->ino == ino) {
      return inode;
    }

    if (ino == 0) {
      return superfs->root_inode;
    }

    // ino は単調増加なので、 ino < inode->ino となった場合は見つからない
    if (ino < inode->ino) {
      return NULL;
    }
  }

  return NULL;
}

/**
* @brief get inode by absolute path
* @param abs_path: absolute path
* @note abs_path == "" #=> same as "/"
* @return one_inode
*/
struct one_inode* get_inode_by_path(const char *abs_path) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

  struct one_inode *inode = superfs->root_inode;

  if (*abs_path == '\0' || (strcmp(abs_path, "/") == 0)) {
    return inode; // root_inode
  }

  unsigned int d, j, depth;
  depth = get_path_depth(abs_path);
  unsigned char *subpath;

  for (d = 0; d < depth; d++) {
    subpath = get_subpath(abs_path, d);

    if (strcmp(subpath, ".") == 0) {
      continue;
    }

    if (strcmp(subpath, "..") == 0) {
      inode = get_inode_by_no(inode->entry->dir.d_files[0]);
      if (inode == NULL) {
        goto not_found; // unexpected error
      }

      continue;
    }

    for (j = 1; j < DIR_MAX_FILES_NUM; j++) {
      if (inode->entry->dir.d_files[j] == ONE_INODE_INO_INVALID) {
        goto not_found;
      }

      struct one_inode *inode_tmp = get_inode_by_no(inode->entry->dir.d_files[j]);
      if (inode_tmp == NULL) {
        goto not_found; // unexpected error
      }

      if (strcmp(inode_tmp->entry->common.name, subpath) == 0) {
        if (inode_tmp->mode & ONE_INODE_TYPE_DIR) { // Dir
          inode = inode_tmp;
        } else { // File
          if (d == (depth - 1)) {
            inode = inode_tmp; // found
            break;
          } else { // 最下層ではないが File の場合
            goto not_found;
          }
        }

        break;
      } else {
        // inode->d_files[j] が探していたものではない場合

        if (j == (DIR_MAX_FILES_NUM - 1)) {
          // d_files の最後まで走査しても見つからなかった場合
          goto not_found;
        }
      }
    }

    memman_free(memman, subpath, strlen(subpath) + 1);
  }

  return inode;

not_found:
  memman_free(memman, subpath, strlen(subpath) + 1);
  return NULL;
}


/**
 * @brief get block by block-no
 * @param no: block-no (not lba)
 * @return block. 0 if not found.
 */
Block* get_block_by_no(unsigned int block_no) {
  Block* block;

  block = superfs->master_block_buffer;
  while ((block = block->next)) {
    if (block->block_no == block_no) {
      return block;
    }

    // block_no は単調増加なので、 block_no < block->block_no となった場合は見つからない
    if (block_no < block->block_no) {
      return NULL;
    }
  }

  return NULL;
}

/**
 * @brief get depth of absolute path
 * @param abs_path: absolute path
 * @return depth of `abs_path`
 * @note ex: "/"               : 1
 *           "/foo"            : 1
 *           "/foo/bar"        : 2
 *           "/foo/bar/buzz"   : 3
 */
unsigned int get_path_depth(const char *abs_path) {
  unsigned int ret = 0, i = 0;

  while (abs_path[i] != '\0') {
    if (abs_path[i++] == '/') {
      ret++;
    }
  }

  return ret;
}


/**
 * @brief get sub path of absolute path
 * @param abs_path: absolute path
 * @param depth: depth of path
 * @return `subpath`
 * @note ex: "/foo/bar/buzz" , 0 -> "foo"
 *           "/foo/bar/buzz" , 1 -> "bar"
 *           "/foo/bar/buzz" , 2 -> "buzz"
 *           "/foo/bar/buzz" , 3 -> NULL
 *           "/"             , 0 -> "" (NOT NULL)
 */
char *get_subpath(const char *abs_path, unsigned int depth) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  unsigned int depth_count = -1, len;

  char *c = (char *)abs_path, *start = NULL, *ret;

  for ( ; *c != '\0'; c++) {
    if (*c == '/') {
      depth_count++;

      if (depth_count == depth) {
        start = c + 1;
      }

      if (depth_count == (depth+1)) {
        break;
      }
    }
  }

  if (start == NULL) {
    return NULL; // abs_path が '/' を1つも含まなかった場合
  }

  // *c は '\0' または '/' を指している
  if ((*c == '/') || (*c == '\0')) {
    len = c - start;
  } else {
    return NULL; // unexpected error
  }

  // len + 1 (終端文字 '\0' 分を足す)
  ret = (char *)memman_alloc(memman, len + 1);

  if (ret == NULL) {
    return NULL;
  }

  memcpy(ret, start, len);

  // 末尾に終端文字 '\0' を付加
  ret[len] = '\0';

  return ret;
}


/**
 * @brief get part of directory part of absolute path
 * @param abs_path: absolute path
 * @return `part of directory name`
 * @note ex: "/foo/bar/buzz" -> "/foo/bar"
 *           "/foo"          -> "" (NOT NULL)
 */
char *get_dirname(const char *abs_path) {
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  unsigned int depth = get_path_depth(abs_path);
  char *end = (char *)abs_path;

  while (depth) {
    if (*(end++) == '/') {
      depth--;
    }
  }

  // この時、 end は abs_path の最後の '/' を指している

  unsigned int len = end - abs_path - 1; // 最後の '/' の分を引く

  char *ret = (char *)memman_alloc(memman, len + 1);
  memcpy(ret, abs_path, len);

  // 末尾に終端文字 '\0' を付加
  ret[len] = '\0';

  return ret;
}


/**
 * @brief get part of file name of absolute path
 * @param abs_path: absolute path
 * @return `part of file name`
 * @note ex: "/foo/bar/buzz" -> "buzz"
 *           "/"             -> "" (NOT NULL)
 */
char *get_filename(const char *abs_path) {
  unsigned int depth = get_path_depth(abs_path);

  return get_subpath(abs_path, depth-1);
}
