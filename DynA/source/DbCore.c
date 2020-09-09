/***********************************************************
 * Author: Wen Li
 * Date  : 9/01/2020
 * Describe: DbCore.c - memory database 
 * History:
   <1> 9/01/2020 , create
************************************************************/
#include "Db.h"
#include "DbTable.h"


static DbTableManage g_tTableManage = {0};

INLINE VOID* db_Malloc(ULONG ulMemSize)
{
	return malloc (ulMemSize);
}


INLINE DWORD db_PailNum(DWORD dwDataNum)
{
	DWORD dwPailNum = M_BASE_DATA_NUM+dwDataNum;
	DWORD dwi;

	while(1)
	{
		dwi = 2;
		while(dwi < 1000)
		{
			if(!(dwPailNum%dwi))
			{
				break;
			}
			dwi++;
		}
		dwPailNum++;

		if(1000 == dwi)
		{
			break;
		}
	}

	return dwPailNum;
}


INLINE DbTable* db_Type2Table(DWORD dwDataType)
{
	if(dwDataType >= DB_TYPE_END)
	{
		return NULL;
	}

    DbTableManage *DbManage = &g_tTableManage;

	return (DbManage->TableList + dwDataType);
}


INLINE HashNode* db_NewNode(DbTable* ptDataTable)
{
	DWORD dwNodeLen;
	HashNode* ptHashNode;

	if(ptDataTable->dwInitDataNum >= ptDataTable->dwMaxDataNum)
	{
		return NULL;
	}

	dwNodeLen = sizeof(HashNode) + ptDataTable->dwDataLen+ptDataTable->dwKeyLen;
	ptHashNode = (HashNode*)((BYTE*)ptDataTable->ptDataHdr + dwNodeLen*ptDataTable->dwInitDataNum);
    ptDataTable->dwInitDataNum++;

	ptHashNode->dwDataId = ptDataTable->dwInitDataNum;
	//ptHashNode->pKeyArea  = KeyArea(ptHashNode);
	//ptHashNode->pDataArea = DataArea(ptHashNode, ptDataTable->dwKeyLen);
	
	return ptHashNode;
}


INLINE DWORD db_FormatDataNode(DbTable* ptDataTable)
{
	HashNode* ptHashNode;
	HashNode* ptHashNodeTail;
	DataManage* pDataManage;
	DWORD dwDataNum;

	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}

	pDataManage = (DataManage*)db_Malloc(sizeof(DataManage));
	if(NULL == pDataManage)
	{
		return R_FAIL;
	}
	ptDataTable->ptIdleDataTable = pDataManage;

	ptHashNodeTail = pDataManage->pHashNodeHdr;
	for(dwDataNum = 0; dwDataNum < ptDataTable->dwMaxDataNum; dwDataNum++)
	{
		ptHashNode = db_NewNode(ptDataTable);
		if(NULL == ptHashNode)
		{
			return R_FAIL;
		}

		if(NULL == pDataManage->pHashNodeHdr)
		{
			pDataManage->pHashNodeHdr = ptHashNode;
			ptHashNode->pDataNxt = NULL;
		}
		else
		{
			ptHashNodeTail->pDataNxt = ptHashNode;
		}		

		ptHashNodeTail = ptHashNode;
		pDataManage->dwCurNodeNum++;
	}
	pDataManage->pHashNodeTail= ptHashNodeTail;

	pDataManage = (DataManage*)db_Malloc(sizeof(DataManage));
	if(NULL == pDataManage)
	{
		return R_FAIL;
	}
	ptDataTable->ptBusyDataTable = pDataManage;
	
	return R_SUCCESS;
}


INLINE DWORD db_HashKey(BYTE* pKey, DWORD dwKeyLen)
{
	DWORD dwi;
	DWORD dwIndex = 5381;
	BYTE* pCurKey = pKey;

	for(dwi = 0; dwi < dwKeyLen; dwi++)
	{
		dwIndex = ((dwIndex<<5) + dwIndex)  + *pCurKey;
		pCurKey++;
	}

	return dwIndex;
}

INLINE HashNode* db_QueryInsidePail(HashPail* ptHashPail, BYTE* pKey, DWORD dwKeyLen)
{
	HashNode* ptNodeHdr;

#ifdef _DEBUG
	if(NULL == ptHashPail ||
	   NULL == pKey ||
	   0 == dwKeyLen)
	{
		return NULL;
	}
#endif

	ptNodeHdr = ptHashPail->pHashNodeHdr;
	while(NULL != ptNodeHdr)
	{
		if(0 == memcmp(KeyArea(ptNodeHdr), pKey, dwKeyLen))
		{
			return ptNodeHdr;
		}

		ptNodeHdr = ptNodeHdr->pPailNxt;
	}

	return NULL;
}


INLINE VOID db_InsertNode2Pail(DbTable* ptDataTable, HashNode* ptHashNode)
{
	HashNode* ptHashNodeHdr;
	HashPail* ptHashPail;

	if(NULL == ptHashNode || NULL == ptDataTable)
	{
		return;
	}

	ptHashPail = ptDataTable->ptHashPail + ptHashNode->dwPailIndex;

	ptHashNodeHdr = ptHashPail->pHashNodeHdr;
	if(NULL != ptHashNodeHdr)
	{
		ptHashNode->pPailPre = NULL;
        ptHashNode->pPailNxt = ptHashNodeHdr;

		ptHashNodeHdr->pPailPre = ptHashNode;
	}
	else
	{
		ptHashNode->pPailPre = NULL;
	    ptHashNode->pPailNxt = NULL;
	}
	

	ptHashPail->pHashNodeHdr = ptHashNode;

	return;
}


INLINE VOID db_DeleteNodeOfPail(DbTable* ptDataTable, HashNode* ptHashNode)
{
	HashPail* ptHashPail;

	if(NULL == ptDataTable || NULL == ptHashNode)
	{
		return;
	}

	ptHashPail = ptDataTable->ptHashPail + ptHashNode->dwPailIndex;
	if(ptHashNode == ptHashPail->pHashNodeHdr)
	{
		ptHashPail->pHashNodeHdr = ptHashNode->pPailNxt;
		if(NULL != ptHashPail->pHashNodeHdr)
		{
			ptHashPail->pHashNodeHdr->pPailPre = NULL;
		}		
	}
	else
	{
		ptHashNode->pDataPre->pDataNxt = ptHashNode->pPailNxt;
		if(NULL != ptHashNode->pPailNxt)
		{
			ptHashNode->pPailNxt->pPailPre = ptHashNode->pDataPre;
		}
	}

	ptHashNode->pPailNxt = NULL;
	ptHashNode->pPailPre = NULL;
	ptHashNode->dwPailIndex = 0;

	return;

}


INLINE HashNode* db_GetIdleNode(DbTable* ptDataTable)
{
	HashNode* ptHashNode;
	DataManage* pIdleManage;
	DataManage* pBusyManage;

	if(NULL == ptDataTable)
	{
		return NULL;
	}

	pIdleManage = ptDataTable->ptIdleDataTable;
	mutex_lock(ptDataTable->ptIdleTableLock);
	if(0 != pIdleManage->dwCurNodeNum)
	{
		ptHashNode = pIdleManage->pHashNodeHdr;
        if(pIdleManage->dwCurNodeNum > 1)
		{
			pIdleManage->pHashNodeHdr = ptHashNode->pDataNxt;
			pIdleManage->dwCurNodeNum--;
		}
		else
		{
			pIdleManage->dwCurNodeNum = 0;
			pIdleManage->pHashNodeHdr = NULL;
			pIdleManage->pHashNodeTail = NULL;
		}
		mutex_unlock(ptDataTable->ptIdleTableLock);

		ptHashNode->dwThrNo = ptDataTable->dwThreadNo;
	}
	else
	{
		mutex_unlock(ptDataTable->ptIdleTableLock);			
		return NULL;
	}

	pBusyManage = ptDataTable->ptBusyDataTable;
	if(pBusyManage->dwCurNodeNum > 0)
	{
		ptHashNode->pDataNxt = NULL;
		ptHashNode->pDataPre = pBusyManage->pHashNodeTail;
		
		pBusyManage->pHashNodeTail->pDataNxt = ptHashNode;
		pBusyManage->pHashNodeTail = ptHashNode;
		pBusyManage->dwCurNodeNum++;
	}
	else
	{
		pBusyManage->pHashNodeHdr  = ptHashNode;
		pBusyManage->pHashNodeTail = ptHashNode;
		
		ptHashNode->pDataNxt = NULL;
		ptHashNode->pDataPre = NULL;
		
		pBusyManage->dwCurNodeNum = 1;
	}
	
	memset(DataArea(ptHashNode, ptDataTable->dwKeyLen), 0, sizeof(ptDataTable->dwDataLen));
	
	return ptHashNode;
}


INLINE VOID db_FreeBusyNode(DbTable* ptDataTable, HashNode* ptBusyNode)
{
	DataManage* pIdleManage;
	DataManage* pBusyManage;
	
	if(NULL == ptDataTable || NULL == ptBusyNode)
	{
		return;
	}

	pBusyManage = ptDataTable->ptBusyDataTable;
	if(0 != pBusyManage->dwCurNodeNum)
	{
		if(pBusyManage->dwCurNodeNum > 1)
		{
			if(ptBusyNode != pBusyManage->pHashNodeHdr &&
			   ptBusyNode != pBusyManage->pHashNodeTail)
			{
				ptBusyNode->pDataPre->pDataNxt = ptBusyNode->pDataNxt;
				ptBusyNode->pDataNxt->pDataPre = ptBusyNode->pDataPre;
			}
			else
			{
				if(ptBusyNode == pBusyManage->pHashNodeHdr)
				{
					pBusyManage->pHashNodeHdr = ptBusyNode->pDataNxt;
					pBusyManage->pHashNodeHdr->pDataPre = NULL;
				}
				else
				{
					pBusyManage->pHashNodeTail = ptBusyNode->pDataPre;
					pBusyManage->pHashNodeTail->pDataNxt = NULL;				
				}
			}
			
			pBusyManage->dwCurNodeNum--;
		}
		else
		{
			pBusyManage->pHashNodeHdr = pBusyManage->pHashNodeTail = NULL;
			pBusyManage->dwCurNodeNum = 0;
		}
	}
	else
	{
		assert(0);
		return;		
	}

	pIdleManage = ptDataTable->ptIdleDataTable;
	mutex_lock(ptDataTable->ptIdleTableLock);
	if(pIdleManage->dwCurNodeNum > 0)
	{
		pIdleManage->pHashNodeTail->pDataNxt = ptBusyNode;
		pIdleManage->pHashNodeTail = ptBusyNode;
			
		pIdleManage->dwCurNodeNum++;
	}
	else
	{		
		pIdleManage->pHashNodeHdr  = ptBusyNode;
		pIdleManage->pHashNodeTail = ptBusyNode;
		pIdleManage->dwCurNodeNum = 1;
	}
	mutex_unlock(ptDataTable->ptIdleTableLock);

	return;
}


INLINE VOID db_MoveNode2BusyEnd(DbTable* ptDataTable, HashNode* ptBusyNode)
{
    DataManage* pBusyManage;

    if(NULL == ptDataTable || NULL == ptBusyNode)
	{
        return;
	}

	pBusyManage = ptDataTable->ptBusyDataTable;
	if(ptBusyNode != pBusyManage->pHashNodeHdr &&
	   ptBusyNode != pBusyManage->pHashNodeTail)
	{
		ptBusyNode->pDataPre->pDataNxt = ptBusyNode->pDataNxt;
		ptBusyNode->pDataNxt->pDataPre = ptBusyNode->pDataPre;

		ptBusyNode->pDataNxt = NULL;
		ptBusyNode->pDataPre = pBusyManage->pHashNodeTail;

		pBusyManage->pHashNodeTail->pDataNxt = ptBusyNode;
		pBusyManage->pHashNodeTail = ptBusyNode;
	}
	else
	{
		if(1 == pBusyManage->dwCurNodeNum)
		{
			return;
		}

		if(ptBusyNode == pBusyManage->pHashNodeHdr)
		{
			pBusyManage->pHashNodeHdr = ptBusyNode->pDataNxt;
			pBusyManage->pHashNodeHdr->pDataPre = NULL;

			ptBusyNode->pDataPre = pBusyManage->pHashNodeTail;
			pBusyManage->pHashNodeTail->pDataNxt = ptBusyNode;
			pBusyManage->pHashNodeTail = ptBusyNode;
			ptBusyNode->pDataNxt = NULL;
		}
		else
		{
			return;
		}
	}

    return;
}

DWORD CreateDataByKey(DbReq* ptCreateReq, DbAck* pCreateAck)
{
	HashNode* ptHashNode;
	DbTable* ptDataTable;

	if(NULL == ptCreateReq || NULL == pCreateAck)
	{
		return R_FAIL;
	}

	ptDataTable = db_Type2Table(ptCreateReq->dwDataType);
	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}

	if(ptDataTable->dwKeyLen != ptCreateReq->dwKeyLen)
	{
		return R_FAIL;
	}

	ptHashNode = db_GetIdleNode(ptDataTable);
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}
	memcpy(KeyArea(ptHashNode), ptCreateReq->pKeyCtx, ptDataTable->dwKeyLen);

	ptHashNode->dwPailIndex = db_HashKey(ptCreateReq->pKeyCtx, ptCreateReq->dwKeyLen)%ptDataTable->dwPailNum;

	db_InsertNode2Pail(ptDataTable, ptHashNode);

	ptDataTable->pptId2NodePtr[ptHashNode->dwDataId] = ptHashNode;

	pCreateAck->dwDataId  = ptHashNode->dwDataId;
	pCreateAck->pDataAddr = DataArea(ptHashNode, ptDataTable->dwKeyLen);
	
	return R_SUCCESS;
}


DWORD CreateDataNonKey(DbReq* ptCreateReq, DbAck* pCreateAck)
{
	HashNode* ptHashNode;
	DbTable* ptDataTable;
	
	if(NULL == ptCreateReq || NULL == pCreateAck)
	{
		return R_FAIL;
	}
	
	ptDataTable = db_Type2Table(ptCreateReq->dwDataType);
	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}
	
	ptHashNode = db_GetIdleNode(ptDataTable);
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}
	
	pCreateAck->dwDataId  = ptHashNode->dwDataId;
	pCreateAck->pDataAddr = DataArea(ptHashNode, ptDataTable->dwKeyLen);
	
	return R_SUCCESS;
}


DWORD DeleteDataByID(DbReq* ptDelReq)
{
	HashNode* ptHashNode;
	DbTable* ptDataTable0;
	DbTable* ptRealTable;
	DWORD dwDataId;
	
	if(NULL == ptDelReq)
	{
		return R_FAIL;
	}
	
	ptDataTable0 = db_Type2Table(ptDelReq->dwDataType);
	if(NULL == ptDataTable0)
	{
		return R_FAIL;
	}
	
    dwDataId = ptDelReq->dwDataId;
	if(dwDataId > ptDataTable0->dwMaxDataNum)
	{
		return R_FAIL;
	}
	
    ptHashNode = ptDataTable0->pptId2NodePtr[dwDataId];
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}

	ptRealTable = db_Type2Table(ptDelReq->dwDataType);
	if(NULL == ptRealTable)
	{
		return R_FAIL;
	}

	if(0 != ptRealTable->dwKeyLen)
	{
		db_DeleteNodeOfPail(ptRealTable, ptHashNode);
	}

	db_FreeBusyNode(ptRealTable, ptHashNode);
    
	return R_SUCCESS;
}

DWORD QueryDataByKey(DbReq* ptQueryReq, DbAck* pQueryAck)
{
	HashNode* ptHashNode;
	DbTable* ptDataTable;
	DWORD dwHashIndex;
	
	if(NULL == ptQueryReq || NULL == pQueryAck)
	{
		return R_FAIL;
	}
	
	ptDataTable = db_Type2Table(ptQueryReq->dwDataType);
	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}

	if(ptDataTable->dwKeyLen != ptQueryReq->dwKeyLen)
	{
		return R_FAIL;
	}
    
	dwHashIndex = db_HashKey(ptQueryReq->pKeyCtx, ptQueryReq->dwKeyLen)%ptDataTable->dwPailNum;
	ptHashNode = db_QueryInsidePail(ptDataTable->ptHashPail+dwHashIndex, 
		                            ptQueryReq->pKeyCtx, 
									ptDataTable->dwKeyLen);
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}

	db_MoveNode2BusyEnd(ptDataTable, ptHashNode);
		
	pQueryAck->dwDataId  = ptHashNode->dwDataId;
	pQueryAck->pDataAddr = DataArea(ptHashNode, ptDataTable->dwKeyLen);
	
	return R_SUCCESS;
}


DWORD QueryDataByID(DbReq* ptQueryReq, DbAck* pQueryAck)
{
	HashNode* ptHashNode;
	DbTable* ptDataTable;
	DWORD dwDataId;
	
	if(NULL == ptQueryReq || NULL == pQueryAck)
	{
		return R_FAIL;
	}

	ptDataTable = db_Type2Table(ptQueryReq->dwDataType);
	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}

	dwDataId = ptQueryReq->dwDataId;
	if(dwDataId > ptDataTable->dwMaxDataNum)
	{
		return R_FAIL;
	}
	
	
    ptHashNode = ptDataTable->pptId2NodePtr[dwDataId];
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}  

	pQueryAck->dwDataId  = ptHashNode->dwDataId;
	pQueryAck->pDataAddr = DataArea(ptHashNode, ptDataTable->dwKeyLen);

	return R_SUCCESS;
}


INLINE DWORD DbCreateTable(DWORD dwDataType, DWORD dwDataLen, DWORD dwKeyLen, DWORD dwDataNum)
{
	DWORD dwAvgThrCap;
	ULONG ulMemSize;
	DbTable* ptCurTable;
    DbTableManage *DbManage = &g_tTableManage;

	ptCurTable = DbManage->TableList + dwDataType;		

    ptCurTable->dwDataType = dwDataType;
	ptCurTable->dwDataLen  = dwDataLen;
	ptCurTable->dwKeyLen   = dwKeyLen;
    ptCurTable->dwMaxDataNum  = dwDataNum;
	
	ptCurTable->dwCreateNum = 0;
	ptCurTable->dwDeleteNum = 0;
	ptCurTable->dwInitDataNum = 0;
    
	ulMemSize = (sizeof(HashNode) + ptCurTable->dwDataLen + ptCurTable->dwKeyLen)*(ptCurTable->dwMaxDataNum+1);
	ptCurTable->ptDataHdr = (HashNode*)db_Malloc(ulMemSize);
	if(NULL == ptCurTable->ptDataHdr)
	{
		return R_FAIL;
	}
		
	ulMemSize = sizeof(HashNode*)*(ptCurTable->dwMaxDataNum+1);
	ptCurTable->pptId2NodePtr = (HashNode**)db_Malloc(ulMemSize);
	if(NULL == ptCurTable->pptId2NodePtr)
	{
		return R_FAIL;
	}

	if(R_FAIL == db_FormatDataNode(ptCurTable))
	{
		return R_FAIL;
	}

	ptCurTable->ptIdleTableLock = (mutex_lock_t*)db_Malloc(sizeof(mutex_lock_t));
	if(NULL == ptCurTable->ptIdleTableLock)
	{
		return R_FAIL;
	}
	mutex_lock_init(ptCurTable->ptIdleTableLock);

	ptCurTable->dwPailNum  = db_PailNum(dwDataNum);
	ptCurTable->ptHashPail = (HashPail*)db_Malloc(sizeof(HashPail)*ptCurTable->dwPailNum);
	if(NULL == ptCurTable->ptHashPail)
	{
		return R_FAIL;
	}

	return R_SUCCESS;
}



