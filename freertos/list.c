#include "list.h"

//链表初始化
void vListInitialiseItem(ListItem_t * const pxItem)
{
  /* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
	pxItem->pvCountainter = NULL;
}

//链表根节点初始化
void vListInitialise( List_t * const pxList )
{
  /* 将链表索引指针指向最后一个节点 */
	pxList->pxIndex = ( ListItem_t * ) &(pxList->xListEnd);
	
	/* 将链表最后一个节点的辅助排序的值设置为最大，确保该节点就是链表的最后节点 */
	pxList->xListEnd.xItemValue = portMAX_DELAY;
	
	/* 将最后一个节点的 pxNext 和 pxPrevious 指针均指向节点自身，表示链表为空 */
	pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
	pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );
	
	/* 初始化链表节点计数器的值为 0，表示链表为空 */
	pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
}

//将节点插到链表尾部
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
{
		ListItem_t * const pxIndex = pxList->pxIndex;
		
		pxNewListItem->pxNext = pxIndex;
		pxNewListItem->pxPrevious = pxIndex->pxPrevious;
		pxIndex->pxPrevious->pxNext = pxNewListItem;
		pxIndex->pxPrevious = pxNewListItem;
	
		/* 记住该节点所在的链表 */
		pxNewListItem->pvCountainter = ( void * ) pxList;
	
		/* 链表节点计数器++ */
		( pxList->uxNumberOfItems )++;
}

//将节点按照升序排列插入到链表
void vListInsert( List_t *const pxList,ListItem_t * const pxNewListItem )
{
		ListItem_t *pxIterator;
		
		/* 获取节点的排序辅助值 */
		const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
		
		/* 寻找节点要插入的位置 */
		if ( xValueOfInsertion == portMAX_DELAY )//处于最末端
		{
				pxIterator = pxList->xListEnd.pxPrevious;
		}
		else
		{
				for ( pxIterator = ( ListItem_t * ) &( pxList->xListEnd );
							pxIterator->pxNext->xItemValue <= xValueOfInsertion;
							pxIterator = pxIterator->pxNext )
				{
						/* 没有事情可做，不断迭代只为了找到节点要插入的位置 */
				}
		}
		
		/* 根据升序排列，将节点插入 */
		pxNewListItem->pxNext = pxIterator->pxNext;
		pxNewListItem->pxNext->pxPrevious = pxNewListItem;
		pxNewListItem->pxPrevious = pxIterator;
		pxIterator->pxNext = pxNewListItem;
		
		/* 记住该节点所在的链表 */
		pxNewListItem->pvCountainter = (void * ) pxList;
		
		/* 链表节点计数器++ */
		( pxList->uxNumberOfItems )++;
}

UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )
{
		/* 获取节点所在的链表 */
		List_t * const pxList = ( List_t * ) pxItemToRemove->pvCountainter;
		
		/* 将指定的节点从链表删除*/
		pxItemToRemove->pxNext->pxPrevious=pxItemToRemove->pxPrevious;
		pxItemToRemove->pxPrevious->pxNext=pxItemToRemove->pxNext;
		
		/*调整链表的节点索引指针 */
		if( pxList -> pxIndex == pxItemToRemove )
		{
				pxList -> pxIndex = pxItemToRemove->pxPrevious;
		}
		/* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
		pxItemToRemove->pvCountainter=NULL;
		
		/* 链表节点计数器-- */
		( pxList->uxNumberOfItems )--;
		
		/* 返回链表中剩余节点的个数 */
		return pxList->uxNumberOfItems;
}

