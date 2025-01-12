#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static int interval = 10;
module_param(interval, int, 0644);
MODULE_PARM_DESC(interval, "Interval between checks in seconds");
static struct timer_list cpu_usage_timer;
static unsigned long prev_total[6]={0,0,0,0,0,0};
static unsigned long prev_idle[6]={0,0,0,0,0,0};
static void calculate_cpu(struct timer_list *timer){
    struct file *file;
    char buf[1024];
    char *line;
    char cpu_name[10];
    loff_t position = 0;
    unsigned long user, nice, system, idle, iowait, irq, softirq, total, active, cpu_usage, soft, steal, guest;
    int i = 0;

    file = filp_open("/proc/stat", O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open /proc/stat, error: %ld\n", PTR_ERR(file));
        mod_timer(&cpu_usage_timer, jiffies +interval *HZ);
        return;
    }
    if (kernel_read(file, buf, sizeof(buf), &position) < 0) {
        printk(KERN_ERR "Failed to read /proc/stat");
        filp_close(file, NULL);
        return;
    }

        filp_close(file, NULL);
        line = buf;
        int index = 0;

        while (i < 6 && sscanf(line, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu%n", cpu_name, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &soft, &steal, &guest, &index) == 11){
            int number_of_params = sscanf(line, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu%n", cpu_name, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &soft, &steal, &guest, &index);
            line+=index;
            printk(KERN_INFO "[%d, cpu: %s, user: %lu, nice: %lu, system: %lu, idle: %lu, iowait: %lu, irq: %lu, softirq: %lu]\n", number_of_params, cpu_name, user, nice, system, idle, iowait, irq, softirq);
            total = user + nice + system +idle + iowait + irq + softirq;
            active = user + nice + system;
            if (strcmp(cpu_name, "intr") == 0) {
                printk(KERN_INFO "Broken");
                break;
            }

            if (prev_total[i] != 0){
                unsigned long delta_total = total - prev_total[i];
                unsigned long delta_idle = idle - prev_idle[i];

                if (delta_total > 0){
                    cpu_usage = 100 * (delta_total - delta_idle) / delta_total;
                    printk(KERN_INFO "CPU Usage: %s %lu%% [Total: %lu, Active: %lu, Idle: %lu]\n",cpu_name, cpu_usage, total, active, idle);
                }
            }
            prev_total[i] = total;
            prev_idle[i] = idle;
            i++;

    }
    printk(KERN_INFO "The end of the loop");
    mod_timer(&cpu_usage_timer, jiffies + interval * HZ);

}

static int __init cpu_usage_init(void) {
    printk(KERN_INFO "CPU Monitor Kernel Module Loaded\n");
    timer_setup(&cpu_usage_timer, calculate_cpu, 0);
    mod_timer(&cpu_usage_timer, jiffies + interval * HZ);
    return 0;
}

static void __exit cpu_usage_exit(void) {
    printk(KERN_INFO "CPU Monitor Kernel Module Unloaded\n");
    del_timer(&cpu_usage_timer);
}



module_init(cpu_usage_init);
module_exit(cpu_usage_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maria Filipczak Pawel Gryszczyk");
MODULE_DESCRIPTION("Im so tired after this...");
