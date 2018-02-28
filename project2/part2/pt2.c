#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <asm/current.h>
#include <asm/errno.h>

#define UID_START 1000

typedef struct ancestry_t {
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

typedef struct task_struct task_struct;

unsigned long **sys_call_table;
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short* target_pid, struct ancestry_t* response);

void rec_seek_anc(struct ancestry_t* response_copy_ptr, unsigned short target_pid, task_struct* ancestor, int ancestor_ctr) {
	pid_t temp_pid = ancestor->pid;
	response_copy_ptr->ancestors[ancestor_ctr++] = temp_pid;
	printk(KERN_INFO "Process %ld is an ancestor of target process %ld", temp_pid, target_pid);
	if (temp_pid == 1 || temp_pid == 0) {
		return;
	}
	ancestor = ancestor->parent;
	rec_seek_anc(response_copy_ptr, target_pid, ancestor, ancestor_ctr);
}

asmlinkage long new_sys_cs3013_syscall2(unsigned short* target_pid, struct ancestry_t* response) {	
	pid_t temp_pid;
	unsigned short target_pid_copy;
	struct ancestry_t response_copy;
	struct ancestry_t* response_copy_ptr = &response_copy;

	int ancestor_ctr = 0;
	int child_ctr = 0;
	int sib_ctr = 0;

	struct list_head* child_task_head;
	struct list_head* sib_task_head;

	struct task_struct* pos;
	struct task_struct* ancestor_task;
	struct task_struct* child_task;
	struct task_struct* sib_task;

	if (copy_from_user(&target_pid_copy, target_pid, sizeof(unsigned short))) {
		return EFAULT;
	}
	if (copy_from_user(response_copy_ptr, response, sizeof(struct ancestry_t))) {
		return EFAULT;
	}

	struct task_struct* target_task = pid_task(find_vpid(target_pid_copy), PIDTYPE_PID);
	if (target_task == NULL) {
		printk(KERN_INFO "Target process with pid %ld not found, terminating");
		return -2;
	}
	printk(KERN_INFO "Target process with pid %ld found", (long int) *target_pid);

	list_for_each_entry(pos, &(target_task->children), sibling) {
		temp_pid = pos->pid;
		printk(KERN_INFO "Process %ld is a child of target process %ld", (long int) temp_pid, (long int) *target_pid);
		response_copy_ptr->children[child_ctr++] = temp_pid;
	}

	list_for_each_entry(pos, &(target_task->sibling), sibling) {
		if (pos->pid == 0) {
			break;
		}
		temp_pid = pos->pid;
		printk(KERN_INFO "Process %ld is a sibling of target process %ld", (long int) temp_pid, (long int) *target_pid);
		response_copy_ptr->siblings[sib_ctr++] = temp_pid;
	}

	if (target_task->pid != 1) {
		ancestor_task = target_task->parent;
		rec_seek_anc(response_copy_ptr, target_pid, ancestor_task, ancestor_ctr);
	}

	if (copy_to_user(response, response_copy_ptr, sizeof(struct ancestry_t))) {
		return EFAULT;
	}
	printk(KERN_INFO "Copying process ancestry to user successful");
	return 0;
}

static unsigned long **find_sys_call_table(void) {
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;
	while (offset < ULLONG_MAX) {
		sct = (unsigned long **) offset;
		if (sct[__NR_close] == (unsigned long *) sys_close) {
			printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
					(unsigned long) sct);
			return sct;
		}
		offset += sizeof(void *);
	}
	return NULL;
}

static void disable_page_protection(void) {
	/*
	 Control Register 0 (cr0) governs how the CPU operates.
	 Bit #16, if set, prevents the CPU from writing to memory marked as
	 read only. Well, our system call table meets that description.
	 But, we can simply turn off this bit in cr0 to allow us to make
	 changes. We read in the current value of the register (32 or 64
	 bits wide), and AND that with a value where all bits are 0 except
	 the 16th bit (using a negation operation), causing the write_cr0
	 value to have the 16th bit cleared (with all other bits staying
	 the same. We will thus be able to write to the protected memory.
	 It’s good to be the kernel!
	 */
	write_cr0(read_cr0() & (~0x10000));
}

static void enable_page_protection(void) {
	/*
	 See the above description for cr0. Here, we use an OR to set the
	 16th bit to re-enable write protection on the CPU.
	 */
	write_cr0(read_cr0() | 0x10000);
}

static int __init interceptor_start(void) {
	/* Find the system call table */
	if(!(sys_call_table = find_sys_call_table())) {
		/* Well, that didn’t work. Cancel the module loading step. */
		printk(KERN_INFO "Searching for syscall table failed");
		return -1;
	}
	
	/* Store a copy of all the existing functions */
	ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

	/* Replace the existing system calls */
	disable_page_protection();

	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;

	enable_page_protection();
	/* And indicate the load was successful */
	printk(KERN_INFO "Loaded interceptor!");
	return 0;
}

static void __exit interceptor_end(void) {
	/* If we don’t know what the syscall table is, don’t bother. */
	if(!sys_call_table) {
		return;
	}

	/* Revert all system calls to what they were before we began. */
	disable_page_protection();

	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;

	enable_page_protection();
	printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
