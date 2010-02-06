#include <string.h>
#include <libutil.h>
#include "cpu.h"
#include "error.h"
#include "gdt.h"
#include "kernel.h"
#include "mm.h"
#include "proc.h"
#include "util/list.h"

// holds all procs
STATIC_LIST(procs);

struct proc *proc_idle;

/* Returns a new process id */
static pid_t newpid(void)
{
	static pid_t next = 0;
	return next++;
}

static bool create_stacks(struct proc *proc, uintptr_t entry, bool usermode)
{
	// allocate and map the stack
	paddr_t kstack_phys = mm_alloc_page();
	if (kstack_phys == NO_PAGE) {
		printk(KERN_ERR "pm: failed to allocate the kernel stack.");
		return false;
	}

	vaddr_t kstack_addr = vm_alloc_kernel_addr(kstack_phys, PAGE_SIZE);
	if (!kstack_addr) {
		printk(KERN_ERR "pm: failed to map the kernel stack.");
		goto fail_kstack;
	}

	paddr_t ustack_phys = mm_alloc_page();
	if (ustack_phys == NO_PAGE) {
		printk(KERN_ERR "pm: failed to allocate the user stack");
		goto fail_ustackp;
	}

	int err = vm_map(proc->context, ustack_phys, (vaddr_t)((uintptr_t)USER_STACK_ADDR - (PAGE_SIZE-1)),
	                 MMF_USER_FLAGS, 1);

	if (err != OK) {
		printk(KERN_ERR "pm: failed to map the user stack");
		goto fail_ustack;
	}

	// prepare the kernel stack

	uintptr_t kstack = (uintptr_t)kstack_addr + PAGE_SIZE - sizeof(struct stack_frame);
	struct stack_frame *st = (struct stack_frame *)kstack;

	memset(st, 0, sizeof(*st));
	st->u_esp = (uintptr_t)USER_STACK_ADDR;
	st->eflags = PROC_INIT_FLAGS;

	uint32_t code, data;
	if (usermode) {
		code = GDT_SEL_UCODE | USER_DPL;
		data = GDT_SEL_UDATA | USER_DPL;
	}
	else {
		code = GDT_SEL_CODE | KERN_DPL;
		data = GDT_SEL_DATA | KERN_DPL;
	}
	st->cs = code;
	st->ds = data;
	st->es = data;
	st->fs = data;
	st->gs = data;
	st->u_ss = data;

	st->eip = entry;

	proc->kstack.phys = kstack_phys;
	proc->kstack.virt = (vaddr_t)kstack;
	proc->ustack.phys = ustack_phys;
	proc->ustack.virt = USER_STACK_ADDR;

	return true;

fail_ustack:
	mm_free_page(ustack_phys);
fail_ustackp:
	vm_free_kernel_addr(kstack_addr, PAGE_SIZE);
fail_kstack:
	mm_free_page(kstack_phys);
	return false;
}

struct proc *proc_create(struct proc *parent, const char *cmdline, uintptr_t entry,
                         struct mm_context *context, bool usermode)
{
	struct proc *proc = kmalloc(sizeof(*proc));
	if (!proc) {
		printk(KERN_ERR "pm: failed to alloc memory for another process.");
		return NULL;
	}
	memset(proc, 0, sizeof(*proc));

	proc->pid    = newpid();
	proc->parent = parent;
	proc->status = PROC_BLOCKED;

	proc->priority = 1;
	proc->ticks = 5;

	bool new_context = false;
	if (!context) {
		proc->context = mc_create();
		new_context = true;
		if (!proc->context) {
			printk(KERN_ERR "pm: failed to create context.");
			goto fail_context;
		}
	}

	proc->cmdline = strdup(cmdline);
	if (!proc->cmdline) {
		printk(KERN_ERR "pm: failed to dup the cmdline.");
		goto fail_cmdline;
	}

	if (!create_stacks(proc, entry, usermode)) {
		printk(KERN_ERR "pm: failed to create process stacks.");
		goto fail_stack;
	}

	list_add_back(procs, proc);
	return proc;

fail_stack:
	kfree(proc->cmdline);
fail_cmdline:
	if (new_context)
		mc_destroy(proc->context);
fail_context:
	kfree(proc);
	return NULL;
}

void proc_destroy(struct proc *proc)
{
	sched_remove(proc);
	list_entry_t *cur;
	list_iterate(cur, procs) {
		if (cur->data == proc) {
			list_del_entry(procs, cur);
			break;
		}
	}

	//vm_free_kernel_addr(proc->kstack.virt, PAGE_SIZE);
	//mm_free_page(proc->kstack.phys);
	mm_free_page(proc->ustack.phys);

	kfree(proc->cmdline);
	mc_destroy(proc->context);
	kfree(proc);
}

bool proc_block(struct proc *proc)
{
	if (proc->status == PROC_BLOCKED)
		return false;

	proc->status = PROC_BLOCKED;
	sched_remove(proc);
	return true;
}

bool proc_unblock(struct proc *proc)
{
	if (proc->status != PROC_BLOCKED)
		return false;

	proc->status = PROC_READY;
	sched_add(proc);
	return true;
}

int init_pm(void)
{
	proc_idle = NULL; //TODO: idle process
	current = NULL;
	return OK;
}
