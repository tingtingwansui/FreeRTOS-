/*
*************************************************************************
*                          包含的头文件
*************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
/*
*************************************************************************
*                            全局变量
*************************************************************************
*/

portCHAR flag1;
portCHAR flag2;
portCHAR flag3;

extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

/*
*************************************************************************
*                            任务控制块 & STACK
*************************************************************************
*/

TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE 128
StackType_t Task1Stack[TASK1_STACK_SIZE]; 
TCB_t Task1TCB;

TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE 128
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;

TaskHandle_t Task3_Handle;
#define TASK3_STACK_SIZE 128
StackType_t Task3Stack[TASK3_STACK_SIZE];
TCB_t Task3TCB;

/*
*************************************************************************
*                        函数声明
*************************************************************************
*/

void delay (uint32_t count);
void Task1_Entry( void *p_arg );
void Task2_Entry( void *p_arg );
void Task3_Entry( void *p_arg );

/*
************************************************************************
                                  main函数
************************************************************************
*/

int main(void)
{
		/* 硬件初始化 */
		/* 将硬件相关的初始化放在这里，如果是软件仿真则没有相关初始化代码 */
		/* 初始化与任务相关的列表，如就绪列表 */
	
		Task1_Handle = 
		xTaskCreateStatic ( (TaskFunction_t)Task1_Entry,
												(char *)"Task1",
												(uint32_t)TASK1_STACK_SIZE,
												(void * )NULL,
												/* 任务优先级，数值越大，优先级越高 */
												(UBaseType_t) 2,
												(StackType_t *)Task1Stack,
												(TCB_t *)&Task1TCB );

		Task2_Handle = 
		xTaskCreateStatic ( (TaskFunction_t)Task2_Entry,
												(char *)"Task2",
												(uint32_t)TASK2_STACK_SIZE,
												(void * )NULL,
												/* 任务优先级，数值越大，优先级越高 */
												(UBaseType_t) 2,
												(StackType_t *)Task2Stack,
												(TCB_t *)&Task2TCB );
		
		Task3_Handle = 
		xTaskCreateStatic ( (TaskFunction_t)Task3_Entry,
												(char *)"Task3",
												(uint32_t)TASK3_STACK_SIZE,
												(void * )NULL,
												/* 任务优先级，数值越大，优先级越高 */
												(UBaseType_t) 3,
												(StackType_t *)Task3Stack,
												(TCB_t *)&Task3TCB );
		
		portDISABLE_INTERRUPTS();
		/* 启动调度器，开始多任务调度，启动成功则不返回 */
		vTaskStartScheduler();
		
		for(;;)
		{
				/* 系统启动成功不会到达这里 */
		}
}

/*
***********************************************************************
*                             函数实现
***********************************************************************
*/
/* 获取空闲任务内存 */
/* 定义空闲任务的栈 */
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
/* 定义空闲任务的任务控制块 */
TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack;
		*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;
}

/*软件延时*/
void delay (uint32_t count)
{
		for(; count!=0;count--);
}

/*任务1*/
void Task1_Entry( void *p_arg )
{
		for( ;; )
		{
			flag1 = 1;
			//vTaskDelay( 2 );
			delay( 100 );
			flag1 = 0;
			delay( 100 );
			//vTaskDelay( 2 );
		}
}

/*任务2*/
void Task2_Entry( void *p_arg )
{
		for( ;; )
		{
			flag2 = 1;
			//vTaskDelay( 2 );
			delay( 100 );
			flag2 = 0;
			//vTaskDelay( 2 );
			delay( 100 );
		}
}

/*任务2*/
void Task3_Entry( void *p_arg )
{
		for( ;; )
		{
			flag3 = 1;
			vTaskDelay( 2 );
			//delay( 100 );
			flag3 = 0;
			vTaskDelay( 2 );
			//delay( 100 );
		}
}

