#include "cpu.h"
#include "kernel.h"
#include "proc.h"
#include "util/list.h"

STATIC_LIST(procs);
static struct proc *next;

void sched_add(struct proc *proc)
{
	list_add_back(procs, proc);
}

void sched_remove(struct proc *proc)
{
	list_entry_t *cur;
	list_iterate(cur, procs) {
		if (cur->data == proc) {
			list_del_entry(procs, cur);
			return;
		}
	}
}

void sched_update(void)
{
	struct proc *cur = cpu_cur_proc();
	if (!cur || cur->ticks-- <= 0 || cur->status == PROC_BLOCKED) {
		if(cur) cur->ticks = PROC_TICKS;
		next = list_front(procs);
		printk(KERN_INFO "sched: next process is '%s'", next->cmdline);
	}
	else {
		next = cur;
	}
}

void sched_put(struct proc *proc)
{
	if (proc) {
		sched_add(proc);
	}
}

struct proc *sched_get(void)
{
	sched_remove(next);

	return next;
}
