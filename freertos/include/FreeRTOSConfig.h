#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_16_BIT_TICKS           0
#define configMAX_TASK_NAME_LEN          16
#define configSUPPORT_STATIC_ALLOCATION  1
#define configMAX_PRIORITIES             5
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 191    //用来配置中断屏蔽寄存器
#define configKERNEL_INTERRUPT_PRIORITY  15
#define configMINIMAL_STACK_SIZE       ( ( unsigned short ) 128 )
#define configCPU_CLOCK_HZ              ((unsigned long) 25000000)
#define configTICK_RATE_HZ              (( TickType_t ) 100)
#define configUSE_PREEMPTION             1          //表示有任务就绪且就绪任务的优先级比当前优先级高，需要执行一次任务切换
#define configUSE_TIME_SLICING           1          //用于控制同优先级任务之间的时间片轮转调度

#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#define vPortSVCHandler     SVC_Handler

#define configASSERT( x )

#endif
