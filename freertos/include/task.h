#ifndef TASK_H
#define TASK_H

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

/* ������ */
typedef void* TaskHandle_t;

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,const char * const pcName,const uint32_t ulStackDepth,void * const pvParameters,UBaseType_t uxPriority,TaskHandle_t * const pxCreatedTask,TCB_t *pxNewTCB );

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
TaskHandle_t xTaskCreateStatic(   TaskFunction_t pxTaskCode,
                                  const char * const pcName,
																	const uint32_t ulStackDepth,
																	void * const pvParameters,
																	/* �������ȼ�����ֵԽ�����ȼ�Խ�� */
																	UBaseType_t uxPriority,
																	StackType_t * const puxStackBuffer,
																	TCB_t * const pxTaskBuffer   );
#endif

void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize );

void prvInitialiseTaskLists( void );
//void xTaskIncrementTick( void );
BaseType_t xTaskIncrementTick( void );
void vTaskStartScheduler( void );
void vTaskDelay( const TickType_t xTicksToDelay );
void prvIdleTask(void *pvParameters);
static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB );
static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait );
static void prvResetNextTaskUnblockTime( void );

/* �����л� */
#define taskYIELD()   portYIELD()
															 
/* �����ٽ�Σ������жϱ����汾������Ƕ�� */
#define taskENTER_CRITICAL()             portENTER_CRITICAL()
/* �����ٽ�Σ����жϱ����汾������Ƕ�� */
#define taskENTER_CRITICAL_FROM_ISR()    portSET_INTERRUPT_MASK_FROM_ISR()
/* �˳��ٽ�Σ������жϱ����汾������Ƕ�� */
#define taskEXIT_CRITICAL()              portEXIT_CRITICAL()
/* �˳��ٽ�Σ����жϱ����汾������Ƕ�� */
#define taskEXIT_CRITICAL_FROM_ISR(x)    portCLEAR_INTERRUPT_MASK_FROM_ISR( x )
/* �����������ȼ��궨�壬�� task.h �ж��� */
#define tskIDLE_PRIORITY           ( ( UBaseType_t ) 0U )
#endif
