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
		/* ���� PendSV,�����������л� */         \
		portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;   \
		__dsb( portSY_FULL_READ_WRITE );          \
		__isb( portSY_FULL_READ_WRITE );          \
}
/* ��������ֵ�Ĺ��жϺ���������Ƕ�ף��������ж�����ʹ�� */
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


/* ������ֵ�Ĺ��жϺ���������Ƕ�ף��������ж�����ʹ�� */
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

/* �����жϱ����Ŀ��жϺ��� */
#define portENABLE_INTERRUPTS() vPortSetBASEPRI( 0 )
/* ���жϱ����Ŀ��жϺ��� */
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
		/* ��BASEPRI����Ϊ0�������Ͳ��������жϡ��������ֻ��
		���ڽ����ж��е����룬����ڴ����ϲ�����
		ʹ�á� */
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
