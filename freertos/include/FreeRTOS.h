#ifndef FREERTOS_H
#define FREERTOS_H

#include "list.h"
#include "FreeRTOSConfig.h"

typedef struct tskTaskControlBlock
{
		volatile StackType_t    *pxTopOfStack;    /* 栈顶 */
		
		ListItem_t              xStateListItem;   /* 任务节点 */
		
		StackType_t             *pxStack;         /* 任务栈起始地址 */
		                                          
		char                    pcTaskName[ configMAX_TASK_NAME_LEN ];/* 任务名称，字符串形式 */
	
		TickType_t              xTicksToDelay; /* 用于延时 */
		UBaseType_t             uxPriority;    /* 优先级 */
}tskTCB;
typedef tskTCB TCB_t;

#endif
