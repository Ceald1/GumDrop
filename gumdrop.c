#include <linux/init.h>   /* Needed for the macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* Needed by all modules */

///< The license type -- this affects runtime behavior
MODULE_LICENSE("GPL");

///< The author -- visible when you use modinfo
MODULE_AUTHOR("ceald");

///< The description -- see modinfo
MODULE_DESCRIPTION("gumdrop stable");

///< The version of the module
MODULE_VERSION("0.2");

static int __init gum_start(void) {
  printk(KERN_INFO "Loading module...\n");
  printk(KERN_INFO "Hewo pwincess\n");
  return 0;
}

static void __exit gum_end(void) { printk(KERN_INFO "Goodbye pwincess\n"); }

module_init(gum_start);
module_exit(gum_end);
