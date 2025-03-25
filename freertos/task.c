#include "task.h"

/* ��������б� */
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
/* ���� uxTopReadyPriority */
static volatile UBaseType_t uxTopReadyPriority = tskIDLE_PRIORITY;
/* ȫ�������ʱ�� */
static UBaseType_t uxCurrentNumberOfTasks;

static List_t xDelayedTaskList1;//���������������ʱ�б���ϵͳʱ��������xTickCountû�����ʱ����һ���б���xTickCountû�����ʱ����һ���б���xTickCount�����������һ���б�
static List_t xDelayedTaskList2;
static List_t * volatile pxDelayedTaskList;//������ʱ�б�ָ�룬ָ��xTickCountû�����ʱʹ�õ������б�
static List_t * volatile pxOverflowDelayedTaskList;//������ʱ�б�ָ�룬ָ��xTickCount���ʱʹ�õ������б�
static TickType_t xNextTaskUnblockTime;
static volatile BaseType_t xNumOfOverflows = ( BaseType_t ) 0;//��¼xTickCount�������

extern TickType_t xTickCount;



/* ����������ȼ��ľ�������ͨ�÷��� */
#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 0 )
		/* uxTopReadyPriority ����Ǿ��������������ȼ� */
		#define taskRECORD_READY_PRIORITY( uxPriority )\
		{\
			if( ( uxPriority ) > uxTopReadyPriority )\
			{\
					uxTopReadyPriority = ( uxPriority );\
			}\
		}/* taskRECORD_READY_PRIORITY */
/*-----------------------------------------------------------*/
		#define taskSELECT_HIGHEST_PRIORITY_TASK()\
		{\
				UBaseType_t uxTopPriority = uxTopReadyPriority;\
				/* Ѱ�Ұ������������������ȼ��Ķ��� */\
				while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )\
				{\
						--uxTopPriority;\
				}\
				/* ��ȡ���ȼ���ߵľ�������� TCB��Ȼ����µ� pxCurrentTCB */\
				listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[ uxTopPriority ])); \
				/* ���� uxTopReadyPriority */\
				uxTopReadyPriority = uxTopPriority;\
		}/* taskSELECT_HIGHEST_PRIORITY_TASK */
		
/*-----------------------------------------------------------*/
		
/* �������궨��ֻ����ѡ���Ż�����ʱ���ã����ﶨ��Ϊ�� */
#define taskRESET_READY_PRIORITY( uxPriority )
#define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )
		
/* ����������ȼ��ľ������񣺸��ݴ������ܹ��Ż���ķ��� */
#else /* configUSE_PORT_OPTIMISED_TASK_SELECTION */
		
		#define taskRECORD_READY_PRIORITY( uxPriority )\
						portRECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )
		
/*-----------------------------------------------------------*/
		/* Ѱ�����ȼ���ߵ����� */
		#define taskSELECT_HIGHEST_PRIORITY_TASK()\
		{\
				UBaseType_t uxTopPriority;\
				/* Ѱ��������ȼ� */\
				portGET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );\
				/* ��ȡ���ȼ���ߵľ�������� TCB��Ȼ����µ� pxCurrentTCB */\
				listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );\
		}/* taskSELECT_HIGHEST_PRIORITY_TASK() */
		
/*-----------------------------------------------------------*/
#if 1
		#define taskRESET_READY_PRIORITY( uxPriority )\
		{\
				if(listCURRENT_LIST_LENGTH(&(pxReadyTasksLists[( uxPriority)]))==(UBaseType_t)0)\
				{\
						portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );\
				}\
		}
#else
		#define taskRESET_READY_PRIORITY( uxPriority )\
		{\
				portRESET_READY_PRIORITY((uxPriority),(uxTopReadyPriority));\
		}
#endif
		
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */
		
/* ��������ӵ������б� */
#define prvAddTaskToReadyList( pxTCB )\
					taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );\
					vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ),\
													&( ( pxTCB )->xStateListItem ) );

/* �л���ʱ�б� */
#define taskSWITCH_DELAYED_LISTS()\
{\
		List_t *pxTemp;\
		pxTemp = pxDelayedTaskList;\
		pxDelayedTaskList = pxOverflowDelayedTaskList;\
		pxOverflowDelayedTaskList = pxTemp;\
		xNumOfOverflows++;\
		prvResetNextTaskUnblockTime();\
}
		
#if( configSUPPORT_STATIC_ALLOCATION == 1 )

TaskHandle_t xTaskCreateStatic(   TaskFunction_t pxTaskCode,
                                  const char * const pcName,
																	const uint32_t ulStackDepth,
																	void * const pvParameters,
																	/* �������ȼ�����ֵԽ�����ȼ�Խ�� */
																	UBaseType_t uxPriority,
																	StackType_t * const puxStackBuffer,
																	TCB_t * const pxTaskBuffer   )
{
		TCB_t *pxNewTCB;
		TaskHandle_t xReturn;
		
		if( ( pxTaskBuffer != NULL ) && ( puxStackBuffer != NULL ) )
		{
				pxNewTCB = ( TCB_t * ) pxTaskBuffer;
				pxNewTCB->pxStack = ( StackType_t * ) puxStackBuffer;
				
				/* �����µ����� */
				prvInitialiseNewTask( pxTaskCode,          /* ������� */
				                      pcName,              /* �������ƣ��ַ�����ʽ */
															ulStackDepth,        /* ����ջ��С����λΪ�� */
															pvParameters,        /* �����β� */
															uxPriority,          /* ���ȼ� */
															&xReturn,            /* ������ */
															pxNewTCB);           /* ����ջ��ʼ��ַ */
		
		/* ��������ӵ������б� */
		prvAddNewTaskToReadyList( pxNewTCB );
		}
		else
		{
				xReturn = NULL;
		}
		
		/* ������������������񴴽��ɹ�����ʱ xReturn Ӧ��ָ��������ƿ� */
		return xReturn;
}

#endif  /* configSUPPORT_STATIC_ALLOCATION */

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,
																	const char * const pcName,
																	const uint32_t ulStackDepth,
																	void * const pvParameters,
																	/* �������ȼ�����ֵԽ�����ȼ�Խ�� */
																	UBaseType_t uxPriority,
																	TaskHandle_t * const pxCreatedTask,
																	TCB_t *pxNewTCB )
{
		StackType_t *pxTopOfStack;
		UBaseType_t x;
	
		/* ��ȡջ����ַ */
		pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
		/* ������ 8 �ֽڶ��� */
		pxTopOfStack = ( StackType_t * )\
									 ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x007 ) ) );
		/* ����������ִ洢�� TCB �� */
		for ( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
		{
				pxNewTCB->pcTaskName[ x ] = pcName[ x ];
			
				if( pcName[ x ] == 0x00 )
				{
						break;
				}
		}
		/* �������ֵĳ��Ȳ��ܳ��� configMAX_TASK_NAME_LEN */
		pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
		
		/* ��ʼ�� TCB �е� xStateListItem �ڵ� */
		vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
		/* ���� xStateListItem �ڵ��ӵ���� */
		listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
		
		/* ��ʼ�����ȼ� */
		if( uxPriority >= ( UBaseType_t ) configMAX_PRIORITIES )
		{
				uxPriority = ( UBaseType_t ) configMAX_PRIORITIES - ( UBaseType_t ) 1U;
		}
		pxNewTCB->uxPriority = uxPriority;
		
		/* ��ʼ������ջ */
		pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack,
														 pxTaskCode,
														 pvParameters );
		
		/* ��������ָ��������ƿ� */
		if( ( void * ) pxCreatedTask != NULL )
		{
				*pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
		}
}
//�����б��ʼ��
void prvInitialiseTaskLists( void )
{
		UBaseType_t uxPriority;
	
		for( uxPriority = ( UBaseType_t ) 0U;
				 uxPriority < ( UBaseType_t ) configMAX_PRIORITIES;
				 uxPriority++ )
		{
				vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
		}
		
		vListInitialise( &xDelayedTaskList1 );
		vListInitialise( &xDelayedTaskList2 );
		
		pxDelayedTaskList = &xDelayedTaskList1;
		pxOverflowDelayedTaskList = &xDelayedTaskList2;
}

extern TCB_t Task1TCB;
extern TCB_t Task2TCB;
TCB_t *pxCurrentTCB;

TaskHandle_t xIdleTaskHandle;

void vTaskStartScheduler( void )
{
		/*===============������������start=================*/
		TCB_t *pxIdleTaskTCBBuffer = NULL;          /* ����ָ�����������ƿ� */
		StackType_t *pxIdleTaskStackBuffer = NULL;  /* ���ڿ�������ջ��ʼ��ַ */
		uint32_t ulIdleTaskStackSize;
	
		/* ��ȡ����������ڴ棺����ջ������ TCB */
		vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer,
	                                 &pxIdleTaskStackBuffer,
	                                 &ulIdleTaskStackSize );
		/* ������������ */
		xIdleTaskHandle =
		xTaskCreateStatic( (TaskFunction_t)prvIdleTask,        /* ������� */
	                     (char *)"IDLE",                     /* �������ƣ��ַ�����ʽ */
											 (uint32_t)ulIdleTaskStackSize,      /* ����ջ��С����λΪ�� */
											 (void *) NULL,                      /* �����β� */
											 /* �������ȼ�����ֵԽ�����ȼ�Խ�� */
											 (UBaseType_t) tskIDLE_PRIORITY,
											 (StackType_t *)pxIdleTaskStackBuffer, /* ����ջ��ʼ��ַ */
											 (TCB_t *)pxIdleTaskTCBBuffer );   /* ������ƿ� */
		
		/*===============������������end=================*/
		xNextTaskUnblockTime = portMAX_DELAY;
											 
		xTickCount = ( TickType_t ) 0U;
		
		/* ���������� */
		if( xPortStartScheduler() != pdFALSE )
		{
				/* * �����������ɹ����򲻻᷵�أ��������������� */
				/* �������ԣ�û���� */
		}
}

extern TCB_t IdleTaskTCB;
/* �����������л����� */
#if 1
void vTaskSwitchContext( void )
{
		/* ��ȡ���ȼ���ߵľ�������� TCB��Ȼ����µ� pxCurrentTCB */
		taskSELECT_HIGHEST_PRIORITY_TASK();
}
#else
void vTaskSwitchContext( void )
{
		/* �����ǰ�����ǿ���������ô��ȥ����ִ������ 1 �������� 2�� 
       �������ǵ���ʱʱ���Ƿ����������������ʱʱ���û�е��ڣ� 
       �Ǿͷ��ؼ���ִ�п������� */
		if( pxCurrentTCB == &IdleTaskTCB )
		{
				if(Task1TCB.xTicksToDelay == 0)
				{
						pxCurrentTCB =&Task1TCB;
				}
				else if(Task2TCB.xTicksToDelay == 0)
				{
						pxCurrentTCB =&Task2TCB;
				}
				else
				{
						return;     /* ������ʱ��û�е����򷵻أ�����ִ�п������� */
				}
		}
		else  /* ��ǰ�����ǿ����������ִ�е����� */
		{
				/*�����ǰ���������� 1 �������� 2 �Ļ������������һ������, 
					����������������ʱ�У����л��������� 
					�����ж��µ�ǰ�����Ƿ�Ӧ�ý�����ʱ״̬�� 
					����ǵĻ������л����������񡣷���Ͳ������κ��л� */
				if(pxCurrentTCB == &Task1TCB)
				{
						if(Task2TCB.xTicksToDelay == 0)
						{
								pxCurrentTCB=&Task2TCB;
						}
						else if(pxCurrentTCB->xTicksToDelay != 0)
						{
								pxCurrentTCB = &IdleTaskTCB;
						}
						else
						{
								return;      /* ���أ��������л�����Ϊ�������񶼴�����ʱ�� */
						}
				}
				else if(pxCurrentTCB == &Task2TCB)
				{
						if (Task1TCB.xTicksToDelay == 0)
						{
								pxCurrentTCB =&Task1TCB;
						}
						else if(pxCurrentTCB->xTicksToDelay != 0)
						{
								pxCurrentTCB = &IdleTaskTCB;
						}
						else
						{
								return;      /* ���أ��������л�����Ϊ�������񶼴�����ʱ�� */
						}
				}
		}
}
#endif

/* ������ʱ */
void vTaskDelay( const TickType_t xTicksToDelay )
{
		TCB_t *pxTCB = NULL;
	
		/* ��ȡ��ǰ�����TCB */
		pxTCB = pxCurrentTCB;
	
		/* ������ʱʱ�� */
		//pxTCB->xTicksToDelay = xTicksToDelay;
		
		/* ��������뵽��ʱ�б� */
		prvAddCurrentTaskToDelayedList( xTicksToDelay );
	
		/* �����л� */
		taskYIELD();
}
/* ����ϵͳʱ�� */
//void xTaskIncrementTick( void )
BaseType_t xTaskIncrementTick( void )
{
		TCB_t *pxTCB;
		TickType_t xItemValue;
		BaseType_t xSwitchRequired = pdFALSE;
		
		/* ����ϵͳʱ�������� xTickCount��xTickCount ��һ���� port.c �ж����ȫ�ֱ��� */
		const TickType_t xConstTickCount = xTickCount + 1;
		xTickCount = xConstTickCount;
	
		/* ��� xConstTickCount ��������л���ʱ�б� */
		if( xConstTickCount == ( TickType_t ) 0U )
		{
				taskSWITCH_DELAYED_LISTS();
		}
		/* �������ʱ������ʱ���� */
		if( xConstTickCount >= xNextTaskUnblockTime )
		{
				for( ;; )
				{
						if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
						{
								/* ��ʱ�б�Ϊ�գ����� xNextTaskUnblockTime Ϊ���ܵ����ֵ */
								xNextTaskUnblockTime = portMAX_DELAY;
								break;
						}
						else /* ��ʱ�б�Ϊ�� */
						{
								pxTCB = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
								xItemValue = listGET_LIST_ITEM_VALUE( &(pxTCB->xStateListItem) );
							
								if( xConstTickCount < xItemValue )
								{
										xNextTaskUnblockTime = xItemValue;
										break;
								}
								/* ���������ʱ�б��Ƴ��������ȴ�״̬ */
								( void ) uxListRemove( &( pxTCB->xStateListItem ) );
								
								/* ������ȴ���������ӵ������б� */
								prvAddTaskToReadyList( pxTCB );
								
								#if ( configUSE_PREEMPTION == 1 )
								{
										if ( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
										{
												xSwitchRequired = pdTRUE;
										}
								}
								#endif
						}
				}
		}/* xConstTickCount >= xNextTaskUnblockTime */
		
		#if ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) )
		{
				if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ pxCurrentTCB->uxPriority ] ) ) > ( UBaseType_t ) 1 )
				{
						xSwitchRequired = pdTRUE;
				}
		}
		#endif /* ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) ) */
		
		return xSwitchRequired;
		/* �����л� */
		//portYIELD();
}

void prvIdleTask(void *pvParameters)
{
    
}
		
/* ������������ӵ������б� */
static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB )
{
		/* �����ٽ�� */
		taskENTER_CRITICAL();
		{
				/* ȫ�������ʱ����һ���� */
				uxCurrentNumberOfTasks++;
				
				/* ��� pxCurrentTCB Ϊ�գ��� pxCurrentTCB ָ���´��������� */
				if( pxCurrentTCB == NULL )
				{
						pxCurrentTCB = pxNewTCB;
						
						/* ����ǵ�һ�δ�����������Ҫ��ʼ��������ص��б� */
						if ( uxCurrentNumberOfTasks == ( UBaseType_t ) 1 )
						{
								/* ��ʼ��������ص��б� */
								prvInitialiseTaskLists();
						}
				}
				else/* ��� pxCurrentTCB ��Ϊ�գ�
							 �������������ȼ��� pxCurrentTCB ָ��������ȼ������ TCB */
				{
						if( pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority )
						{
								pxCurrentTCB = pxNewTCB;
						}
				}
				
				/* ��������ӵ������б� */
				prvAddTaskToReadyList( pxNewTCB );
		}
		/* �˳��ٽ�� */
		taskEXIT_CRITICAL();
}

static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait )
{
		TickType_t xTimeToWake;
	
		/* ��ȡϵͳʱ�������� xTickCount ��ֵ */
		const TickType_t xConstTickCount = xTickCount;
	
		/* ������Ӿ����б����Ƴ� */
		if( uxListRemove( &( pxCurrentTCB->xStateListItem ) ) == ( UBaseType_t ) 0)
		{
				/* �����������ȼ�λͼ�ж�Ӧ��λ��� */
				portRESET_READY_PRIORITY( pxCurrentTCB->uxPriority,
																	uxTopReadyPriority );
		}
		
		/* ����������ʱ����ʱ��ϵͳʱ�������� xTickCount ��ֵ�Ƕ��� */
		xTimeToWake = xConstTickCount + xTicksToWait;
		
		/* ����ʱ���ڵ�ֵ����Ϊ�ڵ������ֵ */
		listSET_LIST_ITEM_VALUE( &( pxCurrentTCB->xStateListItem ),
														 xTimeToWake );
		
		/* ��� */
		if( xTimeToWake < xConstTickCount )
		{
				vListInsert( pxOverflowDelayedTaskList,
										 &( pxCurrentTCB->xStateListItem ) );
		}
		else /* û����� */
		{
				
				vListInsert( pxDelayedTaskList,
										 &( pxCurrentTCB->xStateListItem ) );
			
				/* ������һ���������ʱ�̱��� xNextTaskUnblockTime ��ֵ */
				if( xTimeToWake < xNextTaskUnblockTime )
				{
						xNextTaskUnblockTime = xTimeToWake;
				}
		}
}

static void prvResetNextTaskUnblockTime( void )
{
		TCB_t *pxTCB;
		
		if ( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
		{
				/* ��ǰ��ʱ�б�Ϊ�գ������� xNextTaskUnblockTime �������ֵ */
				xNextTaskUnblockTime = portMAX_DELAY;
		}
		else
		{
				/* ��ǰ�б�Ϊ�գ�������������ʱ�����ȡ��ǰ�б��µ�һ���ڵ������ֵ
					 Ȼ�󽫸ýڵ������ֵ���µ� xNextTaskUnblockTime */
				( pxTCB ) = ( TCB_t *) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
				xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
		}
}
