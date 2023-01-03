# UIUC-CS423 Operating System Implementation - MP2

Goal : Implement Rate-Monotonic CPU Scheduling

How to test this MP?
> 1. cd mp2-sylinhugo/
> 2. sudo su
> 3. make
> 4. dmesg --clear
> 5. insmod mp2.ko
> 6-1. In one terminal: $ ./userapp <period_ms> <process_time_ms> <number_of_task>
> 6-2. Other terminal, do the same instruction
> 7. Terminal(s) will show the result
> 8. Double check the result in cmd "dmesg"
> 9. rmmod mp2.ko
> 10. exit
> 11. Score 100 :))

---

Idea behind this MP:

mp2.c => the main logic of rate-monotonic cpu scheduling
```
#define self_set_current_state(): I copied this function from linux kernel. Because current kernel version don't support set_current_state() anymore. Thanks for recommendation from Jinghao

get_highest_priority(): a function that help use find out the task with highest priority

dispatch_thread_function(): the most important function of this mp. This function help us to do context switch, to switch to other task

admission_control_check(): function check that whether we can add the new task to scheduling or not

registration_task(), yield_task(), deregistration(): these three functions will be triggered by different instructions

procfs_read(): same as how we implement in mp1

procfs_write(): same structure in mp1; however, in mp2 we need to use switch statement to trigger correct funtions

```

---

Before showing the result, there is one caveat!!
We can not imput instruction like:
```
./userapp 100 2000 3
```
Because the period time cannot lower than process time.

Another weird thing is, if I tried below instructions:
```
./userapp 8000 1000 5& ./userapp 5000 1000 5
```
Then the kernel will crash, I cannot find the reason to fix it.

However, in the following result showing, if I run those two instructions in different terminal in almost same time. There is no error result, always finish successfully.


Result showing (I ran this two ./userapp almost in the same time, but terminal one run before terminal two)
- Terminal One:
```
root@cs423 mp2-sylinhugo ./userapp 8000 1000 5
echo R 3776 8000 1000 > /proc/mp2/status
<pid 3776>: <period: 8000>, <process time: 1000>
wakeup_time is: 0 s
process_time is: 1 s
echo Y 3776 > /proc/mp2/status
wakeup_time is: 8 s
process_time is: 1 s
echo Y 3776 > /proc/mp2/status
wakeup_time is: 15 s
process_time is: 1 s
echo Y 3776 > /proc/mp2/status
wakeup_time is: 20 s
process_time is: 1 s
echo Y 3776 > /proc/mp2/status
wakeup_time is: 25 s
process_time is: 1 s
echo Y 3776 > /proc/mp2/status
echo D 3776 > /proc/mp2/status
```
- Terminal Two:
```
root@cs423 mp2-sylinhugo ./userapp 5000 1000 5
echo R 3822 5000 1000 > /proc/mp2/status
<pid 3822>: <period: 5000>, <process time: 1000>
<pid 3776>: <period: 8000>, <process time: 1000>
wakeup_time is: 0 s
process_time is: 1 s
echo Y 3822 > /proc/mp2/status
wakeup_time is: 1 s
process_time is: 1 s
echo Y 3822 > /proc/mp2/status
wakeup_time is: 10 s
process_time is: 2 s
echo Y 3822 > /proc/mp2/status
wakeup_time is: 16 s
process_time is: 1 s
echo Y 3822 > /proc/mp2/status
wakeup_time is: 21 s
process_time is: 1 s
echo Y 3822 > /proc/mp2/status
echo D 3822 > /proc/mp2/status
```

dmesg:
```
[   47.541625] mp2: loading out-of-tree module taints kernel.
[   47.541718] mp2: module verification failed: signature and/or required key missing - tainting kernel
[   47.542128] MP2 MODULE LOADING
[   47.543583] MP2 MODULE LOADED
[   47.543601] Dispatched Thread be Triggered
[   47.543603] Inside get_highest_priority_task() function
[   47.543603] Leave get_highest_priority_task() function
[   47.543604] highest_priority_task=0x0
[   47.543605] Leave Dispatched Thread Functions
[   79.604660] Module received: R 3079 8000 1000
[   79.604670] pid: 3079, period: 8000, runtime: 1000
[   79.604673] Go into R
[   79.604673] Inside registration_task()
[   79.604674] registration_task() get pid: 3079, period: 8000, runtime: 1000
[   79.604676] Inside admisssion_control_check()
[   79.604676] Leave admisssion_control_check()
[   79.604678] Leave registration_task()
[   79.604679] procfs_write: write 17 bytes
[   79.609080] Inside procfs_read() 
[   79.609088] procfs_read: read 49 bytes
[   79.609098] Inside procfs_read() 
[   79.609098] procfs_read: END
[   80.612085] Module received: Y 3079
[   80.612090] pid: 3079, period: 0, runtime: 0
[   80.612091] Go into Y
[   80.612092] Inside yield_task()
[   80.612092] task->deadline_jiff=4294914219, jiffies=4294912471
[   80.612098] Leave yiekd_task
[   80.612098] procfs_write: write 7 bytes
[   80.612104] Dispatched Thread be Triggered
[   80.612107] Inside get_highest_priority_task() function
[   80.612107] TEMP task: 0xffff0000d2a7d300
[   80.612108] Leave get_highest_priority_task() function
[   80.612108] highest_priority_task=0x0
[   80.612109] Leave Dispatched Thread Functions
[   85.443436] Module received: R 3149 4000 1000
[   85.443449] pid: 3149, period: 4000, runtime: 1000
[   85.443453] Go into R
[   85.443454] Inside registration_task()
[   85.443456] registration_task() get pid: 3149, period: 4000, runtime: 1000
[   85.443458] Inside admisssion_control_check()
[   85.443460] Leave admisssion_control_check()
[   85.443463] Leave registration_task()
[   85.443464] procfs_write: write 17 bytes
[   85.453482] Inside procfs_read() 
[   85.453495] procfs_read: read 98 bytes
[   85.453515] Inside procfs_read() 
[   85.453516] procfs_read: END
[   86.455531] Module received: Y 3149
[   86.455536] pid: 3149, period: 0, runtime: 0
[   86.455537] Go into Y
[   86.455537] Inside yield_task()
[   86.455538] task->deadline_jiff=4294914679, jiffies=4294913932
[   86.455541] Dispatched Thread be Triggered
[   86.455542] Inside get_highest_priority_task() function
[   86.455542] TEMP task: 0xffff0000e84cd580
[   86.455542] TEMP task: 0xffff0000d2a7d300
[   86.455543] Leave get_highest_priority_task() function
[   86.455543] highest_priority_task=0x0
[   86.455543] Leave Dispatched Thread Functions
[   86.455547] Leave yiekd_task
[   86.455547] procfs_write: write 7 bytes
[   87.685979] Go in to timer handler
[   87.686041] Timer callback task=0xffff0000d2a7d300
[   87.686067] Leave timer handler
[   87.686474] Dispatched Thread be Triggered
[   87.686478] Inside get_highest_priority_task() function
[   87.686479] TEMP task: 0xffff0000e84cd580
[   87.686482] TEMP task: 0xffff0000d2a7d300
[   87.686483] READY task: 0xffff0000d2a7d300
[   87.686484] Leave get_highest_priority_task() function
[   87.686484] highest_priority_task=0xffff0000d2a7d300
[   87.686485] Find highest_priority_task successfully
[   87.686487] Going to schedule highest_priority_task
[   87.686518] Leave Dispatched Thread Functions
[   88.636541] sched: RT throttling activated
[   88.738664] Module received: Y 3079
[   88.738670] pid: 3079, period: 0, runtime: 0
[   88.738671] Go into Y
[   88.738672] Inside yield_task()
[   88.738673] task->deadline_jiff=4294916219, jiffies=4294914503
[   88.738678] Leave yiekd_task
[   88.738678] procfs_write: write 7 bytes
[   88.738689] Dispatched Thread be Triggered
[   88.738690] Inside get_highest_priority_task() function
[   88.738691] TEMP task: 0xffff0000e84cd580
[   88.738692] TEMP task: 0xffff0000d2a7d300
[   88.738692] Leave get_highest_priority_task() function
[   88.738692] highest_priority_task=0x0
[   88.738693] Cannot find highest_priority_task
[   88.738695] Simply preempt current running task
[   88.738697] Leave Dispatched Thread Functions
[   89.480176] Go in to timer handler
[   89.480269] Timer callback task=0xffff0000e84cd580
[   89.480302] Leave timer handler
[   89.480712] Dispatched Thread be Triggered
[   89.480716] Inside get_highest_priority_task() function
[   89.480718] TEMP task: 0xffff0000e84cd580
[   89.480720] READY task: 0xffff0000e84cd580
[   89.480722] TEMP task: 0xffff0000d2a7d300
[   89.480724] READY task: 0xffff0000d2a7d300
[   89.480726] Leave get_highest_priority_task() function
[   89.480727] highest_priority_task=0xffff0000e84cd580
[   89.480728] Find highest_priority_task successfully
[   89.480731] Going to schedule highest_priority_task
[   89.480747] Leave Dispatched Thread Functions
[   90.481563] Module received: Y 3149
[   90.481569] pid: 3149, period: 0, runtime: 0
[   90.481570] Go into Y
[   90.481571] Inside yield_task()
[   90.481572] task->deadline_jiff=4294915679, jiffies=4294914939
[   90.481579] Leave yiekd_task
[   90.481579] procfs_write: write 7 bytes
[   90.481613] Dispatched Thread be Triggered
[   90.481616] Inside get_highest_priority_task() function
[   90.481617] TEMP task: 0xffff0000e84cd580
[   90.481618] TEMP task: 0xffff0000d2a7d300
[   90.481618] READY task: 0xffff0000d2a7d300
[   90.481619] Leave get_highest_priority_task() function
[   90.481619] highest_priority_task=0xffff0000d2a7d300
[   90.481619] Find highest_priority_task successfully
[   90.481621] Going to schedule highest_priority_task
[   90.481627] Leave Dispatched Thread Functions
[   91.482531] Module received: Y 3079
[   91.482543] pid: 3079, period: 0, runtime: 0
[   91.482544] Go into Y
[   91.482545] Inside yield_task()
[   91.482547] task->deadline_jiff=4294918219, jiffies=4294915189
[   91.482553] Leave yiekd_task
[   91.482553] procfs_write: write 7 bytes
[   91.482585] Dispatched Thread be Triggered
[   91.482586] Inside get_highest_priority_task() function
[   91.482587] TEMP task: 0xffff0000e84cd580
[   91.482587] TEMP task: 0xffff0000d2a7d300
[   91.482588] Leave get_highest_priority_task() function
[   91.482588] highest_priority_task=0x0
[   91.482588] Cannot find highest_priority_task
[   91.482591] Simply preempt current running task
[   91.482596] Leave Dispatched Thread Functions
[   93.576929] Go in to timer handler
[   93.576979] Timer callback task=0xffff0000e84cd580
[   93.576998] Leave timer handler
[   93.577024] Dispatched Thread be Triggered
[   93.577026] Inside get_highest_priority_task() function
[   93.577027] TEMP task: 0xffff0000e84cd580
[   93.577028] READY task: 0xffff0000e84cd580
[   93.577029] TEMP task: 0xffff0000d2a7d300
[   93.577031] READY task: 0xffff0000d2a7d300
[   93.577031] Leave get_highest_priority_task() function
[   93.577032] highest_priority_task=0xffff0000e84cd580
[   93.577033] Find highest_priority_task successfully
[   93.577035] Going to schedule highest_priority_task
[   93.577038] Leave Dispatched Thread Functions
[   93.593487] Module received: D 3149
[   93.593494] pid: 3149, period: 0, runtime: 0
[   93.593496] Go into D
[   93.593497] Inside deregisteration()
[   93.593499] Going to deregister a running task!!
[   93.593503] Leave deregisteration()
[   93.593503] procfs_write: write 7 bytes
[   93.593615] Dispatched Thread be Triggered
[   93.593617] Inside get_highest_priority_task() function
[   93.593617] TEMP task: 0xffff0000d2a7d300
[   93.593618] READY task: 0xffff0000d2a7d300
[   93.593619] Leave get_highest_priority_task() function
[   93.593620] highest_priority_task=0xffff0000d2a7d300
[   93.593621] Find highest_priority_task successfully
[   93.593623] Going to schedule highest_priority_task
[   93.593628] Leave Dispatched Thread Functions
[   94.630958] Module received: Y 3079
[   94.630963] pid: 3079, period: 0, runtime: 0
[   94.630964] Go into Y
[   94.630965] Inside yield_task()
[   94.630965] task->deadline_jiff=4294920219, jiffies=4294915976
[   94.630970] Leave yiekd_task
[   94.630970] procfs_write: write 7 bytes
[   94.631000] Dispatched Thread be Triggered
[   94.631002] Inside get_highest_priority_task() function
[   94.631002] TEMP task: 0xffff0000d2a7d300
[   94.631003] Leave get_highest_priority_task() function
[   94.631003] highest_priority_task=0x0
[   94.631004] Cannot find highest_priority_task
[   94.631005] Simply preempt current running task
[   94.631007] Leave Dispatched Thread Functions
[  113.541815] Go in to timer handler
[  113.541879] Timer callback task=0xffff0000d2a7d300
[  113.541905] Leave timer handler
[  113.542376] Dispatched Thread be Triggered
[  113.542379] Inside get_highest_priority_task() function
[  113.542381] TEMP task: 0xffff0000d2a7d300
[  113.542383] READY task: 0xffff0000d2a7d300
[  113.542383] Leave get_highest_priority_task() function
[  113.542384] highest_priority_task=0xffff0000d2a7d300
[  113.542386] Find highest_priority_task successfully
[  113.542387] Going to schedule highest_priority_task
[  113.542414] Leave Dispatched Thread Functions
[  114.543207] Module received: Y 3079
[  114.543211] pid: 3079, period: 0, runtime: 0
[  114.543212] Go into Y
[  114.543212] Inside yield_task()
[  114.543213] task->deadline_jiff=4294922219, jiffies=4294920955
[  114.543215] Leave yiekd_task
[  114.543216] procfs_write: write 7 bytes
[  114.543245] Dispatched Thread be Triggered
[  114.543245] Inside get_highest_priority_task() function
[  114.543245] TEMP task: 0xffff0000d2a7d300
[  114.543246] Leave get_highest_priority_task() function
[  114.543246] highest_priority_task=0x0
[  114.543246] Cannot find highest_priority_task
[  114.543248] Simply preempt current running task
[  114.543250] Leave Dispatched Thread Functions
[  119.685638] Go in to timer handler
[  119.685789] Timer callback task=0xffff0000d2a7d300
[  119.685973] Leave timer handler
[  119.686451] Dispatched Thread be Triggered
[  119.686457] Inside get_highest_priority_task() function
[  119.686459] TEMP task: 0xffff0000d2a7d300
[  119.686461] READY task: 0xffff0000d2a7d300
[  119.686463] Leave get_highest_priority_task() function
[  119.686465] highest_priority_task=0xffff0000d2a7d300
[  119.686467] Find highest_priority_task successfully
[  119.686470] Going to schedule highest_priority_task
[  119.686530] Leave Dispatched Thread Functions
[  119.689697] Module received: D 3079
[  119.689711] pid: 3079, period: 0, runtime: 0
[  119.689714] Go into D
[  119.689717] Inside deregisteration()
[  119.689719] Going to deregister a running task!!
[  119.689726] Leave deregisteration()
[  119.689728] procfs_write: write 7 bytes
[  119.689875] Dispatched Thread be Triggered
[  119.689878] Inside get_highest_priority_task() function
[  119.689879] Leave get_highest_priority_task() function
[  119.689880] highest_priority_task=0x0
[  119.689882] Leave Dispatched Thread Functions
[  149.545849] Module received: R 3776 8000 1000
[  149.545868] pid: 3776, period: 8000, runtime: 1000
[  149.545873] Go into R
[  149.545874] Inside registration_task()
[  149.545875] registration_task() get pid: 3776, period: 8000, runtime: 1000
[  149.545877] Inside admisssion_control_check()
[  149.545878] Leave admisssion_control_check()
[  149.545881] Leave registration_task()
[  149.545882] procfs_write: write 17 bytes
[  149.551193] Inside procfs_read() 
[  149.551206] procfs_read: read 49 bytes
[  149.551230] Inside procfs_read() 
[  149.551231] procfs_read: END
[  150.553407] Module received: Y 3776
[  150.553412] pid: 3776, period: 0, runtime: 0
[  150.553413] Go into Y
[  150.553413] Inside yield_task()
[  150.553414] task->deadline_jiff=4294931705, jiffies=4294929957
[  150.553417] Dispatched Thread be Triggered
[  150.553417] Inside get_highest_priority_task() function
[  150.553418] TEMP task: 0xffff0000e84cd100
[  150.553418] Leave get_highest_priority_task() function
[  150.553418] highest_priority_task=0x0
[  150.553419] Leave Dispatched Thread Functions
[  150.553421] Leave yiekd_task
[  150.553421] procfs_write: write 7 bytes
[  152.936048] Module received: R 3822 5000 1000
[  152.936188] pid: 3822, period: 5000, runtime: 1000
[  152.936195] Go into R
[  152.936197] Inside registration_task()
[  152.936199] registration_task() get pid: 3822, period: 5000, runtime: 1000
[  152.936201] Inside admisssion_control_check()
[  152.936204] Leave admisssion_control_check()
[  152.936210] Leave registration_task()
[  152.936210] procfs_write: write 17 bytes
[  152.938487] Inside procfs_read() 
[  152.938497] procfs_read: read 98 bytes
[  152.938527] Inside procfs_read() 
[  152.938538] procfs_read: END
[  153.940020] Module received: Y 3822
[  153.940025] pid: 3822, period: 0, runtime: 0
[  153.940026] Go into Y
[  153.940026] Inside yield_task()
[  153.940027] task->deadline_jiff=4294931803, jiffies=4294930804
[  153.940030] Dispatched Thread be Triggered
[  153.940031] Inside get_highest_priority_task() function
[  153.940031] TEMP task: 0xffff0000d2a7d680
[  153.940032] TEMP task: 0xffff0000e84cd100
[  153.940032] Leave get_highest_priority_task() function
[  153.940032] highest_priority_task=0x0
[  153.940032] Leave Dispatched Thread Functions
[  153.940039] Leave yiekd_task
[  153.940039] procfs_write: write 7 bytes
[  154.941169] Module received: Y 3822
[  154.941175] pid: 3822, period: 0, runtime: 0
[  154.941176] Go into Y
[  154.941177] Inside yield_task()
[  154.941178] task->deadline_jiff=4294933053, jiffies=4294931054
[  154.941183] Leave yiekd_task
[  154.941183] procfs_write: write 7 bytes
[  154.941215] Dispatched Thread be Triggered
[  154.941217] Inside get_highest_priority_task() function
[  154.941217] TEMP task: 0xffff0000d2a7d680
[  154.941218] TEMP task: 0xffff0000e84cd100
[  154.941218] Leave get_highest_priority_task() function
[  154.941218] highest_priority_task=0x0
[  154.941219] Leave Dispatched Thread Functions
[  157.571758] Go in to timer handler
[  157.571829] Timer callback task=0xffff0000e84cd100
[  157.571869] Leave timer handler
[  157.572166] Dispatched Thread be Triggered
[  157.572171] Inside get_highest_priority_task() function
[  157.572174] TEMP task: 0xffff0000d2a7d680
[  157.572178] TEMP task: 0xffff0000e84cd100
[  157.572180] READY task: 0xffff0000e84cd100
[  157.572182] Leave get_highest_priority_task() function
[  157.572184] highest_priority_task=0xffff0000e84cd100
[  157.572186] Find highest_priority_task successfully
[  157.572190] Going to schedule highest_priority_task
[  157.572207] Leave Dispatched Thread Functions
[  158.626292] Module received: Y 3776
[  158.626300] pid: 3776, period: 0, runtime: 0
[  158.626301] Go into Y
[  158.626302] Inside yield_task()
[  158.626303] task->deadline_jiff=4294933705, jiffies=4294931975
[  158.626311] Leave yiekd_task
[  158.626312] procfs_write: write 7 bytes
[  158.626346] Dispatched Thread be Triggered
[  158.626348] Inside get_highest_priority_task() function
[  158.626348] TEMP task: 0xffff0000d2a7d680
[  158.626349] TEMP task: 0xffff0000e84cd100
[  158.626349] Leave get_highest_priority_task() function
[  158.626349] highest_priority_task=0x0
[  158.626350] Cannot find highest_priority_task
[  158.626352] Simply preempt current running task
[  158.626354] Leave Dispatched Thread Functions
[  162.952328] Go in to timer handler
[  162.952343] Timer callback task=0xffff0000d2a7d680
[  162.952368] Leave timer handler
[  162.952677] Dispatched Thread be Triggered
[  162.952681] Inside get_highest_priority_task() function
[  162.952683] TEMP task: 0xffff0000d2a7d680
[  162.952684] READY task: 0xffff0000d2a7d680
[  162.952685] TEMP task: 0xffff0000e84cd100
[  162.952687] READY task: 0xffff0000e84cd100
[  162.952688] Leave get_highest_priority_task() function
[  162.952689] highest_priority_task=0xffff0000d2a7d680
[  162.952690] Find highest_priority_task successfully
[  162.952692] Going to schedule highest_priority_task
[  162.952711] Leave Dispatched Thread Functions
[  164.006250] Module received: Y 3822
[  164.006254] pid: 3822, period: 0, runtime: 0
[  164.006255] Go into Y
[  164.006255] Inside yield_task()
[  164.006256] task->deadline_jiff=4294934303, jiffies=4294933320
[  164.006260] Leave yiekd_task
[  164.006260] procfs_write: write 7 bytes
[  164.006290] Dispatched Thread be Triggered
[  164.006291] Inside get_highest_priority_task() function
[  164.006292] TEMP task: 0xffff0000d2a7d680
[  164.006293] TEMP task: 0xffff0000e84cd100
[  164.006293] READY task: 0xffff0000e84cd100
[  164.006293] Leave get_highest_priority_task() function
[  164.006294] highest_priority_task=0xffff0000e84cd100
[  164.006294] Find highest_priority_task successfully
[  164.006295] Going to schedule highest_priority_task
[  164.006298] Leave Dispatched Thread Functions
[  165.057765] Module received: Y 3776
[  165.057769] pid: 3776, period: 0, runtime: 0
[  165.057770] Go into Y
[  165.057771] Inside yield_task()
[  165.057771] task->deadline_jiff=4294935705, jiffies=4294933583
[  165.057776] Leave yiekd_task
[  165.057776] procfs_write: write 7 bytes
[  165.057806] Dispatched Thread be Triggered
[  165.057810] Inside get_highest_priority_task() function
[  165.057811] TEMP task: 0xffff0000d2a7d680
[  165.057811] TEMP task: 0xffff0000e84cd100
[  165.057812] Leave get_highest_priority_task() function
[  165.057812] highest_priority_task=0x0
[  165.057812] Cannot find highest_priority_task
[  165.057814] Simply preempt current running task
[  165.057816] Leave Dispatched Thread Functions
[  168.068671] Go in to timer handler
[  168.068748] Timer callback task=0xffff0000d2a7d680
[  168.068778] Leave timer handler
[  168.068817] Dispatched Thread be Triggered
[  168.068828] Inside get_highest_priority_task() function
[  168.068830] TEMP task: 0xffff0000d2a7d680
[  168.068833] READY task: 0xffff0000d2a7d680
[  168.068835] TEMP task: 0xffff0000e84cd100
[  168.068837] READY task: 0xffff0000e84cd100
[  168.068839] Leave get_highest_priority_task() function
[  168.068840] highest_priority_task=0xffff0000d2a7d680
[  168.068842] Find highest_priority_task successfully
[  168.068846] Going to schedule highest_priority_task
[  168.068879] Leave Dispatched Thread Functions
[  169.124575] Module received: Y 3822
[  169.124579] pid: 3822, period: 0, runtime: 0
[  169.124580] Go into Y
[  169.124581] Inside yield_task()
[  169.124582] task->deadline_jiff=4294935553, jiffies=4294934600
[  169.124587] Leave yiekd_task
[  169.124587] procfs_write: write 7 bytes
[  169.124618] Dispatched Thread be Triggered
[  169.124620] Inside get_highest_priority_task() function
[  169.124620] TEMP task: 0xffff0000d2a7d680
[  169.124621] TEMP task: 0xffff0000e84cd100
[  169.124621] READY task: 0xffff0000e84cd100
[  169.124622] Leave get_highest_priority_task() function
[  169.124622] highest_priority_task=0xffff0000e84cd100
[  169.124622] Find highest_priority_task successfully
[  169.124624] Going to schedule highest_priority_task
[  169.124626] Leave Dispatched Thread Functions
[  170.171340] Module received: Y 3776
[  170.171348] pid: 3776, period: 0, runtime: 0
[  170.171350] Go into Y
[  170.171350] Inside yield_task()
[  170.171352] task->deadline_jiff=4294937705, jiffies=4294934862
[  170.171362] Leave yiekd_task
[  170.171362] procfs_write: write 7 bytes
[  170.171398] Dispatched Thread be Triggered
[  170.171400] Inside get_highest_priority_task() function
[  170.171400] TEMP task: 0xffff0000d2a7d680
[  170.171401] TEMP task: 0xffff0000e84cd100
[  170.171401] Leave get_highest_priority_task() function
[  170.171402] highest_priority_task=0x0
[  170.171402] Cannot find highest_priority_task
[  170.171405] Simply preempt current running task
[  170.171409] Leave Dispatched Thread Functions
[  173.190073] Go in to timer handler
[  173.190104] Timer callback task=0xffff0000d2a7d680
[  173.190120] Leave timer handler
[  173.190272] Dispatched Thread be Triggered
[  173.190275] Inside get_highest_priority_task() function
[  173.190276] TEMP task: 0xffff0000d2a7d680
[  173.190277] READY task: 0xffff0000d2a7d680
[  173.190278] TEMP task: 0xffff0000e84cd100
[  173.190280] READY task: 0xffff0000e84cd100
[  173.190281] Leave get_highest_priority_task() function
[  173.190281] highest_priority_task=0xffff0000d2a7d680
[  173.190282] Find highest_priority_task successfully
[  173.190285] Going to schedule highest_priority_task
[  173.190293] Leave Dispatched Thread Functions
[  174.191012] Module received: Y 3822
[  174.191020] pid: 3822, period: 0, runtime: 0
[  174.191020] Go into Y
[  174.191021] Inside yield_task()
[  174.191024] task->deadline_jiff=4294936803, jiffies=4294935867
[  174.191029] Leave yiekd_task
[  174.191029] procfs_write: write 7 bytes
[  174.191059] Dispatched Thread be Triggered
[  174.191064] Inside get_highest_priority_task() function
[  174.191064] TEMP task: 0xffff0000d2a7d680
[  174.191065] TEMP task: 0xffff0000e84cd100
[  174.191066] READY task: 0xffff0000e84cd100
[  174.191066] Leave get_highest_priority_task() function
[  174.191066] highest_priority_task=0xffff0000e84cd100
[  174.191066] Find highest_priority_task successfully
[  174.191068] Going to schedule highest_priority_task
[  174.191073] Leave Dispatched Thread Functions
[  175.241422] Module received: Y 3776
[  175.241429] pid: 3776, period: 0, runtime: 0
[  175.241430] Go into Y
[  175.241430] Inside yield_task()
[  175.241431] task->deadline_jiff=4294939705, jiffies=4294936129
[  175.241436] Leave yiekd_task
[  175.241436] procfs_write: write 7 bytes
[  175.241467] Dispatched Thread be Triggered
[  175.241471] Inside get_highest_priority_task() function
[  175.241471] TEMP task: 0xffff0000d2a7d680
[  175.241472] TEMP task: 0xffff0000e84cd100
[  175.241472] Leave get_highest_priority_task() function
[  175.241473] highest_priority_task=0x0
[  175.241473] Cannot find highest_priority_task
[  175.241475] Simply preempt current running task
[  175.241478] Leave Dispatched Thread Functions
[  178.054911] Go in to timer handler
[  178.054969] Timer callback task=0xffff0000d2a7d680
[  178.054998] Leave timer handler
[  178.055031] Dispatched Thread be Triggered
[  178.055033] Inside get_highest_priority_task() function
[  178.055035] TEMP task: 0xffff0000d2a7d680
[  178.055037] READY task: 0xffff0000d2a7d680
[  178.055039] TEMP task: 0xffff0000e84cd100
[  178.055041] READY task: 0xffff0000e84cd100
[  178.055042] Leave get_highest_priority_task() function
[  178.055044] highest_priority_task=0xffff0000d2a7d680
[  178.055045] Find highest_priority_task successfully
[  178.055048] Going to schedule highest_priority_task
[  178.055079] Leave Dispatched Thread Functions
[  178.056943] Module received: D 3822
[  178.056950] pid: 3822, period: 0, runtime: 0
[  178.056953] Go into D
[  178.056955] Inside deregisteration()
[  178.056957] Going to deregister a running task!!
[  178.056977] Leave deregisteration()
[  178.056978] procfs_write: write 7 bytes
[  178.057001] Dispatched Thread be Triggered
[  178.057005] Inside get_highest_priority_task() function
[  178.057007] TEMP task: 0xffff0000e84cd100
[  178.057010] READY task: 0xffff0000e84cd100
[  178.057011] Leave get_highest_priority_task() function
[  178.057013] highest_priority_task=0xffff0000e84cd100
[  178.057015] Find highest_priority_task successfully
[  178.057018] Going to schedule highest_priority_task
[  178.057026] Leave Dispatched Thread Functions
[  178.058590] Module received: D 3776
[  178.058595] pid: 3776, period: 0, runtime: 0
[  178.058598] Go into D
[  178.058599] Inside deregisteration()
[  178.058601] Going to deregister a running task!!
[  178.058607] Leave deregisteration()
[  178.058608] procfs_write: write 7 bytes
[  178.058725] Dispatched Thread be Triggered
[  178.058728] Inside get_highest_priority_task() function
[  178.058729] Leave get_highest_priority_task() function
[  178.058730] highest_priority_task=0x0
[  178.058732] Leave Dispatched Thread Functions
```
