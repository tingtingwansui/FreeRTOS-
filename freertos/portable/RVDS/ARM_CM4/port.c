#include "portmacro.h"
#include "task.h"

#define portINITIAL_XPSR        ( 0x01000000 )
#define portSTART_ADDRESS_MASK  ( ( StackType_t )  0xfffffffeUL )
/* SysTick 控制寄存器 */
#define portNVIC_SYSTICK_CTRL_REG   (*((volatile uint32_t *) 0xe000e010 ))
/* SysTick 重装载寄存器寄存器 */
#define portNVIC_SYSTICK_LOAD_REG   (*((volatile uint32_t *) 0xe000e014 ))
	
/* SysTick 时钟源选择 */
#ifndef configSYSTICK_CLOCK_HZ
			#define configSYSTICK_CLOCK_HZ     configCPU_CLOCK_HZ
			#define portNVIC_SYSTICK_CLK_BIT   ( 1UL << 2UL )
#else
			#define portNVIC_SYSTICK_CLK_BIT   ( 0 )
#endif

#define portNVIC_SYSTICK_INT_BIT         ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT      ( 1UL << 0UL )

static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;
TickType_t xTickCount;

static void prvTaskExitError( void )
{
		/* 函数停止在这里 */
		for(;;);
}

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,
																		TaskFunction_t pxCode,
																		void *pvParameters )
{
		/* 异常发生时，自动加载到 CPU 寄存器的内容 */
		pxTopOfStack--;
		*pxTopOfStack = portINITIAL_XPSR;
		pxTopOfStack--;
		*pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK;
		pxTopOfStack--;
		*pxTopOfStack = ( StackType_t ) prvTaskExitError;
		pxTopOfStack -= 5;/* R12, R3, R2 and R1 默认初始化为 0 */
		*pxTopOfStack = ( StackType_t ) pvParameters;
	
		/* 异常发生时，手动加载到 CPU 寄存器的内容 */
		pxTopOfStack -= 8;
	
		/* 返回栈顶指针，此时 pxTopOfStack 指向空闲栈 */
		return pxTopOfStack;
}

__asm void vPortSVCHandler( void )
{
		extern pxCurrentTCB;
		
		PRESERVE8
		
		ldr r3, =pxCurrentTCB
		ldr r1, [r3]
		ldr r0, [r1]
		ldmia  r0!, {r4-r11}
		msr psp, r0
		isb
		mov r0,#0
		msr basepri, r0
		orr r14, #0xd
		
		bx r14
}

__asm void xPortPendSVHandler( void )
{
		extern pxCurrentTCB;
		extern vTaskSwitchContext;
	
		PRESERVE8
	
		mrs r0,psp
		isb
	
		ldr r3, =pxCurrentTCB
		ldr r2, [r3]
	
		stmdb r0!,{r4-r11}
		str r0, [r2]
			
		stmdb sp!,{r3, r14}
		mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
		msr basepri, r0
		dsb
		isb
		bl vTaskSwitchContext
		mov r0, #0
		msr basepri, r0
		ldmia sp!, {r3, r14}
		
		ldr r1, [r3]
		ldr r0, [r1]
		ldmia r0!, {r4-r11}
		msr psp, r0
		isb
		bx r14
		nop
}

/*
* 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.3，百度搜索“PM0056”即可找到这个文档
* 在 Cortex-M 中，内核外设SCB 的地址范围为：0xE000ED00-0xE000ED3F
* 0xE000ED008 为 SCB 外设中 SCB_VTOR 这个寄存器的地址，里面存放的是向量表的起始地址，即 MSP 的地址
*/

__asm void prvStartFirstTask( void )
{
		PRESERVE8
	
		/* 在 Cortex-M 中，0xE000ED08 是 SCB_VTOR 这个寄存器的地址,里面存放的是向量表的起始地址，即 MSP 的地址 */
		ldr r0, =0xE000ED08
		ldr r0, [r0]
		ldr r0, [r0]
	
		/* 设置主堆栈指针msp的值 */
		msr msp,r0
	
		/* 使能全局中断 */
		/* 
       CPSID I ;PRIMASK=1 ;关中断
       CPSIE I ;PRIMASK=0 ;开中断
       CPSID F ;FAULTMASK=1 ;关异常
       CPSIE F ;FAULTMASK=0 ;开异常 
		*/
		cpsie i
		cpsie f
		dsb
		isb
	
		/* 调用SVC去启动第一个任务 */
		svc 0
		nop
		nop
}

/*
* 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.7，百度搜索“PM0056”即可找到这个文档
* 在 Cortex-M 中，内核外设 SCB 中 SHPR3 寄存器用于设置 SysTick 和 PendSV 的异常优先级
* System handler priority register 3 (SCB_SHPR3) SCB_SHPR3：0xE000 ED20
* Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception
* Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV
*/

#define portNVIC_SYSPRI2_REG  (*(( volatile uint32_t *) 0xe000ed20))

#define portNVIC_PENDSV_PRI  (((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI (((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

BaseType_t xPortStartScheduler( void )
{
		/* 配置PendSV 和SysTick 的中断优先级为最低  */
		portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
		portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
		
		/* 初始化 SysTick */
		vPortSetupTimerInterrupt();
	
		uxCriticalNesting = 0;
		
		/* 启动第一个任务，不再返回 */
		prvStartFirstTask();
		
		/* 不能运行到这里 */
		return 0;
}

/* 进入临界段 */
void vPortEnterCritical( void )
{
		portDISABLE_INTERRUPTS();
		uxCriticalNesting++;
	
		if( uxCriticalNesting == 1 )
		{
				configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
		}
}
/* 退出临界段 */
void vPortExitCritical( void )
{
		configASSERT( uxCriticalNesting );
		uxCriticalNesting--;
		if( uxCriticalNesting == 0 )
		{
				portENABLE_INTERRUPTS();
		}
}

/* SysTick中断服务函数 */
void xPortSysTickHandler( void )
{
		/* 关中断 */
		vPortRaiseBASEPRI();
		{
				/* 更新系统时基 */
				if( xTaskIncrementTick() != pdFALSE)
				{
						/* 任务切换，即触发 PendSV */
						//portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
						taskYIELD();
				}
		}
		/* 开中断 */
		vPortClearBASEPRIFromISR();
}

void vPortSetupTimerInterrupt( void )
{
		/* 设置重装载寄存器的值 */
		portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
	
		/* 设置系统定时器的时钟等于内核时钟
			 使能 SysTick 定时器中断
			 使能 SysTick 定时器 */
		portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT |
																	portNVIC_SYSTICK_INT_BIT |
																	portNVIC_SYSTICK_ENABLE_BIT  );
}
