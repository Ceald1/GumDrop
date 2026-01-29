#include <linux/init.h>   /* Needed for the macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* Needed by all modules */

///< The license type -- this affects runtime behavior
MODULE_LICENSE("GPL");

///< The author -- visible when you use modinfo
MODULE_AUTHOR("Ceald");

///< The description -- see modinfo
MODULE_DESCRIPTION("gumdrop stable");

///< The version of the module
MODULE_VERSION("0.02");

static int __init _start(void) {
  printk(KERN_INFO "Loading hello module...\n");
  printk(KERN_INFO "Hello world\n");
  return 0;
}

static void __exit _end(void) { printk(KERN_INFO "Goodbye Mr.\n"); }

module_init(_start);
module_exit(_end);
