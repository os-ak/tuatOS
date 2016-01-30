#define BLOCK_DATA_LEN         4096 // 4KB
#define SECTOR_SIZE            512  // 512 Byte
#define BLOCK_DATA_LEN_SECTORS (BLOCK_DATA_LEN / SECTOR_SIZE)
#define BLOCK_GROUP_BLOCKS_NUM 16
#define DIR_MAX_FILES_NUM      16
#define MAX_FILENAME_LEN       16

#define ONE_INODE_TYPE_DIR     0x8000
#define ONE_INODE_TYPE_FILE    0x0000
#define ONE_INODE_INO_INVALID  0xFFFFFFFF // 2^32 - 1
#define ONE_INODE_INO_MAX      0xFFFFFFFF // 2^32 - 1
#define BLOCK_NO_INVALID       0xFFFFFFFF // 2^32 - 1
#define BLOCK_NO_MAX           0xFFFFFFFF // 2^32 - 1

#define FD_MODE_WRITE				1 << 0
#define FD_MODE_READ				1 << 1
#define FD_MODE_READ_WRITE	(FD_MODE_WRITE | FD_MODE_WRITE)
#define FD_MODE_APPEND			1 << 2
#define FD_MODE_TRUNCATE		1 << 3

typedef struct Block {
  unsigned int block_no;  // block no
  unsigned char *block_data;  // block data (BLOCK_DATA_LEN bytes)

  unsigned char loaded; // 1 if block_data is on_memory
  unsigned char free;   // 1 if free
  unsigned char dirty;  // 1 if dirty (= updated but not wrote to device)

  struct Block *prev;
  struct Block *next;
} Block;


// blocks[i-1]->block_no < blocks[i]->block_no となることが保証される
typedef struct block_group {
  Block *blocks[BLOCK_GROUP_BLOCKS_NUM];
  struct block_group *next; // NULL or pointer to next BlockGroup
} BlockGroup;


// 4 + 16 + 4 * 16 = 84 bytes
typedef struct dir_entry {
  unsigned int    ino;
  unsigned char   d_name[MAX_FILENAME_LEN];

  // d_files[0] = ino of parent
  // d_files[1..DIR_MAX_FILES_NUM-1] = ino of children
  // d_files は前から詰まっていることが保証される
  unsigned int    d_files[DIR_MAX_FILES_NUM]; // array of ino (of DirEntry or FileEntry)
} DirEntry;


// 4 + 16 + 4 + 4 + 2 = 30 bytes
typedef struct file_entry {
  unsigned int  ino;
  unsigned char f_name[MAX_FILENAME_LEN]; // `SAME` memory alignment of DirEntry

  unsigned int  parent_ino; // ino of parent directory

  BlockGroup *block_group;

  unsigned short links_count;       // Hard-links count
  // unsigned int* p_dirty_inoblk;  // inode block dirty flag pointer
} FileEntry;

typedef union file_system_entry {
  DirEntry dir;
  FileEntry file;

  struct entry {
    unsigned int  ino;
    unsigned char name[MAX_FILENAME_LEN];
  } common;
} FSEntry;

struct one_inode {
  unsigned int   ino;
  unsigned int   size; // byte size of file or directory
  unsigned short mode; // File mode (Directory if MSB == 1)

  union file_system_entry *entry;

  struct one_inode *prev;
  struct one_inode *next;

  unsigned char padding[106]; // reserved (128-4-4-2-4-4-4)
};

typedef struct file_descriptor {
	struct one_inode *inode;
	unsigned int mode;
	unsigned int pos;
} FileDescriptor;

void api_putstr0(char *s);
void api_end(void);
int api_getpid(void);
void api_int_str(int in);
void api_sleep(int time);
void api_exit(void);
FileDescriptor *api_open(char* abs_path,unsigned int mode);
int api_write(FileDescriptor *fd, int bytes, char *src);
int api_read(FileDescriptor *fd, int bytes, char *result);
int api_close(FileDescriptor *fd);
int api_crate(char *abs_path);
