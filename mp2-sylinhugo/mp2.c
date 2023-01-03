#define LINUX

#include "mp2_given.h"
#include <linux/init.h>    // for __init and __exit
#include <linux/jiffies.h> // for timer setting
#include <linux/kernel.h>  // needed for pr_info()
#include <linux/kthread.h> // for spawning a new kernel thread
#include <linux/list.h>    // for list_head
#include <linux/module.h>  // needed by all modules
#include <linux/mutex.h>   // for mutex
#include <linux/proc_fs.h> // define procfs_read & procfs_write ...
#include <linux/sched.h>   // for task_struct and scheduler-related APIs
#include <linux/slab.h>    // kmalloc()
#include <linux/timer.h>   // for timer_setup
#include <linux/version.h> // for LINUX_VERSION_CODE
#include <uapi/linux/sched/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_ID");
MODULE_DESCRIPTION("CS-423 MP2");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define DEBUG 1
#define DIRECTORY "mp2"
#define ENTRY "status"
#define KMEM_NAME "mp2_task_struct"
#define DISPATCHING_THREAD_NAME "dispatching_thread"
#define PROCFS_MAX_SIZE 2048UL
#define ADMISSION_SCALE 1000
#define ADMISSION_LIMIT 693

// inline void self_set_task_state(struct task_struct *tsk, int state_value) {
//   do {
//     (tsk)->task_state_change = _THIS_IP_;
//     smp_store_mb((tsk)->__state, (state_value));
//   } while (0);
// }

// inline void set_current_state(struct task_struct *tsk, int state_value) {
//   do {
//     debug_normal_state_change((state_value));
//     smp_store_mb(tsk->__state, (state_value));
//   } while (0);
// }

#define self_set_current_state(tsk, state_value)                               \
  do {                                                                         \
    debug_normal_state_change((state_value));                                  \
    smp_store_mb(tsk->__state, (state_value));                                 \
  } while (0)

typedef enum mp2_task_state_enum { READY, RUNNING, SLEEPING } task_state;

// Follow the definition of handout
struct mp2_task_struct {
  struct task_struct *linux_task;
  struct timer_list wakeup_timer;
  struct list_head list;
  pid_t pid;
  unsigned long period_ms;  // Pi
  unsigned long runtime_ms; // Ci
  unsigned long deadline_jiff;
  task_state state;
};

struct proc_dir_entry *mp2_dir;
struct proc_dir_entry *status_entry;
struct kmem_cache *mp2_task_cache;
struct mp2_task_struct *running_task; // our main task
struct task_struct *dispatching_thread;

LIST_HEAD(mp2_task_list);
DEFINE_MUTEX(task_list_mutex);

struct mp2_task_struct *get_highest_priority_task(void) {
  struct mp2_task_struct *temp;
  struct mp2_task_struct *res;
  pr_info("Inside get_highest_priority_task() function");

  mutex_lock_interruptible(&task_list_mutex);
  // traverse a list and find the task that is READY with smallest period_ms
  list_for_each_entry(temp, &mp2_task_list, list) {
    printk("TEMP task: 0x%llx", (u64)temp);
    if (temp->state == READY) {
      printk("READY task: 0x%llx", (u64)temp);
      if (res == NULL)
        res = temp;
      else if (res != NULL && temp->period_ms < res->period_ms)
        res = temp;
    }
  }
  mutex_unlock(&task_list_mutex);

  pr_info("Leave get_highest_priority_task() function");
  return res;
}

int dispatch_thread_function(void *data) {
  struct mp2_task_struct *highest_priority_task;
  struct sched_param param;

  while (!kthread_should_stop()) {
    pr_info("Dispatched Thread be Triggered\n");
    highest_priority_task = get_highest_priority_task();
    pr_alert("highest_priority_task=0x%llx", (u64)highest_priority_task);

    // If we find the highest_priority_task successfully
    if (highest_priority_task != NULL) {
      pr_info("Find highest_priority_task successfully\n");
      // According to handout 6a, we only set the running_task to READY
      // when the state of running_task is RUNNING
      if (running_task && running_task->state == RUNNING) {
        pr_info("Also find the running task");
        running_task->state = READY;
        // running_task->linux_task->__state = TASK_INTERRUPTIBLE;
        param.sched_priority = 0;
        const struct sched_attr attr = {
            .sched_policy = SCHED_NORMAL,
            .sched_priority = param.sched_priority,
            .sched_nice = PRIO_TO_NICE(running_task->linux_task->static_prio),
        };
        sched_setattr_nocheck(running_task->linux_task, &attr);
        running_task = NULL;
      }
      pr_info("Going to schedule highest_priority_task");
      highest_priority_task->state = RUNNING;
      param.sched_priority = 99;
      const struct sched_attr attr = {
          .sched_policy = SCHED_FIFO,
          .sched_priority = param.sched_priority,
          .sched_nice =
              PRIO_TO_NICE(highest_priority_task->linux_task->static_prio),
      };
      sched_setattr_nocheck(highest_priority_task->linux_task, &attr);
      // wake_up_process set the highest_priority_task to TASK_RUNNING
      // and then enque this highest_priority_task to run queue
      wake_up_process(highest_priority_task->linux_task);
      running_task = highest_priority_task;

    } else if (highest_priority_task == NULL && running_task != NULL) {
      // We cannot find highest_priority_task successfully
      // We should simply preempt the currently running task
      pr_info("Cannot find highest_priority_task\n");
      pr_info("Simply preempt current running task");
      running_task->state = READY;
      param.sched_priority = 0;
      const struct sched_attr attr = {
          .sched_policy = SCHED_NORMAL,
          .sched_priority = param.sched_priority,
          .sched_nice = PRIO_TO_NICE(running_task->linux_task->static_prio),
      };
      sched_setattr_nocheck(running_task->linux_task, &attr);
    }
    pr_info("Leave Dispatched Thread Functions\n");
    // make dispatch_thread as INTERRUPTIBLE
    // which means dispatch_thread can go to sleep if we called schedule()
    set_current_state(TASK_INTERRUPTIBLE);
    // 1. schedule() will do context switch
    // schedule() move off dispathc_thread from run queue, so dispatch_thread go
    // to sleep now
    // 2. this help us find the task in queue with highest priority
    // because we have set new task as SCHED_FIFO
    // hence, kernel will context switch to that task
    schedule();
  }
  pr_info("kthread_should_stop() be called, we are going to unload the module");
  return 0;
}

int admisssion_control_check(unsigned long period_ms,
                             unsigned long runtime_ms) {
  unsigned long total_time = 0;
  struct mp2_task_struct *temp;
  pr_info("Inside admisssion_control_check()");

  mutex_lock_interruptible(&task_list_mutex);
  // traverse the list to sum up the time
  list_for_each_entry(temp, &mp2_task_list, list) {
    total_time += (temp->runtime_ms * ADMISSION_SCALE) / temp->period_ms;
  }
  mutex_unlock(&task_list_mutex);
  total_time += (runtime_ms * ADMISSION_SCALE) / period_ms;
  if (total_time > ADMISSION_LIMIT) {
    return -1;
  }

  pr_info("Leave admisssion_control_check()");
  return 1;
}

void _timer_callback(struct timer_list *data) {
  struct mp2_task_struct *task = from_timer(task, data, wakeup_timer);
  mutex_lock_interruptible(&task_list_mutex);
  pr_alert("Go in to timer handler");
  printk("Timer callback task=0x%llx", (u64)task);
  task->state = READY;
  // We cannot mod_timer here, we can only trigger a task
  // when the task has higher priority
  // if we mod_timer here, which means we follow time elapse, not by priority
  mutex_unlock(&task_list_mutex);
  wake_up_process(dispatching_thread);
  pr_info("Leave timer handler");
}

int registration_task(pid_t pid, unsigned long period_ms,
                      unsigned long runtime_ms) {
  struct mp2_task_struct *new_task;

  pr_info("Inside registration_task()");
  pr_info("registration_task() get pid: %d, period: %lu, runtime: %lu", pid,
          period_ms, runtime_ms);

  if (admisssion_control_check(period_ms, runtime_ms) == -1) {
    pr_err("new task doesn't pass admission control check!\n");
    return -1;
  }

  new_task = kmem_cache_alloc(mp2_task_cache, GFP_KERNEL);
  new_task->linux_task = find_task_by_pid(pid);
  new_task->pid = pid;
  new_task->period_ms = period_ms;
  new_task->runtime_ms = runtime_ms;
  new_task->deadline_jiff =
      jiffies +
      msecs_to_jiffies(period_ms); // jiffies is the current time of system
  new_task->state = SLEEPING;
  INIT_LIST_HEAD(&(new_task->list));

  if (!new_task->linux_task) {
    pr_err("some thing went wrong during getting linux_task!\n");
  }
  timer_setup(&(new_task->wakeup_timer), _timer_callback, 0);
  mutex_lock_interruptible(&task_list_mutex);
  list_add(&(new_task->list), &mp2_task_list);
  mutex_unlock(&task_list_mutex);

  pr_info("Leave registration_task()");
  return 1;
}

int yield_task(pid_t pid) {
  struct mp2_task_struct *task;
  struct mp2_task_struct *temp;
  pr_info("Inside yield_task()");

  mutex_lock(&task_list_mutex);
  // traverse the list to find a correct task
  list_for_each_entry(temp, &mp2_task_list, list) {
    if (temp->pid == pid)
      task = temp;
  }
  mutex_unlock(&task_list_mutex);

  if (!task) {
    pr_alert("We cannot find the task match specific PID");
    return -1;
  }
  printk("task->deadline_jiff=%lu, jiffies=%lu", task->deadline_jiff, jiffies);

  // According to document,
  // we should make sure next period is already passed or not
  if (task->deadline_jiff > jiffies) {
    task->state = SLEEPING;
    self_set_current_state(task->linux_task, TASK_UNINTERRUPTIBLE);
    // task->linux_task->__state = TASK_UNINTERRUPTIBLE;
    mod_timer(&task->wakeup_timer, task->deadline_jiff);
    // mutex_lock_interruptible(&task_list_mutex);
    // mutex_unlock(&task_list_mutex);
  }
  // after moding the timer, we still need to add more period to the task
  // for making the task can be executed in the next future deadline
  task->deadline_jiff += msecs_to_jiffies(task->period_ms);
  wake_up_process(dispatching_thread);
  schedule();
  pr_info("Leave yiekd_task");
  return 0;
}

int deregistration(pid_t pid) {
  struct mp2_task_struct *task;
  struct mp2_task_struct *temp;
  pr_info("Inside deregisteration()");

  mutex_lock_interruptible(&task_list_mutex);
  // traverse the list to find a correct task
  list_for_each_entry(temp, &mp2_task_list, list) {
    if (temp->pid == pid)
      task = temp;
  }
  if (!task) {
    pr_info("Failed to find the task!");
    return -1;
  }

  pr_info("Going to deregister a running task!!");
  del_timer_sync(&task->wakeup_timer);
  list_del(&task->list);
  kmem_cache_free(mp2_task_cache, task);

  if (task == running_task) {
    running_task = NULL;
    wake_up_process(dispatching_thread);
  }

  mutex_unlock(&task_list_mutex);
  pr_info("Leave deregisteration()");
  return 1;
}

static ssize_t procfs_read(struct file *filp, char __user *buffer,
                           size_t length, loff_t *offset) {
  // in every new read, we have to re-initialize size of proc buffer
  unsigned long procfs_buffer_size = 0;
  char *procfs_buffer = (char *)kmalloc(PROCFS_MAX_SIZE, GFP_KERNEL);
  struct mp2_task_struct *temp;
  pr_info("Inside procfs_read() \n");

  // if *offset > 0,
  // which means we didn't finish procfs_read in last time
  if (*offset) {
    pr_info("procfs_read: END\n");
    *offset = 0;
    return 0;
  }

  mutex_lock_interruptible(&task_list_mutex);
  list_for_each_entry(temp, &mp2_task_list, list) {
    // the first parameter of sprintf is ptr, which means when it's a position
    // to append new characters; hence, when we go to next loop we've to add
    // the
    // offset to got to new position for append
    procfs_buffer_size +=
        sprintf(procfs_buffer + procfs_buffer_size,
                "<pid %u>: <period: %lu>, <process time: %lu>\n", temp->pid,
                temp->period_ms, temp->runtime_ms);
  }
  mutex_unlock(&task_list_mutex);

  procfs_buffer_size = min(procfs_buffer_size, length);
  if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size)) {
    pr_err("Data Send : Err!\n");
  }

  *offset += procfs_buffer_size;
  pr_info("procfs_read: read %lu bytes\n", procfs_buffer_size);
  kfree(procfs_buffer);
  return procfs_buffer_size;
}

static ssize_t procfs_write(struct file *filp, const char __user *buffer,
                            size_t length, loff_t *offset) {
  char indicator;
  int pid = 0;
  unsigned long period_ms = 0;
  unsigned long runtime_ms = 0;
  unsigned long procfs_buffer_size = min(PROCFS_MAX_SIZE, length);
  int ret = 0;

  char *procfs_buffer = (char *)kmalloc(PROCFS_MAX_SIZE, GFP_KERNEL);
  if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
    pr_err("Data Write : Err!\n");
  }
  pr_info("Module received: %s", procfs_buffer);

  ret = sscanf(procfs_buffer, "%c %u %lu %lu", &indicator, &pid, &period_ms,
               &runtime_ms);

  pr_info("pid: %d, period: %lu, runtime: %lu", pid, period_ms, runtime_ms);

  switch (indicator) {
  case 'R':
    pr_info("Go into R");
    if (registration_task(pid, period_ms, runtime_ms) == -1) {
      pr_alert("Failed to register new task\n");
      return -1;
    }
    break;
  case 'Y':
    pr_info("Go into Y");
    if (yield_task(pid) == -1)
      pr_alert("Failed to yield current task\n");
    break;
  case 'D':
    pr_info("Go into D");
    if (deregistration(pid) == -1)
      pr_alert("Failed to deregister current task\n");
    break;
  }

  pr_info("procfs_write: write %lu bytes\n", procfs_buffer_size);
  kfree(procfs_buffer);
  return procfs_buffer_size;
}

static int _procfs_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return 0;
}

static int _procfs_close(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;
}

#ifdef HAVE_PROC_OPS
static struct proc_ops proc_file_fops = {
    .proc_read = procfs_read,
    .proc_write = procfs_write,
    .proc_open = _procfs_open,
    .proc_release = _procfs_close,
};
#else
static const struct file_operations file_ops_4_our_proc_file = {
    .read = procfs_read,
    .write = procfs_write,
    .open = procfs_open,
    .release = procfs_close,
};
#endif

// mp2_init - Called when module is loaded
int __init mp2_init(void) {
  int ret_val = 0;
#ifdef DEBUG
  printk(KERN_ALERT "MP2 MODULE LOADING\n");
#endif
  // Insert your code here ...
  // initialize "mp2" directory
  mp2_dir = proc_mkdir(DIRECTORY, NULL);
  if (mp2_dir == NULL) {
    pr_alert("Error: Could not initialize /proc/%s\n", DIRECTORY);
    ret_val = -ENOMEM;
    goto out;
  }

  // initialize "status" proc ectry
  status_entry = proc_create(ENTRY, 0666, mp2_dir, &proc_file_fops);
  if (status_entry == NULL) {
    pr_alert("Error: Could not initialize /proc/mp2/%s\n", ENTRY);
    ret_val = -ENOMEM;
    goto badfile;
  }

  // it seems like we can set random name in name part
  mp2_task_cache = kmem_cache_create(KMEM_NAME, sizeof(struct mp2_task_struct),
                                     0, SLAB_HWCACHE_ALIGN, NULL);
  if (mp2_task_cache == NULL) {
    pr_alert("Error: Something bad happened, when creating %s\n", KMEM_NAME);
    ret_val = -ENOMEM;
    goto out;
  }

  // using a kernel thread to handle our main scheduling logic
  dispatching_thread =
      kthread_run(dispatch_thread_function, NULL, DISPATCHING_THREAD_NAME);

  printk(KERN_ALERT "MP2 MODULE LOADED\n");
  return 0;

badfile:
  remove_proc_entry(DIRECTORY, NULL);
out:
  return ret_val;
}

// mp2_exit - Called when module is unloaded
void __exit mp2_exit(void) {
  struct mp2_task_struct *cursor;
  struct mp2_task_struct *temp;
#ifdef DEBUG
  printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
#endif
  // release linked list
  list_for_each_entry_safe(cursor, temp, &mp2_task_list, list) {
    del_timer_sync(&(cursor->wakeup_timer));
    list_del(&(cursor->list));
    kmem_cache_free(mp2_task_cache, cursor);
  }

  mutex_destroy(&task_list_mutex);

  // this will trigger kthread_should_stop
  kthread_stop(dispatching_thread);
  kmem_cache_destroy(mp2_task_cache);

  // release proc directory & entry
  remove_proc_entry(ENTRY, mp2_dir);
  remove_proc_entry(DIRECTORY, NULL);

  printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp2_init);
module_exit(mp2_exit);
