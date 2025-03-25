#include "list.h"

//�����ʼ��
void vListInitialiseItem(ListItem_t * const pxItem)
{
  /* ��ʼ���ýڵ����ڵ�����Ϊ�գ���ʾ�ڵ㻹û�в����κ����� */
	pxItem->pvCountainter = NULL;
}

//������ڵ��ʼ��
void vListInitialise( List_t * const pxList )
{
  /* ����������ָ��ָ�����һ���ڵ� */
	pxList->pxIndex = ( ListItem_t * ) &(pxList->xListEnd);
	
	/* ���������һ���ڵ�ĸ��������ֵ����Ϊ���ȷ���ýڵ������������ڵ� */
	pxList->xListEnd.xItemValue = portMAX_DELAY;
	
	/* �����һ���ڵ�� pxNext �� pxPrevious ָ���ָ��ڵ�������ʾ����Ϊ�� */
	pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
	pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );
	
	/* ��ʼ������ڵ��������ֵΪ 0����ʾ����Ϊ�� */
	pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
}

//���ڵ�嵽����β��
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
{
		ListItem_t * const pxIndex = pxList->pxIndex;
		
		pxNewListItem->pxNext = pxIndex;
		pxNewListItem->pxPrevious = pxIndex->pxPrevious;
		pxIndex->pxPrevious->pxNext = pxNewListItem;
		pxIndex->pxPrevious = pxNewListItem;
	
		/* ��ס�ýڵ����ڵ����� */
		pxNewListItem->pvCountainter = ( void * ) pxList;
	
		/* ����ڵ������++ */
		( pxList->uxNumberOfItems )++;
}

//���ڵ㰴���������в��뵽����
void vListInsert( List_t *const pxList,ListItem_t * const pxNewListItem )
{
		ListItem_t *pxIterator;
		
		/* ��ȡ�ڵ��������ֵ */
		const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
		
		/* Ѱ�ҽڵ�Ҫ�����λ�� */
		if ( xValueOfInsertion == portMAX_DELAY )//������ĩ��
		{
				pxIterator = pxList->xListEnd.pxPrevious;
		}
		else
		{
				for ( pxIterator = ( ListItem_t * ) &( pxList->xListEnd );
							pxIterator->pxNext->xItemValue <= xValueOfInsertion;
							pxIterator = pxIterator->pxNext )
				{
						/* û��������������ϵ���ֻΪ���ҵ��ڵ�Ҫ�����λ�� */
				}
		}
		
		/* �����������У����ڵ���� */
		pxNewListItem->pxNext = pxIterator->pxNext;
		pxNewListItem->pxNext->pxPrevious = pxNewListItem;
		pxNewListItem->pxPrevious = pxIterator;
		pxIterator->pxNext = pxNewListItem;
		
		/* ��ס�ýڵ����ڵ����� */
		pxNewListItem->pvCountainter = (void * ) pxList;
		
		/* ����ڵ������++ */
		( pxList->uxNumberOfItems )++;
}

UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )
{
		/* ��ȡ�ڵ����ڵ����� */
		List_t * const pxList = ( List_t * ) pxItemToRemove->pvCountainter;
		
		/* ��ָ���Ľڵ������ɾ��*/
		pxItemToRemove->pxNext->pxPrevious=pxItemToRemove->pxPrevious;
		pxItemToRemove->pxPrevious->pxNext=pxItemToRemove->pxNext;
		
		/*��������Ľڵ�����ָ�� */
		if( pxList -> pxIndex == pxItemToRemove )
		{
				pxList -> pxIndex = pxItemToRemove->pxPrevious;
		}
		/* ��ʼ���ýڵ����ڵ�����Ϊ�գ���ʾ�ڵ㻹û�в����κ����� */
		pxItemToRemove->pvCountainter=NULL;
		
		/* ����ڵ������-- */
		( pxList->uxNumberOfItems )--;
		
		/* ����������ʣ��ڵ�ĸ��� */
		return pxList->uxNumberOfItems;
}

