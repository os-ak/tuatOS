/* ??ｿｽ?ｿｽ}??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ`??ｿｽ?ｿｽ^??ｿｽ?ｿｽX??ｿｽ?ｿｽN??ｿｽ?ｿｽﾖ係 */

#include "bootpack.h"
#include <stdio.h>

struct TASKCTL *taskctl;
struct TIMER *task_timer;
extern struct SHEET *sht_win_b[5],*sht_back;
unsigned int switch_count;

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
	task->flags = TASK_RUNNING; /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
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
	switch_count=0;

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
	task->flags = TASK_RUNNING;	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ}??ｿｽ?ｿｽ[??ｿｽ?ｿｽN */
	task->priority = 2; /* 0.02??ｿｽ?ｿｽb */
	task->level = 0;	/* ??ｿｽ?ｿｽﾅ搾ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽx??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
	task_add(task);
	task_switchsub();	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽx??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾝ抵ｿｽ */
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
			task->flags = TASK_SETTING; /* ??ｿｽ?ｿｽg??ｿｽ?ｿｽp??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ}??ｿｽ?ｿｽ[??ｿｽ?ｿｽN */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* ??ｿｽ?ｿｽﾆりあ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ0??ｿｽ?ｿｽﾉゑｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾆにゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
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
	return 0; /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽS??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽg??ｿｽ?ｿｽp??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
}

void task_run(struct TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level; /* レベルを変更しない */
	}
	if (priority > 0) {
		task->priority = priority;
	}

	if (task->flags == TASK_RUNNING && task->level != level) { /* 動作中のレベルの変更 */
		task_remove(task); /* これを実行するとflagsは1になるので下のifも実行される */
	}
	if (task->flags != TASK_RUNNING) {
		/* スリープから起こされる場合 */
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1; /* 次回タスクスイッチのときにレベルを見直す */
	return;
}

void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	/* task??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾇゑｿｽ??ｿｽ?ｿｽﾉゑｿｽ??ｿｽ?ｿｽ驍ｩ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽT??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾉゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
			break;
		}
	}

	tl->running--;
	taskctl->now_tasks--;
	if (i < tl->now) {
		tl->now--; /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌで、??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ?ｿｽ?ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
	}
	if (tl->now >= tl->running) {
		/* now??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾈ値??ｿｽ?ｿｽﾉなゑｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽA??ｿｽ?ｿｽC??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
		tl->now = 0;
	}
	task->flags = TASK_SLEEPING; /* ??ｿｽ?ｿｽX??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ[??ｿｽ?ｿｽv??ｿｽ?ｿｽ??ｿｽ?ｿｽ */

	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ轤ｵ */
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}

void task_sleep(struct TASK *task,int flg)
{
	struct TASK *now_task;

	if (task->flags == TASK_RUNNING) {
		/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
		now_task = task_now();
		task_remove(task); /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽs??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽflags??ｿｽ?ｿｽ??ｿｽ?ｿｽ1??ｿｽ?ｿｽﾉなゑｿｽ */
		if(flg!=TASK_SLEEPING){
			task->flags=flg;
			taskctl->wait0[taskctl->wait_num]= task;
			taskctl->wait_num++;
		}
		if (task == now_task) {
			/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽg??ｿｽ?ｿｽﾌス??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ[??ｿｽ?ｿｽv??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌで、??ｿｽ?ｿｽ^??ｿｽ?ｿｽX??ｿｽ?ｿｽN??ｿｽ?ｿｽX??ｿｽ?ｿｽC??ｿｽ?ｿｽb??ｿｽ?ｿｽ`??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽK??ｿｽ?ｿｽv */
			task_switchsub();
			now_task = task_now(); /* ??ｿｽ?ｿｽﾝ抵ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾅの、??ｿｽ?ｿｽu??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾝのタ??ｿｽ?ｿｽX??ｿｽ?ｿｽN??ｿｽ?ｿｽv??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ轤､ */
			farjmp(0, now_task->sel);
		}
	}
	return;
}

void task_clean(struct TASK *task){
	struct TASK *now_task;
	if (task->flags == TASK_RUNNING) {
		/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
		task_remove(task);
	}
	task->flags = TASK_NONUSE;
	task->priority = 2;
	task->level = 2;
	task->pid = -1;
	//??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾔゑｿｽ???ｿｽ?ｿｽ??ｿｽ?ｿｽZ??ｿｽ?ｿｽb??ｿｽ?ｿｽg
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
		/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽg??ｿｽ?ｿｽﾌ削除??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾌで、??ｿｽ?ｿｽ^??ｿｽ?ｿｽX??ｿｽ?ｿｽN??ｿｽ?ｿｽX??ｿｽ?ｿｽC??ｿｽ?ｿｽb??ｿｽ?ｿｽ`??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽK??ｿｽ?ｿｽv */
		task_switchsub();
		now_task = task_now(); /* ??ｿｽ?ｿｽﾝ抵ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾅの、??ｿｽ?ｿｽu??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾝのタ??ｿｽ?ｿｽX??ｿｽ?ｿｽN??ｿｽ?ｿｽv??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾄゑｿｽ??ｿｽ?ｿｽ轤､ */
		farjmp(0, now_task->sel);
	}
	return;
}

void task_switchsub(void)
{
	int i;
	/* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾔ擾ｿｽ??ｿｽ?ｿｽ?ｿｽ???ｿｽ?ｿｽ??ｿｽ?ｿｽx??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽT??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; /* ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽﾂゑｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ??ｿｽ?ｿｽ */
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
		switch_count++;
		farjmp(0, new_task->sel);
	}
	return;
}

void catch_hdd(int flg, int in1,int in2,void* in3){
	struct TASK *task = task_now();
	int i;
	char *s;
	task->h_data.flg = flg;
	task->h_data.in1 = in1;
	task->h_data.in2 = in2;
	task->h_data.in3 = in3;
	if(taskctl->wait_num==0){
		start_hdd(flg, in1,in2,in3);
		putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING0", 11);
		task_sleep(task,TASK_HDD_USING);//本来はこの前にハードディスク処理が入る
		return;
	}else{
		for(i=0;i<taskctl->wait_num;i++){
			if(taskctl->wait0[i]->flags==TASK_HDD_USING){
				putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_WAITING", 11);
				task_sleep(task,TASK_HDD_WAITING);
				return;
			}
		}
		putfonts8_asc_sht(sht_win_b[task->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING2", 11);
		start_hdd(task->h_data.flg, task->h_data.in1,task->h_data.in2,task->h_data.in3);
		task_sleep(task,TASK_HDD_USING);//本来はこの前にハードディスク処理が入る
	}
	return ;
}

void end_hdd(){
	int i;
	struct TASK *task;
	if(taskctl->wait_num > 1){
		//どこにいるのか
		for(i=0;i<taskctl->wait_num;i++){
				if(taskctl->wait0[i]->flags==TASK_HDD_USING){
					break;
				}
		}
		taskctl->wait_num--;
		task_run(taskctl->wait0[i],-1,-1);
		/* ずらし */
		for (; i < taskctl->wait_num; i++) {
			taskctl->wait0[i] = taskctl->wait0[i+1];
		}
		taskctl->wait0[0]->flags=TASK_HDD_USING;
		putfonts8_asc_sht(sht_win_b[taskctl->wait0[0]->pid-3], 24, 28, COL8_000000, COL8_C6C6C6, "HDD_USING3", 11);
		start_hdd(taskctl->wait0[0]->h_data.flg,taskctl->wait0[0]->h_data.in1,taskctl->wait0[0]->h_data.in2,taskctl->wait0[0]->h_data.in3);
	}else if(taskctl->wait_num == 1){
		taskctl->wait_num--;
		task = taskctl->wait0[0];
		taskctl->wait0[0]= NULL;
	  task_run(task,-1,-1);
	}
	return;
}

void task_exit(){
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
