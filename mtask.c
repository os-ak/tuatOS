/* ?��}?��?��?��`?��^?��X?��N?��֌W */

#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
extern struct SHEET *sht_win_b[5],*sht_back;

struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	taskctl->now_tasks++;
	tl->running++;
	task->flags = TASK_RUNNING; /* ?��?��?��?�� */
	return;
}

void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = TASK_NONUSE;
		taskctl->tasks0[i].pid = -1;
		taskctl->tasks0[i].name = "NONE";
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}

	task = task_alloc();
	task->pid = 1;
	task->name = "OS";
	task->flags = TASK_RUNNING;	/* ?��?��?��?��?��}?��[?��N */
	task->priority = 2; /* 0.02?��b */
	task->level = 0;	/* ?��ō�?��?��?��x?��?�� */
	task_add(task);
	task_switchsub();	/* ?��?��?��x?��?��?��ݒ� */
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);

	idle = task_alloc();
	idle->pid = 0;
	idle->name = "idle";
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);

	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == TASK_NONUSE) {
			task = &taskctl->tasks0[i];
			task->flags = TASK_SETTING; /* ?��g?��p?��?��?��}?��[?��N */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* ?��Ƃ肠?��?��?��?��0?��ɂ�?��Ă�?��?��?��?��?��Ƃɂ�?��?�� */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* ?��?��?��?��?��S?��?��?��g?��p?��?�� */
}

void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* ?��?��?��x?��?��?��?��?��ύX?��?��?��Ȃ� */
	}
	if (priority > 0) {
		task->priority = priority;
	}

	if (task->flags == TASK_RUNNING && task->level != level) { /* ?��?��?��?��?���??��?��x?��?��?��̕ύX */
		task_remove(task); /* ?��?��?��?��?��?��?��?��?��s?��?��?��?��?��?��flags?��?��1?��ɂȂ�?��̂ŉ�?��?��if?��?��?��?��?��s?��?��?��?��?��?�� */
	}
	if (task->flags != TASK_RUNNING && task->flags != TASK_HDD_USING  && task->flags != TASK_OTHER_WAITING) {
		/* ?��X?��?��?��[?��v?��?��?��?��?��N?��?��?��?��?��?��?��?��?����? */
		task->level = level;
		task_add(task);
	}else if(priority == -100 && task->flags == TASK_HDD_USING){
		task->level = level;
		task_add(task);
	}else if(priority == -200 && task->flags == TASK_OTHER_WAITING){
		task_add(task);
	}
	taskctl->lv_change = 1; /* ?��?��?��?��?��^?��X?��N?��X?��C?��b?��`?��̂Ƃ�?���??��?��x?��?��?��?��?��?��?��?��?��?�� */
	return;
}

void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* task?��?��?��ǂ�?��ɂ�?��邩?��?��?��T?��?�� */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			/* ?��?��?��?��?��ɂ�?��?�� */
			break;
		}
	}

	tl->running--;
	taskctl->now_tasks--;
	if (i < tl->now) {
		tl->now--; /* ?��?��?��?��?��?��?��̂ŁA?��?��?��?��?��?��?��?��?�����?��Ă�?��?�� */
	}
	if (tl->now >= tl->running) {
		/* now?��?��?��?��?��?��?��?��?��Ȓl?��ɂȂ�?��Ă�?��?��?��?��?��A?��C?��?��?��?��?��?�� */
		tl->now = 0;
	}
	task->flags = TASK_SLEEPING; /* ?��X?��?��?��[?��v?��?�� */

	/* ?��?��?��炵 */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}

void task_sleep(struct TASK *task,int flg)
{
	struct TASK *now_task;

	if (task->flags == TASK_RUNNING || flg==TASK_HDD_USING) {
		/* ?��?��?��?��?��?��?��?��?��?��?��?�� */
		now_task = task_now();
		task_remove(task); /* ?��?��?��?��?��?��?��?��?��s?��?��?��?��?��?��flags?��?��1?��ɂȂ� */
		if(flg!=TASK_SLEEPING){
			if(flg == TASK_HDD_USING){
				start_hdd(task->h_data.flg, task->h_data.in1,task->h_data.in2,task->h_data.in3);
			}
			task->flags=flg;
			taskctl->wait0[taskctl->wait_num]= task;
			taskctl->wait_num++;
		}
		if (task == now_task) {
			/* ?��?��?��?��?��?��?��g?��̃X?��?��?��[?��v?��?��?��?��?��?��?��̂ŁA?��^?��X?��N?��X?��C?��b?��`?��?��?��K?��v */
			task_switchsub();
			now_task = task_now(); /* ?��ݒ�?��?��?��ł́A?��u?��?��?��݂̃^?��X?��N?��v?��?��?��?��?��?��?��Ă�?��炤 */
			farjmp(0, now_task->sel);
		}
	}
	return;
}

void task_clean(struct TASK *task){
	struct TASK *now_task;
	if (task->flags == TASK_RUNNING) {
		/* ?��?��?��?��?��?��?��?��?��?��?��?�� */
		task_remove(task);
	}
	task->flags = TASK_NONUSE;
	task->priority = 2;
	task->level = 2;
	task->pid = -1;
	//?��?��?��?��?��?��?��Ԃ�??��?��Z?��b?��g
	task->tss.eflags = 0x00000202; /* IF = 1; */
	task->tss.eax = 0;
	task->tss.ecx = 0;
	task->tss.edx = 0;
	task->tss.ebx = 0;
	task->tss.ebp = 0;
	task->tss.esi = 0;
	task->tss.edi = 0;
	task->tss.es = 0;
	task->tss.ds = 0;
	task->tss.fs = 0;
	task->tss.gs = 0;
	task->tss.ldtr = 0;
	task->tss.iomap = 0x40000000;
	now_task = task_now();
	if (task == now_task) {
		/* ?��?��?��?��?��?��?��g?��̍폜?��?��?��?��?��?��?��̂ŁA?��^?��X?��N?��X?��C?��b?��`?��?��?��K?��v */
		task_switchsub();
		now_task = task_now(); /* ?��ݒ�?��?��?��ł́A?��u?��?��?��݂̃^?��X?��N?��v?��?��?��?��?��?��?��Ă�?��炤 */
		farjmp(0, now_task->sel);
	}
	return;
}

void task_switchsub(void)
{
	int i;
	/* ?��?��?��ԏ�?���??��?��x?��?��?��?��?��T?��?�� */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; /* ?��?��?���?��?��?��?�� */
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) {
		tl->now = 0;
	}
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}

int catch_hdd(int flg, int in1,int in2,void* in3){
	struct TASK *task = task_now();
	int i;
	char *s;
	task->h_data.flg = flg;
	task->h_data.in1 = in1;
	task->h_data.in2 = in2;
	task->h_data.in3 = in3;
	task->h_data.status = 100;
	if(taskctl->wait_num==0){
		putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING0", 11);
		task_sleep(task,TASK_HDD_USING);
		sprintf(s, "return status: %d",task->h_data.status);
		putfonts8_asc_sht(sht_back, 700, 700, COL8_FFFFFF, COL8_008484, s, 40);
		return task->h_data.status;
	}else{
		for(i=0;i<taskctl->wait_num;i++){
			if(taskctl->wait0[i]->flags==TASK_HDD_USING){
				putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_WAITING", 11);
				task_sleep(task,TASK_HDD_WAITING);
				return task->h_data.status;
			}
		}
	  putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING2", 11);
		task_sleep(task,TASK_HDD_USING);
	}
	return task->h_data.status;
}

void end_hdd(int status){
	int i,j;
	char *s;
	struct TASK *task;
	for(j=0;j<MAX_TASKS;j++){
		if(taskctl->tasks0[j].flags == TASK_HDD_USING)
			task = &taskctl->tasks0[j];
	}
	if(taskctl->wait_num > 1){
		//?��ǂ�?��ɂ�?��?��?��̂�
		for(i=0;i<taskctl->wait_num;i++){
				if(taskctl->wait0[i]->flags==TASK_HDD_USING){
					break;
				}
		}
		taskctl->wait_num--;
		task = taskctl->wait0[i];
		task->h_data.status = status;
		task_run(task,-1,-100);
		/* ?��?��?��炵 */
		for (; i < taskctl->wait_num; i++) {
			taskctl->wait0[i] = taskctl->wait0[i+1];
		}
		taskctl->wait0[0]->flags=TASK_HDD_USING;
		//putfonts8_asc_sht(sht_win_b[taskctl->wait0[0]->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING3", 11);
		start_hdd(taskctl->wait0[0]->h_data.flg,taskctl->wait0[0]->h_data.in1,taskctl->wait0[0]->h_data.in2,taskctl->wait0[0]->h_data.in3);
	}else if (taskctl->wait_num == 1){
		taskctl->wait_num--;
		//task = taskctl->wait0[0];
		task->h_data.status = status;
		task_run(task,-1,-100);
	}
	return;
}

void task_exit(){
	char *s;
	struct TASK *task = task_now();
	task_clean(task);
}

void sys_exit(){
	task_exit();
}

int sys_getpid(){
	struct TASK *task = task_now();
	return task->pid;
}

void sys_sleep(int time){
	struct FIFO32 fifo;
	struct TIMER *timer;
	struct TASK *task = task_now();
	char *s;
	int fifobuf[128];
	fifo32_init(&fifo, 128, fifobuf, 0);
	fifo.task = task;
	timer = timer_alloc();
	timer_init(timer, &fifo, 9999);
	timer_settime(timer, time);
	task_sleep(task,TASK_SLEEPING);
	return;
}
