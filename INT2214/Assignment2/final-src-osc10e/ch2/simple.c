/**
 * simple.c
 *
 * A simple kernel module. 
 * 
 * To compile, run makefile by entering "make"
 *
 * Operating System Concepts - 10th Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hash.h> // Define constant value GOLDEN_RATIO_PRIME
#include <linux/gcd.h> // Conrain gcd(a, b) function
#include <asm/param.h> // To get Hz value
#include <linux/jiffies.h> // To get jiffies value
/* This function is called when the module is loaded. */
int simple_init(void)
{      printk(KERN_INFO "Loading Kernel Module\n");

       printk(KERN_INFO "%lu\n", GOLDEN_RATIO_PRIME); // Print GOLDEN_RATIO_PRIME

       printk(KERN_INFO "Jiffies in simple_init(): %lu\n", jiffies); // Print jiffies
       
       printk(KERN_INFO "%d\n", HZ); // Print Hz
       
       return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	printk(KERN_INFO "Removing Module\n");

       printk(KERN_INFO "%lu\n", gcd(3300, 24)); // Print GCD of 3300 and 24

       printk(KERN_INFO "Jiffies in simple_exit(): %lu\n", jiffies); // Print jiffies
}      

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");

