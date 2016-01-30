/* �R���\�[���֌W */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>
extern struct SHEET *sht_win_b[5];
extern struct TASKCTL *taskctl;
FileDescriptor *common_fd;
FileDescriptor *pwd_fd;

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, fifobuf[128], *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	char cmdline[128];
	cons.sht = sheet;
	cons.cur_x =  8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *) 0x0fec) = (int) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

	/* �v�����v�g�\�� */
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task,TASK_SLEEPING);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /* �J�[�\���p�^�C�} */
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /* ����0�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /* ����1�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if (i == 2) {	/* �J�[�\��ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* �J�[�\��OFF */
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			if (256 <= i && i <= 511) { /* �L�[�{�[�h�f�[�^�i�^�X�NA�o�R�j */
				if (i == 8 + 256) {
					/* �o�b�N�X�y�[�X */
					if (cons.cur_x > 16) {
						/* �J�[�\�����X�y�[�X�ŏ����Ă����A�J�[�\����1�߂� */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* �J�[�\�����X�y�[�X�ŏ����Ă������s���� */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					cons_runcmd(cmdline, &cons, fat, memtotal);	/* �R�}���h���s */
					/* �v�����v�g�\�� */
					cons_putchar(&cons, '>', 1);
				} else {
					/* ���ʕ��� */
					if (cons.cur_x <= cons_x - 16) {
						/* �ꕶ���\�����Ă����A�J�[�\����1�i�߂� */
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}else{
						cons_newline(&cons);
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* �J�[�\���ĕ\�� */
			if (cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {	/* �^�u */
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + cons_x-16) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* 32�Ŋ����؂ꂽ��break */
			}
		}
	} else if (s[0] == 0x0a) {	/* ���s */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* ���A */
		/* �Ƃ肠�����Ȃɂ����Ȃ� */
	} else {	/* ���ʂ̕��� */
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			/* move��0�̂Ƃ��̓J�[�\�����i�߂Ȃ� */
			cons->cur_x += 8;
			if (cons->cur_x == 8 + cons_x-16) {
				cons_newline(cons);
			}
		}
	}
	return;
}

void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < 28 + cons_y-53) {
		cons->cur_y += 16; /* ���̍s�� */
	} else {
		/* �X�N���[�� */
		for (y = 28; y < 28 + cons_y-53; y++) {
			for (x = 8; x < 8 + cons_x-16; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + cons_y-53; y < 28 + cons_y-37; y++) {
			for (x = 8; x < 8 + cons_x-16; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + cons_x-16, 28 + cons_y-37);
	}
	cons->cur_x = 8;
	return;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
	char *s;
	int in;
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0) {
		cmd_dir(cons);
	} else if (strncmp(cmdline, "type ", 5) == 0) {
		cmd_type(cons, fat, cmdline);
	} else if (strcmp(cmdline, "run_num") == 0){
		sprintf(s,"now running tasks: %d\n",taskctl->now_tasks);
		cons_putstr0(cons, s);
	} else if (strcmp(cmdline, "ps") == 0) {
		cmd_ps(cons);
	} else if (strcmp(cmdline, "wait_num") == 0){
		sprintf(s,"now waiting tasks: %d\n",taskctl->wait_num);
		cons_putstr0(cons, s);
	} else if (strncmp(cmdline, "pause ", 6) == 0){
		in=atoi(cmdline+6);
		cmd_pause(cons,in);
	} else if (strncmp(cmdline, "run ",4) == 0){
		in=atoi(cmdline+4);
		cmd_run(cons,in);
	} else if (strncmp(cmdline, "wakeup ",7) == 0){
		in=atoi(cmdline+7);
		cmd_wakeup(cons,in);
	} else if (strncmp(cmdline, "kill ",5) == 0){
		in=atoi(cmdline+5);
		cmd_kill(cons,in);
	} else if (strncmp(cmdline, "ls", 2) == 0) {
	cmd_ls(cons, cmdline);
	} else if (strncmp(cmdline, "cat ", 4) == 0) {
		cmd_cat(cons, cmdline);
	} else if (strncmp(cmdline, "mkfile ", 7) == 0) {
		cmd_mkfile(cons, cmdline);
	} else if (strncmp(cmdline, "open ", 5) == 0) {
		cmd_open(cons, cmdline);
	} else if (strncmp(cmdline, "close ", 6) == 0) {
		cmd_close(cons, cmdline);
	} else if (strncmp(cmdline, "echo ", 5) == 0) {
		cmd_echo(cons, cmdline);
	} else if (strcmp(cmdline, "pwd") == 0) {
		cmd_pwd(cons);
	} else if (strncmp(cmdline, "cd ", 3) == 0) {
		cmd_cd(cons, cmdline);
	} else if (strncmp(cmdline, "mkdir ", 6) == 0) {
		cmd_mkdir(cons, cmdline);
	} else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			cons_putstr0(cons, "Bad command.\n");
		}
	}
	return;
}

void cmd_mkdir(struct CONSOLE *cons, char *cmdline) {
	unsigned int return_ino;
	int status;
	char s[60];

	if ((status = syscall_create(cmdline + 6, 0, &return_ino))) {
		cons_putstr0(cons, "Couldn't make directory.\n");
		sprintf(s, "Status : %d\n", status);
		cons_putstr0(cons, s);
		return;
	}

	sprintf(s, "ino : %d\n", return_ino);
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}


void cmd_ls(struct CONSOLE *cons, char *cmdline) {
	FSEntry **entry;
	FileDescriptor *fd;
	int size, i, status;
	char s[60];

	if (strncmp(cmdline, "ls ", 3) == 0) {
		fd = syscall_open(cmdline + 3, 0);
	} else if (strcmp(cmdline, "ls") == 0) {
		fd = pwd_fd;
	}

	if ((status = syscall_get_fsentry(fd, entry, &size))) {
		sprintf(s, "Couldn't get FSEntry.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "total		%d in %s\n", size, fd->inode->entry->common.name);
	cons_putstr0(cons, s);
	for (i = 0; i < size; i++) {
		sprintf(s, "%s %d %s\n", (get_inode_by_no(entry[i]->common.ino)->mode == 0) ? "F" : "D" ,entry[i]->common.ino, entry[i]->common.name);
		cons_putstr0(cons, s);
	}

	syscall_close(fd);
	return;
}


void cmd_cd(struct CONSOLE *cons, char *cmdline) {
	FileDescriptor *prev_pwd_fd;
	char s[60];

	if (pwd_fd == NULL) {
		sprintf(s, "FD is not opened.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	prev_pwd_fd = pwd_fd;
	pwd_fd = syscall_open(cmdline + 3, 0);

	if (pwd_fd == NULL) {
		sprintf(s, "Couldn't Change Directory.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		pwd_fd = prev_pwd_fd;
		return;
	}

	cons_newline(cons);
	return ;
}


void cmd_pwd(struct CONSOLE *cons) {
	char s[60];

	if (pwd_fd == NULL) {
		sprintf(s, "FD is not opened.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "%s\n", pwd_fd->inode->entry->common.name);
	cons_putstr0(cons, s);
	cons_newline(cons);

	return;
}


void cmd_echo(struct CONSOLE *cons, char *cmdline) {
	int status;
	char s[60];

	if (common_fd == NULL) {
		sprintf(s, "FD is not opened.");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	if ((status = syscall_write(common_fd, strlen(cmdline + 5), cmdline + 5))) {
		sprintf(s, "Couldn't write data.");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "Wrote.");
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}


void cmd_close(struct CONSOLE *cons, char *cmdline) {
	int status;
	char s[60], *filename;

	strcpy(filename, common_fd->inode->entry->common.name);

	if (common_fd == NULL) {
		sprintf(s, "FD is not opened.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	status = syscall_close(common_fd);

	if (status != 0) {
		sprintf(s, "Couldn't close.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "Closed %s.\n", filename);
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}

void cmd_open(struct CONSOLE *cons, char *cmdline) {
	int status;
	char s[60];

	common_fd = syscall_open(cmdline + 5, 0);

	if (common_fd == NULL) {
		sprintf(s, "File Not Found.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "Opened %s.", common_fd->inode->entry->common.name);
	cons_putstr0(cons, s);
	cons_newline(cons);

	return;
}

void cmd_mkfile(struct CONSOLE *cons, char *cmdline) {
	unsigned int return_ino;
	int status;
	char s[60];

	if ((status = syscall_create(cmdline + 7, 1, &return_ino))) {
		cons_putstr0(cons, "Couldn't make file.\n");
		sprintf(s, "Status : %d\n", status);
		cons_putstr0(cons, s);
		return;
	}

	sprintf(s, "ino : %d\n", return_ino);
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}

// void cmd_echo(struct CONSOLE *cons, char *cmdline) {
// 	FileDescriptor *fd = syscall_
// }

void cmd_cat(struct CONSOLE *cons, char *cmdline) {
	// struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int status;
	char s[60], src[16];

	FileDescriptor *fd = syscall_open(cmdline + 4, 0);

	if (fd == NULL) {
		sprintf(s, "File Not Found.\n");
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	if ((status = syscall_read(fd, 16, 0, src))) {
		sprintf(s, "Couldn't Read Data.\n");
		cons_putstr0(cons, s);
		sprintf(s, "Status : %d\n", status);
		cons_putstr0(cons, s);
		cons_newline(cons);
		return;
	}

	sprintf(s, "%s", test_find_block(fd));
	cons_putstr0(cons, s);
	cons_newline(cons);

	syscall_close(fd);

	return;
}

void cmd_run(struct CONSOLE *cons,int in){
	char *s;
	int i;
	for(i=0;i<MAX_TASKS;i++){
		if(taskctl->tasks0[i].pid == in){
			sprintf(s,"pid:%d already exists\n",in);
			cons_putstr0(cons, s);
			return;
		}
	}
	if(in>=3 && in<=7){
		task_run(task_b_make(in), 2, 1);
		sheet_updown(sht_win_b[in-3], 1);
	}else{
		sprintf(s,"you can't run pid:%d\n",in);
		cons_putstr0(cons, s);
	}
	return ;
}

void cmd_kill(struct CONSOLE *cons,int in){
	char *s;
	int i;
	if(in<=2){
		sprintf(s,"you can't kill pid:%d\n",in);
		cons_putstr0(cons, s);
		return;
	}
	for(i=0;i<1000;i++){
		if(taskctl->tasks0[i].pid == in){
			if(in>=3 && in<=7){
				sprintf(s, "task_b pid:%d", in);
				make_window8(sht_win_b[in-3]->buf, 344, 52, s, 0);
				sheet_updown(sht_win_b[in-3], -1);
			}
			task_clean(&taskctl->tasks0[i]);
			return;
		}
	}
	sprintf(s,"pid:%d dose not exist\n",in);
	cons_putstr0(cons, s);
	return ;
}

void cmd_wakeup(struct CONSOLE *cons,int in){
	int i;
	char *s;
	for(i=0;i<MAX_TASKS;i++){
		if(taskctl->tasks0[i].pid == in){
				task_run(&taskctl->tasks0[i],-1,-200);
				return;
		}
	}
	sprintf(s,"pid:%d dose not exist\n",in);
	cons_putstr0(cons, s);
	return;
}

void cmd_pause(struct CONSOLE *cons,int in){
	int i;
	char *s;
	for(i=0;i<MAX_TASKS;i++){
		if(taskctl->tasks0[i].pid == in){
			task_sleep(&taskctl->tasks0[i],TASK_OTHER_WAITING);
			return;
		}
	}
	sprintf(s,"pid:%d dose not exist\n",in);
	cons_putstr0(cons, s);
	return ;
}

void cmd_ps(struct CONSOLE *cons){
	int i,cnt = 0;
	char *c,*s;
	for(i=0;i<MAX_TASKS;i++){
		if(taskctl->tasks0[i].flags != TASK_NONUSE){
				cnt++;
				switch(taskctl->tasks0[i].flags){
					case TASK_SETTING:
						c="SETING";
						break;
					case TASK_SLEEPING:
						c="SLEEP";
						break;
					case TASK_RUNNING:
						c="RUN";
						break;
					case TASK_HDD_WAITING:
						c="WAIT";
						break;
					case TASK_HDD_USING:
						c="HDD USE";
						break;
					case TASK_OTHER_WAITING:
						c="OTHER";
						break;
					default:
						c="ERROR";
				}
				sprintf(s,"pid:%3d level:%2d state:%-6s ",taskctl->tasks0[i].pid,taskctl->tasks0[i].level,c);
				cons_putstr0(cons, s);
				sprintf(s,"name:");//"%-10s",taskctl->tasks0[i].name);
				strcat(s,taskctl->tasks0[i].name);
				cons_putstr0(cons,s);
				cons_newline(cons);
		}
	}
	sprintf(s,"running tasks:%d\n",taskctl->now_tasks);
	cons_putstr0(cons, s);
	sprintf(s,"total tasks:%d\n",cnt);
	cons_putstr0(cons, s);
	return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
	return;
}

void cmd_dir(struct CONSOLE *cons)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	if (finfo != 0) {
		/* �t�@�C�������������ꍇ */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* �t�@�C�����������Ȃ������ꍇ */
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	char name[18], *p, *q;
	struct TASK *task = task_now(),*app_task;
	int cnt=0;
	int i, segsiz, datsiz, esp, dathrb;

	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* �Ƃ肠�����t�@�C�����̌�����0�ɂ��� */

	/* �t�@�C�����T�� */
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* �������Ȃ������̂Ō�����".HRB"�����Ă������x�T���Ă݂� */
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		/* �t�@�C�������������ꍇ */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			segsiz = *((int *) (p + 0x0000));
			esp    = *((int *) (p + 0x000c));
			datsiz = *((int *) (p + 0x0010));
			dathrb = *((int *) (p + 0x0014));
			q = (char *) memman_alloc_4k(memman, segsiz);
			*((int *) 0xfe8) = (int) q;
			set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + 1004, segsiz - 1,      (int) q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datsiz; i++) {
				q[esp + i] = p[dathrb + i];
			}
			app_task = task_alloc();
			for(i=0;i<MAX_TASKS;i++){
				if(taskctl->tasks0[i].pid >= 100){
						cnt++;
				}
			}
			app_task->pid = 100+cnt;
			app_task->name = "UserTask";
			app_task->tss.esp =  memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 -4;
			app_task->tss.eip = (int) &app_main;
			app_task->tss.es = 1 * 8;
			app_task->tss.cs = 2 * 8;
			app_task->tss.ss = 1 * 8;
			app_task->tss.ds = 1 * 8;
			app_task->tss.fs = 1 * 8;
			app_task->tss.gs = 1 * 8;
			*((int *) (app_task->tss.esp + 4)) = esp;
			task_run(app_task,2,2);
			memman_free_4k(memman, (int) q, segsiz);
			task_sleep(task,TASK_SLEEPING); //ユーザコマンドのためにコマンド表示を一時的に停止
		} else {
			cons_putstr0(cons, ".hrb file format error.\n");
		}
		memman_free_4k(memman, (int) p, finfo->size);
		//cons_newline(cons);
		return 1;
	}
	/* �t�@�C�����������Ȃ������ꍇ */
	return 0;
}

void app_main(int esp){
	struct TASK *task = task_now();
	int i;
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	start_app(0x1b, 1003 * 8, esp, 1004 * 8,&(task->tss.esp0));
	cons_newline(cons);
	task_exit();
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	int cs_base = *((int *) 0xfe8);
	struct TASK *task = task_now();
	FileDescriptor *fd;
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	int *reg = &eax + 1;	/* eaxの次の番地 */
	/* 保存のためのPUSHADを強引に書き換える */
	/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
	/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if (edx == 2) {
    cons_putstr0(cons, (char *) ebx + cs_base);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx + cs_base, ecx);
	} else if (edx == 4) {
		return &(task->tss.esp0);
	}	else if (edx == 5){
		//getpid;
		reg[7] = sys_getpid();
	}	else if (edx == 6){
		//int_str
		char *s;
		sprintf(s,"%d\n",eax);
		cons_putstr0(cons,s);
	}	else if (edx == 7){
		//sleep;
		char *s;
		sys_sleep(eax);
	}	else if (edx == 8){
		//exit
		sys_exit();
	} else if (edx == 9){
		//open
		fd == syscall_open((char *) ebx + cs_base,eax); //アプリのデータセグメントの番地を加算
		reg[7] = (int)fd;
	}else if (edx == 10){
		//write
		reg[7] = syscall_write((FileDescriptor *)eax,ebx,(char *)ecx + cs_base);
	}else if (edx == 11){
		//read
		reg[7] = syscall_read((FileDescriptor *)eax,ebx,0,(char *)ecx + cs_base);
	}else if (edx == 12){
		//close
		reg[7] = syscall_close((FileDescriptor *)eax);
	}else if (edx == 13){
		unsigned int ino;
		reg[7] = syscall_create((char *)ebx + cs_base ,1,&ino);
	}
	return 0;
}

int *inthandler0d(int *esp)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct TASK *task = task_now();
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	return &(task->tss.esp0);	/* �ُ��I�������� */
}
