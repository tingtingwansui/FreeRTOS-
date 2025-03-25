#ifndef TASK_H
#define TASK_H

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

/* 任务句柄 */
typedef void* TaskHandle_t;

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,const char * const pcName,const uint32_t ulStackDepth,void * const pvParameters,UBaseType_t uxPriority,TaskHandle_t * const pxCreatedTask,TCB_t *pxNewTCB );

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
TaskHandle_t xTaskCreateStatic(   TaskFunction_t pxTaskCode,
                                  const char * const pcName,
																	const uint32_t ulStackDepth,
																	void * const pvParameters,
																	/* 任务优先级，数值越大，优先级越高 */
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

/* 任务切换 */
#define taskYIELD()   portYIELD()
															 
/* 进入临界段，不带中断保护版本，不能嵌套 */
#define taskENTER_CRITICAL()             portENTER_CRITICAL()
/* 进入临界段，带中断保护版本，可以嵌套 */
#define taskENTER_CRITICAL_FROM_ISR()    portSET_INTERRUPT_MASK_FROM_ISR()
/* 退出临界段，不带中断保护版本，不能嵌套 */
#define taskEXIT_CRITICAL()              portEXIT_CRITICAL()
/* 退出临界段，带中断保护版本，可以嵌套 */
#define taskEXIT_CRITICAL_FROM_ISR(x)    portCLEAR_INTERRUPT_MASK_FROM_ISR( x )
/* 空闲任务优先级宏定义，在 task.h 中定义 */
#define tskIDLE_PRIORITY           ( ( UBaseType_t ) 0U )
#endif
