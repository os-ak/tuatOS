/* asmhead.nas */
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; /* ?u?[?g?Z?N?^?͂�???܂Ńf?B?X?N????���?? */
	char leds; /* ?u?[?g???̃L?[?{?[?h??ED?�?? */
	char vmode; /* ?r?f?I???[?h  ???r?b?g?J???[?? */
	char reserve;
	short scrnx, scrny; /* ????��x */
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
void io_out8(int port, int data);
void io_out16(int port,int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler2E(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_hrb_api(void);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
void asm_end_app(void);
void asm_hdd(void);
void test_hdd(int flg, int in1,int in2,void* in3);
void stop_hdd(int status);
void asm_end_hdd(void);

/* fifo.c */
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);
#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

/* int.c */
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

/* memory.c */
#define MEMMAN_FREES		4090	/* ??????2KB */
#define MEMMAN_ADDR			0x003c0000
struct FREEINFO {	/* ?????? */
	unsigned int addr, size;
};
struct MEMMAN {		/* ???????�?? */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* sheet.c */
#define MAX_SHEETS		256
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};
struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
#define CX 22
#define CY 20
#define cons_x (CX*16+16)
#define cons_y (CY*16+37)
/* timer.c */
#define MAX_TIMER		500
struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;
void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);

/* mtask.c */
#define MAX_TASKS		1000	/* ?�???X?N?? */
#define TASK_GDT0		3		/* TSS??DT?�???�???��??��??�???? */
#define MAX_TASKS_LV	100
#define MAX_TASKLEVELS	10
//�t���O�e��
#define TASK_NONUSE -1
#define TASK_RUNNING 2
#define TASK_SLEEPING 1
#define TASK_SETTING 0
#define TASK_HDD_WAITING 4
#define TASK_HDD_USING 5
#define TASK_OTHER_WAITING 6

struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
struct HDD_DATA{
	int flg,in1,in2;
	void* in3;
	int status;
};
struct TASK {
	int sel, flags; /* sel��GDT�̔ԍ��̂��� */
	int pid;
	int level, priority;
	char *name;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct HDD_DATA h_data;
};
struct TASKLEVEL {
	int running; /* ????��?�?????X?N?�?? */
	int now; /* ???�????��?�?????X?N???ǂꂾ???????????�???邽?߂̕�?? */
	struct TASK *tasks[MAX_TASKS_LV];
};
struct TASKCTL {
	int now_lv; /* ���ݓ��쒆�̃��x�� */
	int wait_num;//���ݑҋ@���̃^�X�N��
	int now_tasks;//���ݎ��s���̃^�X�N��
	char lv_change; /* �����^�X�N�X�C�b�`�̂Ƃ��ɁA���x�����ς����ق����������ǂ��� */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
	struct TASK *wait0[MAX_TASKS];
};
extern struct TIMER *task_timer;
struct TASK *task_now(void);
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_switch(void);
void task_switchsub(void);
void task_remove(struct TASK *task);
void task_sleep(struct TASK *task,int flag);
void task_clean(struct TASK *task);
struct TASK *task_b_make(int i);
void task_exit();
void task_b_main(struct SHEET *sht_win_b, int num);
void app_main(int esp);
void task_hdd_main(int flg, int in1,int in2,void* in3);
void start_hdd(int flg, int in1,int in2,void* in3);
void end_hdd(int status);
int catch_hdd(int flg, int in1, int in2, void* in3);
void sys_exit(void);
void sys_sleep(int in);
int sys_getpid(void);

/* window.c */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);

/* console.c */
struct CONSOLE {
	struct SHEET *sht;
	int cur_x, cur_y, cur_c;
};
void console_task(struct SHEET *sheet, unsigned int memtotal);
void cons_putchar(struct CONSOLE *cons, int chr, char move);
void cons_newline(struct CONSOLE *cons);
void cons_putstr0(struct CONSOLE *cons, char *s);
void cons_putstr1(struct CONSOLE *cons, char *s, int l);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal);
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0d(int *esp);
void cmd_kill(struct CONSOLE *cons,int in);
void cmd_pause(struct CONSOLE *cons,int in);
void cmd_run(struct CONSOLE *cons,int in);
void cmd_wakeup(struct CONSOLE *cons,int in);
void cmd_ps(struct CONSOLE *cons);

/* file.c */
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

/* hdd.c */
/* ATA Information */
//IRQ number
#define ATA_IRQ_NUM 14
//base addr of primary IDE
#define ATA_BASE_REG 0x1F0
//Data
#define ATA_DTR (ATA_BASE_REG+0)
//Err
#define ATA_ERR (ATA_BASE_REG+1)
//Feature
#define ATA_FTR (ATA_BASE_REG+1)
//Sector Count
#define ATA_SCR (ATA_BASE_REG+2)
//Sector Number
#define ATA_SNR (ATA_BASE_REG+3)
//Cylinder Low
#define ATA_CLR (ATA_BASE_REG+4)
//Cylinder High
#define ATA_CHR (ATA_BASE_REG+5)
//Device/Head
#define ATA_DHR (ATA_BASE_REG+6)
//Status
#define ATA_STR (ATA_BASE_REG+7)
//Commnad
#define ATA_CMDR (ATA_BASE_REG+7)
//Alternate Status
#define ATA_ASR 0x3F6
//Device Control
#define ATA_DCR ATA_ASR

/*Special Bits*/
#define ATA_BIT_BSY 0x80
#define ATA_BIT_DRQ 0x08
#define ATA_BIT_ERR 0x01
#define ATA_BIT_DRDY 0x40
#define ATA_BIT_nIEN 0x02
#define ATA_BIT_DEV 0x10

/*Commands*/
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_SET_FEATURES 0xEF
#define ATA_SUBCMD_SET_TRANFER 0x03
#define ATA_CMD_READ_SECTOR 0x20
#define ATA_CMD_WRITE_SECTOR 0x30
#define ATA_CMD_SET_MULTIMODE 0xC6
#define ATA_CMD_WRITE_MULTIPLE_SECTOR 0xC5
#define ATA_CMD_READ_MULTIPLE_SECTOR 0xC4

/* Limitation */
#define ATA_WAIT_LIMIT 10000000
#define ATA_RETRY_LIMIT 5

/* Other definition */
#define DEV_ATA 1
#define DEV_ATAPI 2
#define DEV_UNKNOWN 3
#define DEV_NIL 4
#define BLOCK_LENGTH 256
#define ATA_DEFAULT_DHR 0x40

void inthandler2E(int *esp);

struct ATA_CMD_STR{
	int feature;
	int sec_cnt,sec_num;
	int ch,cl;
	int dev_head;
	int DRDY_chk;
	int command;
};

struct DEV_INF{
	int pio_mode;
	int dma_mode;
};
void ide_set_device_number(int device);
int ide_initialize_common_check();
int ide_initialize_check_by_type(int device,int ch,int cl);
int ide_wait_BSYclr();
int ide_device_selection_protocol(int device);
void ide_finish_sector_rw();
int ide_identify_device(int device,void* const buf);
int ide_get_dev_type(int cl,int ch);
int ide_pio_datain_protocol(int device,struct ATA_CMD_STR* ata,int sec_cnt,void* const buf);
int ide_wait_DRDY();
//void ide_ata_read_identify_data(int length,void* const buf);
void ide_ata_read_data(int length,void* const buf);
int ide_wait_BSYclr_DRQclr();
int ide_nondata_protocol(int device,struct ATA_CMD_STR* ata);
void ide_protocol_exec_command(struct ATA_CMD_STR* ata);
int ide_set_features(int device,int subcmd,int sec_cnt,int sec_num,int cl,int ch);
int ide_set_multimode(int device,int sec_cnt);
int ide_initialize_transfer_mode(int device);
int ide_get_pio_mode(int device);
int ide_ata_read_sector_pio(int device,int lba,int sec_cnt,void* const buf);
int ide_pio_dataout_protocol(int device,struct ATA_CMD_STR* ata,int sec_cnt,void* const buf);
void ide_ata_write_data(int length,void* const buf);
int ide_ata_write_sector_pio(int device,int lba,int sec_cnt, void* const buf);
int ide_ata_write_multiple_sector_pio(int device,int lba,int sec_cnt, void* const buf);
int ide_ata_read_multiple_sector_pio(int device,int lba,int sec_cnt,void* const buf);
int ide_initialize_dev_params(int device,int head,int sector);
void inthandler2E(int *esp);
void ide_initialize_device(int device,int sec_cnt);
int ide_get_is_interrupt();
int ide_get_dev_identify_inf(int num);
int ide_get_maximum_logical_sector();
int ide_read(int lba, int sec_cnt, void* const buf);
int ide_write(int lba, int sec_cnt, void* const buf);


struct IDE_WAITER {
	unsigned int timeout,count;
	int flag;
};
void ide_init_waiter();
void ide_set_wait_time(unsigned int timeout);
void ide_wait_10milsec(unsigned int timeout);


/*
 * fs.c
 */
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


// max (84, 30, (4 + 16)) = 84 bytes
typedef union file_system_entry {
  DirEntry dir;
  FileEntry file;

  struct entry {
    unsigned int  ino;
    unsigned char name[MAX_FILENAME_LEN];
  } common;
} FSEntry;


// 128 bytes
struct one_inode {
  unsigned int   ino;
  unsigned int   size; // byte size of file or directory
  unsigned short mode; // File mode (Directory if MSB == 1)

  union file_system_entry *entry;

  struct one_inode *prev;
  struct one_inode *next;

  unsigned char padding[106]; // reserved (128-4-4-2-4-4-4)
};


typedef struct super_fs {
  unsigned char  dev_no; // 0

  Block *master_block_buffer; // block_no: 0, next_block is first block.
  DirEntry *root_direntry;

  struct one_inode *root_inode;
} SuperFS;


#define FD_MODE_WRITE				1 << 0
#define FD_MODE_READ				1 << 1
#define FD_MODE_READ_WRITE	(FD_MODE_WRITE | FD_MODE_WRITE)
#define FD_MODE_APPEND			1 << 2
#define FD_MODE_TRUNCATE		1 << 3

typedef struct file_descriptor {
	struct one_inode *inode;
	unsigned int mode;
	unsigned int pos;
} FileDescriptor;

/*
- SuperFSの初期化
- Entry の作成 (syscall_create) (絶対パス, ディレクトリかどうか)
  - ino の確保
- Entry の削除 (syscall_remove) (パス)
  - ino の開放
  - Block の削除 -> BlockGroup の削除 -> one_inode の更新
  - DirEntry の辻褄合わせ（d_files[]を更新する）
  - (Links の辻褄合わせ)
- ファイルの書き込み (syscall_write) (ファイルパス, データ, データのサイズ, 書き換えかどうか)
  - 書き換えだったら: Block の削除 -> BlockGroup の削除 -> one_inode の更新
  - BlockGroup を作成 -> Block の確保 -> one_inode の更新
  - データを Block に書き込み
- ファイルの読み込み (syscall_read) (ファイルパス, サイズ)
  - ファイルを読んで返す
- ファイルリスト (syscall_filelist) (パス)
  - FSEntry (ただし block_group == NULL)
- ino -> ファイル名
*/

/**
 * @brief "create" procedure for block device
 *
 * @param dev_no (0)
 * @param block_count: fs size become (block_count * BLOCK_DATA_LEN) bytes
 * @return 0: success, error if others
 * @result created SuperFS is assigned to `superfs` (statically defined in fs.c)
 */
int make_superfs(unsigned char dev_no, unsigned int block_count);

/**
 * @brief "create" procedure for block device
 *
 * @param abs_path
 * @param is_file: 1 if File, 0 if Directory
 * @return 0: success, error if others
 */
int syscall_create(char *abs_path, unsigned char is_file, unsigned int *return_ino);

/**
 * @brief "remove" file or directory located `abs_path`
 * @param abs_path
 * @param is_recursive: 1 if recursive, 0 if not recursive
 * @return 0: success, error if others
 *
 * @note abs_path に存在するファイルまたはディレクトリを "データ(ブロック)ごと削除します"
 *       is_recursive: 0 で呼び出した場合に、 `abs_path` がディレクトリを指す場合はエラーを返します
 */
int syscall_remove(char *abs_path, unsigned char is_recursive);

/**
 * @brief create FileDescriptor and returns
 * @param abs_path: absolute path
 * @param mode: FS_MODE_*
 */
FileDescriptor *syscall_open(char *abs_path, unsigned int mode);

/**
 * @brief close FileDescriptor and return status
 * @param fd: FileDescriptor
 */
int syscall_close(FileDescriptor *fd);

/**
 * @brief read data using FileDescriptor
 * @param fd: FileDescriptor
 * @param bytes: number of bytes to read
 * @param result: result data (must bigger than `bytes`)
 */
int syscall_read(FileDescriptor *fd, int bytes, int offset, char *result);

/**
 * @brief write data using FileDescriptor
 * @param fd: FileDescriptor
 * @param bytes: number of bytes to write
 * @param src: source data (must bigger than `bytes`)
 */
int syscall_write(FileDescriptor *fd, int bytes, char *src);

/**
 * @brief seek FileDescriptor
 * @param fd: FileDescriptor
 * @param offset: offset (see note)
 * @param is_relative: 0 if absolute, others is relative
 * @note if absolute, result `fd`'s position == `offset`
 *       if relative, result `fd`'s position == `current position` + `offset`
 */
int syscall_seek(FileDescriptor *fd, int offset, char is_relative);


/**
 * @brief "remove" file or directory of one-inode(`ino`)
 * @param ino
 * @param is_recursive: 1 if recursive, 0 if not recursive
 * @return 0: success, error if others
 *
 * @note one-inode に存在するファイルまたはディレクトリを "データ(ブロック)ごと削除します"
 *       is_recursive: 0 で呼び出した場合に、 `ino` が指す one-inode がディレクトリを指す場合はエラーを返します
 */
int remove_fsentry_by_ino(unsigned int ino, unsigned char is_recursive);

/**
 * @brief "write" file located `abs_path`
 *
 * @param abs_path
 * @param data
 * @param data_size: size bytes
 * @param is_overwrite: not used
 * @param return_file_entry: store (FileEntry *) of `abs_path` if success
 * @return 0: success, error if others
 */
// NOTE: 現時点では1ブロックしか書き込まない
// int syscall_write(char *abs_path, unsigned char *data, unsigned int data_size, unsigned char is_overwrite, FileEntry **return_file_entry);

/**
 * @brief "read" file located `abs_path` and store into block(s)
 *
 * @param abs_path
 * @param size: size bytes
 * @param offset: offset bytes
 * @param return_file_entry: store (FileEntry *) of `abs_path` if success
 * @return 0: success, error if others
 *
 * @note 読み込むデータの範囲が複数のブロックにまたがる場合は、またがるブロックのデータすべてを読み込みます
 */
// int syscall_read(char *abs_path, unsigned int size, unsigned int offset, FileEntry **return_file_entry);

/**
 * @brief "get" array of (FSEntry *) located `abs_path`
 *
 * @param abs_path
 * @param return_entry: head address of (array of (FSEntry *))
 * @param return_size: length of (array of (FSEntry *))
 * @return 0: success, error if others
 *
 * @note abs_path がファイルを指す場合には FSEntry 1つを返します
 *       abs_path がディレクトリを指す場合には、
 *         特殊ファイル(".", "..") を含めたそのディレクトリにある全ての (FSEntry *) の配列を返します
 */
int syscall_get_fsentry(FileDescriptor *fd, FSEntry **return_entry, int *return_size);

/**
 * @brief get `previous` inode by inode no
 * @param ino inode no
 * @return one_inode
 */
struct one_inode* get_prev_inode_by_no(unsigned int ino);

/**
 * @brief get `previous` block by block no
 * @param block_no: block no
 * @return block
 */
struct Block* get_prev_block_by_no(unsigned int block_no);

/**
 * @brief return unused one-inode number greater than `greater_than`
 * @return ONE_INODE_INO_INVALID if error, others are unused one_inode number
 */
unsigned int get_unused_one_inode_number(unsigned int greater_than);

/**
 * @brief return unused block number
 * @param from: find unused block with block-no from `from` (including from)
 * @return BLOCK_NO_INVALID if error, others are unused block number
 */
unsigned int get_unused_blockno(int from);

/**
 * @brief free Block
 * @note this function can't free master_block_buffer (block_no: 0)
 * @return 0: success, error if others
 */
int free_block(Block *block);

/**
 * @brief free Blocks cointained by BlockGroup and BlockGroups following by block_group
 * @return 0: success, error if others
 */
int free_block_group(BlockGroup *block_group);

/////

/**
 * @brief get inode by inode no
 * @param ino inode no
 * @return NULL if error, others are one-inode
 */
struct one_inode* get_inode_by_no(unsigned int ino);

/**
 * @brief get inode by absolute path
 * @param abs_path: absolute path
 * @note if abs_path == "", it works as if abs_path == "/"
 * @return one_inode
 */
struct one_inode* get_inode_by_path(const char *abs_path);

// Block* get_block_by_file_entry_and_block_index(FileEntry *file_entry, unsigned int block_index);

/**
 * @brief get block by block-no
 * @param blkno: block-no (not lba)
 * @return NULL: error, others are success
 */
Block* get_block_by_no(unsigned int block_no);

/**
 * @brief get depth of absolute path
 * @param abs_path: absolute path
 * @return depth of `abs_path`
 * @note ex: "/"               : 1
 *           "/hoge"           : 1
 *           "/hoge/fuga/nyaa" : 3
 */
unsigned int get_path_depth(const char *abs_path);

/**
 * @brief get sub path of absolute path
 * @param abs_path: absolute path
 * @param depth: depth of path
 * @return `subpath`
 * @note ex: "/a/b/c" , 0 -> "a"
 *           "/a/b/c" , 1 -> "b"
 *           "/a/b/c" , 2 -> "c"
 *           "/a/b/c" , 3 -> NULL
 *           "/a/b/c/", 3 -> "" (NOT NULL)
 *           "/"      , 0 -> "" (NOT NULL)
 */
char *get_subpath(const char *abs_path, unsigned int depth);

/**
 * @brief get part of directory part of absolute path
 * @param abs_path: absolute path
 * @return `part of directory name`
 * @note ex: "/hoge/fuga/nyaa" -> "/hoge/fuga"
 *           "/hoge"           -> "" (NOT NULL)
 */
char *get_dirname(const char *abs_path);

/**
 * @brief get part of file name of absolute path
 * @param abs_path: absolute path
 * @return `part of file name`
 * @note ex: "/hoge/fuga/nyaa" -> "nyaa"
 *           "/"               -> "" (NOT NULL)
 */
char *get_filename(const char *abs_path);


int test_find_inode(const char *filename);
// unsigned char *test_find_block(const char *filename);
unsigned char *test_find_block(FileDescriptor *fd);
unsigned int test_find_blockno(const char *filename);
struct one_inode *test_get_root_inode();
