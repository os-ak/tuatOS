/* bootpack??ｿｽ?ｿｽ?ｿｽ???ｿｽ?ｿｽ??ｿｽ?ｿｽC??ｿｽ?ｿｽ??ｿｽ?ｿｽ */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED		0xed
struct SHEET *sht_win_b[5], *sht_back,*sht_win_c[2];
struct MEMMAN *memman;
struct SHTCTL *shtctl;
extern unsigned int switch_count;

void print_test_title     (struct SHEET *sht_back, int print_x, int *print_y, const char *abs_path, char is_file);
int debug_print_inode_list(struct SHEET *sht_back, int print_x, int *print_y);
int debug_print_blocks    (struct SHEET *sht_back, int print_x, int *print_y, const char *abs_path);
int test_syscall_create   (struct SHEET *sht_back, int print_x, int  print_y, const char *abs_path, char is_file, unsigned int *result_ino);
// int test_syscall_read     (struct SHEET *sht_back, int print_x, int  print_y, const char *abs_path, unsigned int size, unsigned int offset, FileEntry *result_file_entry);
int test_syscall_read     (struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd, unsigned int bytes, unsigned int offset);
// int test_syscall_write    (struct SHEET *sht_back, int print_x, int  print_y, const char *abs_path, unsigned char *data, unsigned int size, char is_overwrite, FileEntry *result_file_entry);
int test_syscall_write    (struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd, unsigned int bytes, unsigned char *src);
FileDescriptor *test_syscall_open (struct SHEET *sht_back, int print_x, int print_y, const char *abs_path, unsigned int mode);
int test_syscall_close    (struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[80];
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[32];
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons,*buf_win_b,*buf_win_c;;
	struct SHEET *sht_mouse, *sht_win, *sht_cons;
	struct TASK *task_a, *task_cons;
	struct TIMER *timer, sw_timer;
	/* for HDD TEST */
	int hdd_test,flg,flg2,buf[384];
	buf[0] = 2015;
	buf[1] = 12;
	buf[2] = 5;
	/* for HDD TEST finish*/

	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC??ｿｽ?ｿｽﾌ擾ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽI??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌゑｿｽCPU??ｿｽ?ｿｽﾌ奇ｿｽ??ｿｽ?ｿｽ闕橸ｿｽﾝ禁止??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	ide_init_waiter();
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIT??ｿｽ?ｿｽ??ｿｽ?ｿｽPIC1??ｿｽ?ｿｽﾆキ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ{??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ(11111000) */
	// io_out8(PIC1_IMR, 0xef); /* ??ｿｽ?ｿｽ}??ｿｽ?ｿｽE??ｿｽ?ｿｽX??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ(11101111) */
	io_out8(PIC1_IMR, 0xaf); /* ??ｿｽ?ｿｽ}??ｿｽ?ｿｽE??ｿｽ?ｿｽX??ｿｽ?ｿｽ??ｿｽ?ｿｽHDD??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ(10101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	ide_initialize_device(0,2);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);

	/* sht_back */
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽF??ｿｽ?ｿｽﾈゑｿｽ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, cons_x * cons_y);
	sheet_setbuf(sht_cons, buf_cons, cons_x, cons_y, -1); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽF??ｿｽ?ｿｽﾈゑｿｽ */
	make_window8(buf_cons, cons_x, cons_y, "console", 0);
	make_textbox8(sht_cons, 8, 28, cons_x-16, cons_y-37, COL8_000000);
	task_cons = task_alloc();
	task_cons->pid = 2;
	task_cons->name = "console";
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level=2, priority=2 */

	for (i = 0; i < 5; i++) {
		sht_win_b[i] = sheet_alloc(shtctl);
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 344 * 52);
		sheet_setbuf(sht_win_b[i], buf_win_b, 344, 52, -1); /* ?ｿｽ?ｿｽ?ｿｽ?ｿｽ?ｿｽF?ｿｽﾈゑｿｽ */
		sprintf(s, "task_b pid:%d", i+3);
		make_window8(buf_win_b, 344, 52, s, 0);
	}

	for(i=0;i<2;i++){
	sht_win_c[i] = sheet_alloc(shtctl);
	buf_win_c = (unsigned char *) memman_alloc_4k(memman, 344 * 52);
	sheet_setbuf(sht_win_c[i], buf_win_c, 344, 52, -1); /* ?ｿｽ?ｿｽ?ｿｽ?ｿｽ?ｿｽF?ｿｽﾈゑｿｽ */
	sprintf(s, "task_c");
	make_window8(buf_win_c, 344, 52, s, 0);
}

	/* sht_win */
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽF??ｿｽ?ｿｽﾈゑｿｽ */
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾊ抵ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾉなゑｿｽ??ｿｽ?ｿｽ謔､??ｿｽ?ｿｽﾉ搾ｿｽ??ｿｽ?ｿｽW??ｿｽ?ｿｽv??ｿｽ?ｿｽZ */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back,  0,  0);
	sheet_slide(sht_cons, 600,  4);
	sheet_slide(sht_win,  328+100, 16);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_cons,  1);
	sheet_updown(sht_win,   2);
	sheet_updown(sht_mouse, 3);

	for(i=0;i<5;i++){
		sheet_slide(sht_win_b[i], 100, 216+i*100);
	}

	for(i=0;i<2;i++){
		sheet_slide(sht_win_c[i], 100, 16+i*100);
	}


	/* ?????L?[?{?[?h??????H?????????????A???????????????? */
	  fifo32_put(&keycmd, KEYCMD_LED);
	  fifo32_put(&keycmd, key_leds);

	  int print_y = 100;

	  // sprintf(s, "%x",hdd_test);
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, (hdd_test==DEV_ATA)?"It's ATA":s, 40);
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, "---------SIZE-------", 100);
	  // sprintf(s, "int: %d",sizeof(int));
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // sprintf(s, "short int: %d",sizeof(short));
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // sprintf(s, "long int: %d",sizeof(long int));
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // sprintf(s, "char: %d",sizeof(char));
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, "------HDD READ & WRITE---------", 100);
	  // sprintf(s, "INITIALIZE TRSF MODE(): %d",flg);
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // sprintf(s, "PIO_MODE: %d",ide_get_pio_mode(0));
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 40);
	  // flg = ide_ata_write_multiple_sector_pio(0,20,3,buf);
	  // flg2 = ide_ata_read_multiple_sector_pio(0,20,3,buf);
	  // flg = ide_ata_write_sector_pio(0,18,1,buf);
	  // flg2 = ide_ata_read_sector_pio(0,18,1,rbuf);
	  // flg = ide_ata_write_sector_pio(0,19,1,buf);
	  // flg2 = ide_ata_read_sector_pio(0,19,1,rbuf);
	  // sprintf(s, "WRTIE:%d READ:%d READ DATA: %d, %d, %d",flg,flg2,buf[0],buf[1],buf[2]);
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  // sprintf(s, "INTERRUPTS: %d",ide_get_is_interrupt());
	  // putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  //////////////////////////////////////////////////////////////////////////////
	  //////////////////////////////////////////////////////////////////////////////
	  //////////////////////////////////////////////////////////////////////////////
	  putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, "------FILE SYSTEM---------", 100);
	  int result, flg_write = 0;
	  unsigned int result_ino;
	  unsigned char result_data[16];
	  unsigned char file_path[64];
	  FileEntry *result_file_entry;

	  unsigned int print_x = 0;

	  result = make_superfs(0, 128);
	  sprintf(s, "make_superfs(0, 128) #=> %d", result);
	  putfonts8_asc_sht(sht_back, 0, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);

	  // /test file
	  // if (1) {
	  //   int is_file = 1;
	  //   strcpy(file_path, "/test");
	  //   print_test_title(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   result = test_syscall_create(sht_back, print_x, (print_y += 20), file_path, is_file, &result_ino);
	  //   if (result != 0) goto after_test;
	  //
	  //   FileDescriptor *fd = test_syscall_open(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   unsigned char data_to_write[16] = "ABCDEFG";
	  //   result = test_syscall_write(sht_back, print_x, (print_y += 20), fd, 16, data_to_write);
	  //   if (result != 0) goto after_test;
	  //
	  //   result = test_syscall_read(sht_back, print_x, (print_y += 20), fd, 16, 0);
	  //   if (result != 0) goto after_test;
	  //
	  //   sprintf(s, "test_find_block #=> %s", test_find_block(fd));
	  //   putfonts8_asc_sht(sht_back, print_x, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  //   if (result != 0) goto after_test;
	  //
	  //   result = test_syscall_close(sht_back, print_x, (print_y += 20), fd);
	  //   if (result != 0) goto after_test;
	  // }
	  // //
	  // // /mitsumine directory
	  // if (1) {
	  //   int is_file = 0;
	  //   strcpy(file_path, "/mitsumine");
	  //   print_test_title(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   result = test_syscall_create(sht_back, print_x, (print_y += 20), file_path, is_file, &result_ino);
	  //   if (result != 0) goto after_test;
	  // }
	  // //
	  // // /mitsumine/mashiro directory
	  // if (1) {
	  //   int is_file = 0;
	  //   strcpy(file_path, "/mitsumine/mashiro");
	  //   print_test_title(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   result = test_syscall_create(sht_back, print_x, (print_y += 20), file_path, is_file, &result_ino);
	  //   if (result != 0) goto after_test;
	  //
	  //   sprintf(s, "file_path: %d, %s", file_path, file_path);
	  //   putfonts8_asc_sht(sht_back, print_x, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  // }
	  // //
	  // // /mitsumine/mashiro/curry file
	  // if (1) {
	  //   int is_file = 1;
	  //   strcpy(file_path, "/mitsumine/mashiro/curry");
	  //   print_test_title(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   result = test_syscall_create(sht_back, print_x, (print_y += 20), file_path, is_file, &result_ino);
	  //   if (result != 0) goto after_test;
	  //
	  //   FileDescriptor *fd = test_syscall_open(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   unsigned char data_to_write[16] = "amakuchi";
	  //   result = test_syscall_write(sht_back, print_x, (print_y += 20), fd, 16, data_to_write);
	  //   if (result != 0) goto after_test;
	  //
	  //   result = test_syscall_close(sht_back, print_x, (print_y += 20), fd);
	  //   if (result != 0) goto after_test;
	  // }
	  //
	  //
	  // // get fs_entry of /mitsumine
	  // if (1) {
	  //   int is_file = 0;
	  //   FSEntry **return_entry;
	  //   int return_size;
	  //   strcpy(file_path, "/mitsumine/mashiro");
	  //   print_test_title(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   FileDescriptor *fd = test_syscall_open(sht_back, print_x, (print_y += 20), file_path, is_file);
	  //
	  //   result = syscall_get_fsentry(fd, return_entry, &return_size);
	  //   sprintf(s, "syscall_get_fsentry(\"%s\") #=> %d, entry: %d, size: %d", file_path, result, *return_entry, return_size);
	  //   putfonts8_asc_sht(sht_back, print_x, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  //   if (result != 0) { goto after_test; }
	  //
	  //   FileEntry *entry;
	  //   unsigned int i;
	  //   for (i = 0; i < return_size; i++) {
	  //     entry = &(return_entry[i]->file);
	  //     sprintf(s, "addr: %d, ino: %d, parent_ino: %d, f_name: %s", entry, entry->ino, entry->parent_ino, entry->f_name);
	  //     putfonts8_asc_sht(sht_back, print_x, (print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);
	  //   }
	  // }
	  //
		struct FIFO32 fifosw;
		struct TIMER *timersw;
		int fifoswbuf[128];
		fifo32_init(&fifosw, 128, fifoswbuf, 0);
		timersw = timer_alloc();
		timer_init(timersw, &fifosw, 9999);
		timer_settime(timersw, 500);

	after_test:

	for (;;) {
		sprintf(s, "time: %d",catch_time());
	  putfonts8_asc_sht(sht_back, 0,160, COL8_FFFFFF, COL8_008484, s, 200);
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			/* ??ｿｽ?ｿｽL??ｿｽ?ｿｽ[??ｿｽ?ｿｽ{??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽg??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾉ托ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽf??ｿｽ?ｿｽ[??ｿｽ?ｿｽ^??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾎ、??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		if(fifo32_status(&fifosw) !=0 ){
			if(fifo32_get(&fifosw)==9999){
				sprintf(s,"sw : %d",switch_count);
				putfonts8_asc_sht(sht_back, 0,180, COL8_FFFFFF, COL8_008484, s, 200);
				switch_count=0;
				timer_settime(timersw, 500);
			}
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a,TASK_SLEEPING);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { /* ??ｿｽ?ｿｽL??ｿｽ?ｿｽ[??ｿｽ?ｿｽ{??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽf??ｿｽ?ｿｽ[??ｿｽ?ｿｽ^ */
				if (i < 0x80 + 256) { /* ??ｿｽ?ｿｽL??ｿｽ?ｿｽ[??ｿｽ?ｿｽR??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽ??ｿｽ?ｿｽ?ｿｽ??ｿｽ?ｿｽR??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽﾉ変奇ｿｽ */
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾍ包ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽA??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽt??ｿｽ?ｿｽ@??ｿｽ?ｿｽx??ｿｽ?ｿｽb??ｿｽ?ｿｽg */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
							((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;	/* ??ｿｽ?ｿｽ蝠ｶ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾉ変奇ｿｽ */
					}
				}
				if (s[0] != 0) { /* ??ｿｽ?ｿｽﾊ常文??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
					if (key_to == 0) {	/* ??ｿｽ?ｿｽ^??ｿｽ?ｿｽX??ｿｽ?ｿｽNA??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						if (cursor_x < 128) {
							/* ??ｿｽ?ｿｽ齦ｶ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽA??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ1??ｿｽ?ｿｽﾂ進??ｿｽ?ｿｽﾟゑｿｽ */
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	/* ??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {	/* ??ｿｽ?ｿｽo??ｿｽ?ｿｽb??ｿｽ?ｿｽN??ｿｽ?ｿｽX??ｿｽ?ｿｽy??ｿｽ?ｿｽ[??ｿｽ?ｿｽX */
					if (key_to == 0) {	/* ??ｿｽ?ｿｽ^??ｿｽ?ｿｽX??ｿｽ?ｿｽNA??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						if (cursor_x > 8) {
							/* ??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽX??ｿｽ?ｿｽy??ｿｽ?ｿｽ[??ｿｽ?ｿｽX??ｿｽ?ｿｽﾅ擾ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽA??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ1??ｿｽ?ｿｽﾂ戻ゑｿｽ */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	/* ??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x1c) {	/* Enter */
					if (key_to != 0) {	/* ??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x0f) {	/* Tab */
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1; /* ??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2); /* ??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌカ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽON */
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000; /* ??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽo??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						fifo32_put(&task_cons->fifo, 3); /* ??ｿｽ?ｿｽR??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ\??ｿｽ?ｿｽ[??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌカ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽOFF */
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x2a) {	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽV??ｿｽ?ｿｽt??ｿｽ?ｿｽg ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* ??ｿｽ?ｿｽE??ｿｽ?ｿｽV??ｿｽ?ｿｽt??ｿｽ?ｿｽg ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽV??ｿｽ?ｿｽt??ｿｽ?ｿｽg OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	/* ??ｿｽ?ｿｽE??ｿｽ?ｿｽV??ｿｽ?ｿｽt??ｿｽ?ｿｽg OFF */
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {	/* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {	/* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	/* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa) {	/* ??ｿｽ?ｿｽL??ｿｽ?ｿｽ[??ｿｽ?ｿｽ{??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽf??ｿｽ?ｿｽ[??ｿｽ?ｿｽ^??ｿｽ?ｿｽ??ｿｽ?ｿｽ?ｿｽ??ｿｽ?ｿｽﾉ受け趣ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {	/* ??ｿｽ?ｿｽL??ｿｽ?ｿｽ[??ｿｽ?ｿｽ{??ｿｽ?ｿｽ[??ｿｽ?ｿｽh??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽf??ｿｽ?ｿｽ[??ｿｽ?ｿｽ^??ｿｽ?ｿｽ??ｿｽ?ｿｽ?ｿｽ??ｿｽ?ｿｽﾉ受け趣ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾈゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				/* ??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌ再表??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* ??ｿｽ?ｿｽ}??ｿｽ?ｿｽE??ｿｽ?ｿｽX??ｿｽ?ｿｽf??ｿｽ?ｿｽ[??ｿｽ?ｿｽ^ */
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* ??ｿｽ?ｿｽ}??ｿｽ?ｿｽE??ｿｽ?ｿｽX??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌ移難ｿｽ */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
						/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ{??ｿｽ?ｿｽ^??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽAsht_win??ｿｽ?ｿｽ??ｿｽ?ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) { /* ??ｿｽ?ｿｽJ??ｿｽ?ｿｽ[??ｿｽ?ｿｽ\??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽp??ｿｽ?ｿｽ^??ｿｽ?ｿｽC??ｿｽ?ｿｽ} */
				if (i != 0) {
					timer_init(timer, &fifo, 0); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ0??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				} else {
					timer_init(timer, &fifo, 1); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ1??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}

struct TASK *task_b_make(int i){
	struct TASK *task_b;
	task_b = task_alloc();
	task_b->pid = i;
	task_b->name ="task_b";
	task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_b->tss.eip = (int) &task_b_main;
	task_b->tss.es = 1 * 8;
	task_b->tss.cs = 2 * 8;
	task_b->tss.ss = 1 * 8;
	task_b->tss.ds = 1 * 8;
	task_b->tss.fs = 1 * 8;
	task_b->tss.gs = 1 * 8;
	*((int *) (task_b->tss.esp + 4)) = (int) sht_win_b[i-3];
	*((int *) (task_b->tss.esp + 8)) = i;
	return task_b;
}

void start_hdd(int flg, int in1,int in2,void* in3){
	struct TASK *task_hdd;
	task_hdd = task_alloc();
	task_hdd->pid = 50;
	task_hdd->name = "hdd";
	task_hdd->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024-20;
	task_hdd->tss.eip = (int) &task_hdd_main;
	task_hdd->tss.es = 1 * 8;
	task_hdd->tss.cs = 2 * 8;
	task_hdd->tss.ss = 1 * 8;
	task_hdd->tss.ds = 1 * 8;
	task_hdd->tss.fs = 1 * 8;
	task_hdd->tss.gs = 1 * 8;
	*((int *) (task_hdd->tss.esp + 4))=flg;
	*((int *) (task_hdd->tss.esp + 8))=in1;
	*((int *) (task_hdd->tss.esp + 12))=in2;
	*((int *) (task_hdd->tss.esp + 16))=(int)in3;
	task_run(task_hdd, 2, 2);
	return;
}

struct TASK *task_c_make(int i){
	struct TASK *task_c;
	task_c = task_alloc();
	task_c->pid = i;
	task_c->name ="task_c";
	task_c->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_c->tss.eip = (int) &task_c_main;
	task_c->tss.es = 1 * 8;
	task_c->tss.cs = 2 * 8;
	task_c->tss.ss = 1 * 8;
	task_c->tss.ds = 1 * 8;
	task_c->tss.fs = 1 * 8;
	task_c->tss.gs = 1 * 8;
	*((int *) (task_c->tss.esp + 4)) = (int) sht_win_c[(i/10)-1];
	*((int *) (task_c->tss.esp + 8)) = i;
	return task_c;
}

struct TASK *task_make_test(int i){
	struct TASK *task;
	task = task_alloc();
	task->pid = i;
	task->name ="test_task";
	task->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	task->tss.eip = (int) &task_main_test;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	return task;
}

void task_main_test(int i){
	for(;;)
		;
}

void task_hdd_main(int flg, int in1,int in2,void* in3)
{
	struct FIFO32 fifo;
	struct TIMER *timer;
	int fifobuf[128],i;
	if(flg==0){
		ide_read(in1,in2,in3);
	}else if(flg==1){
		ide_write(in1,in2,in3);
	}
	fifo32_init(&fifo, 128, fifobuf, 0);
	timer = timer_alloc();
	timer_init(timer, &fifo, 55500);
	timer_settime(timer, 200);
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 55500) {
				end_hdd();//int42
				task_exit();
			}
		}
	}
}

void task_c_main(struct SHEET *sht_win_c,int num)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int i, n, fifobuf[128], cnt = 0;
	char *s;
	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 10000);
	timer_settime(timer_1s, 100);

	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 10000) {
				cnt++;
				sprintf(s, "%07d", cnt);
				putfonts8_asc_sht(sht_win_c, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				timer_settime(timer_1s, 100);
			}
		}
	}
	task_exit();
}

void task_b_main(struct SHEET *sht_win_b,int num)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s,*timer_2s;
	int i, n, fifobuf[128], cnt = 5;
	int in3[128],out[128];
	char s[12],out_string[128] = "OUT:";
	int in1,in2;
	int flg;
	int call = 0;
	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 10000);
	timer_settime(timer_1s, 100);
	timer_2s = timer_alloc();
	timer_init(timer_2s, &fifo, 11100);
	timer_settime(timer_2s, 500);

	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 10000) {
				cnt--;
				if(call==0){
					sprintf(s, "PRE WRITE %d", cnt);
					putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				}else if(call==1){
					sprintf(s, "PRE READ  %d", cnt);
					putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				}else{
					//sprintf(s, "ALL END  %d", cnt);
					putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, out_string, 20);
				}
				if(cnt==-10){
					sprintf(s, "task_b pid:%d", num);
					make_window8(sht_win_b->buf, 344, 52, s, 0);
					sheet_updown(sht_win_b, -1);
					break;
				}
				timer_settime(timer_1s, 100);
			}else if(i==11100){
				if(call==0){
					//譖ｸ縺崎ｾｼ縺ｿ繝?繧ｹ繝?
					call++;
					flg=1;
					in1=20+(num*10);
					in2=num;
					for(n=0;n<num;n++){
						in3[n]=n+(num*100);
					}
					catch_hdd(flg,in1,in2,in3);
					cnt=5;
					timer_settime(timer_2s, 400);
				}else{
					//隱ｭ縺ｿ霎ｼ縺ｿ繝?繧ｹ繝?
					call++;
					flg=0;
					in1=20+(num*10);
					in2=num;
					catch_hdd(flg,in1,in2,in3);
					for(n=0;n<num;n++){
						sprintf(s,"%d ",in3[n]);
						strcat(out_string,s);
					}
				}
			}
		}
	}
	task_exit();
}

void print_test_title(struct SHEET *sht_back, int print_x, int *print_y, const char *abs_path, char is_file) {
  char s[80];

  sprintf(s, "## test: \"%s\" (%s)", abs_path, is_file ? "file" : "directory");
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);
}

int debug_print_inode_list(struct SHEET *sht_back, int print_x, int *print_y) {
  char s[40];

  struct one_inode *inode = test_get_root_inode();
  *print_y -= 20;

  do {
    sprintf(s, "ino: %d, name: %s", inode->ino, inode->entry->common.name);
    putfonts8_asc_sht(sht_back, print_x, (*print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);

  } while((inode = inode->next));

  return 0;
}

int debug_print_blocks(struct SHEET *sht_back, int print_x, int *print_y, const char *abs_path) {
  char s[40];

  *print_y -= 20;
  struct one_inode *inode = get_inode_by_path(abs_path);
  if (inode == NULL || (inode->mode & ONE_INODE_TYPE_DIR)) { return 1; }

  unsigned int block_idx = 0;
  BlockGroup *block_group = inode->entry->file.block_group;

  while (1) {
    if (block_idx == BLOCK_GROUP_BLOCKS_NUM) {
      block_group = block_group->next;

      if (block_group == NULL) {
        break;
      }
    }

    Block *block = block_group->blocks[block_idx % BLOCK_GROUP_BLOCKS_NUM];
    if (block == NULL) {
      break;
    }

    sprintf(s, "%d: block_no: %d, addr: %d", block_idx, block->block_no, block);
    putfonts8_asc_sht(sht_back, print_x, (*print_y += 20), COL8_FFFFFF, COL8_008484, s, 200);

    block_idx++;
  };

  return 0;
}

int test_syscall_create(struct SHEET *sht_back, int print_x, int print_y,
                        const char *abs_path, char is_file, unsigned int *result_ino) {
  char s[80];
  int result = syscall_create(abs_path, is_file, result_ino);
  sprintf(s, "syscall_create(\"%s\", %d) #=> %d, ino: %d", abs_path, is_file, result, *result_ino);
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);

  return result;
}
//
int test_syscall_write(struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd, unsigned int bytes, unsigned char *src) {
                      //  const char *abs_path, unsigned char *data, unsigned int size, char is_overwrite, FileEntry *result_file_entry) {
  char s[80];
  int result = syscall_write(fd, bytes, src);
  sprintf(s, "syscall_write #=> %d", result);
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);

  return result;
}
//
FileDescriptor *test_syscall_open(struct SHEET *sht_back, int print_x, int print_y, const char *abs_path, unsigned int mode) {
  char s[80];
  FileDescriptor *fd = syscall_open(abs_path, mode);
  sprintf(s, "syscall_open #=> fd->inode->entry->common.name #=> %s", fd->inode->entry->common.name);
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);

  return fd;
}
//
int test_syscall_read(struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd, unsigned int bytes, unsigned int offset) {
                      //  const char *abs_path, unsigned int size, unsigned int offset, FileEntry *result_file_entry) {
  char s[80], src[128];
  int result = syscall_read(fd, bytes, offset, src);
  sprintf(s, "syscall_read #=> %d", result);
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);

  return result;
}
//
int test_syscall_close(struct SHEET *sht_back, int print_x, int print_y, FileDescriptor *fd) {
  char s[80];
  int result = syscall_close(fd);
  sprintf(s, "syscall_close #=> %d", result);
  putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);

  return result;
}
//
//
// int test_syscall_remove(struct SHEET *sht_back, int print_x, int print_y,
//                         const char *abs_path, char is_recursive) {
//   char s[80];
//   int result = syscall_remove(abs_path, is_recursive);
//   sprintf(s, "syscall_remove(\"%s\", %d) #=> %d", abs_path, is_recursive, result);
//   putfonts8_asc_sht(sht_back, print_x, print_y, COL8_FFFFFF, COL8_008484, s, 200);
//
//   return result;
// }
