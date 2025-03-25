#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "stdint.h"
#include "stddef.h"
#include "projdefs.h"
#include "FreeRTOSConfig.h"

#define portCHAR     char
#define portFLOAT    float
#define portDOUBLE   double
#define portLONG     long
#define portSHORT    short
#define portSTACK_TYPE uint32_t
#define portBASE_TYPE long

#define portFORCE_INLINE __forceinline
	
typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

static void prvTaskExitError( void );
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,TaskFunction_t pxCode,void *pvParameters );
BaseType_t xPortStartScheduler( void );
void vPortSetupTimerInterrupt( void );

#if(configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

#define portNVIC_INT_CTRL_REG     (*(( volatile uint32_t * ) 0xe000ed04))
#define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )

#define portSY_FULL_READ_WRITE    ( 15 )

#define portYIELD()                          \
{                                            \
		/* 触发 PendSV,产生上下文切换 */         \
		portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;   \
		__dsb( portSY_FULL_READ_WRITE );          \
		__isb( portSY_FULL_READ_WRITE );          \
}
/* 不带返回值的关中断函数，不能嵌套，不能在中断里面使用 */
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()
static portFORCE_INLINE void vPortRaiseBASEPRI( void )
{
		uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
		__asm
		{
				msr basepri, ulNewBASEPRI
				dsb
				isb
		}
}


/* 带返回值的关中断函数，可以嵌套，可以在中断里面使用 */
#define portSET_INTERRUPT_MASK_FROM_ISR()  ulPortRaiseBASEPRI()
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI( void )
{
		uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
		__asm
		{
				mrs ulReturn, basepri
				msr basepri, ulNewBASEPRI
				dsb
				isb
		}
		return ulReturn;
}

/* 不带中断保护的开中断函数 */
#define portENABLE_INTERRUPTS() vPortSetBASEPRI( 0 )
/* 带中断保护的开中断函数 */
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)
static portFORCE_INLINE void vPortSetBASEPRI( uint32_t ulBASEPRI )
{
		__asm
		{
				msr basepri, ulBASEPRI
		}
}

static portFORCE_INLINE void vPortClearBASEPRIFromISR( void )
{
	__asm
	{
		/* 将BASEPRI设置为0，这样就不会屏蔽中断。这个函数只有
		用于降低中断中的掩码，因此内存屏障不存在
		使用。 */
		msr basepri, #0
	}
}
void vPortEnterCritical( void );
void vPortExitCritical( void );

#define portENTER_CRITICAL()        vPortEnterCritical()

#define portEXIT_CRITICAL()         vPortExitCritical()

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )\
				( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )

#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )\
				( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )\
				uxTopPriority = ( 31UL - ( uint32_t ) __clz( ( uxReadyPriorities ) ) )
#endif
