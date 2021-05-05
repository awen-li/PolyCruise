/***********************************************************
 * Author: Wen Li
 * Date  : 9/01/2020
 * Describe: DbCore.c - memory database 
 * History:
   <1> 9/01/2020 , create
************************************************************/
#include "Db.h"
#include "DbTable.h"

#ifdef __cplusplus
extern "C"{
#endif 

static DbTableManage *g_tTableManage = NULL;



static inline VOID* db_Malloc(ULONG ulMemSize)
{
    VOID *Mem = malloc (ulMemSize);
    assert (Mem != NULL);
    memset (Mem, 0, ulMemSize);
    
    return Mem;
}

static inline DWORD db_PailNum(DWORD dwDataNum)
{
	DWORD dwPailNum = dwDataNum<<3;
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


static inline DbTable* db_Type2Table(DWORD dwDataType)
{
	if(dwDataType >= DB_TYPE_END)
	{
		return NULL;
	}

    DbTableManage *DbManage = g_tTableManage;

	return (DbManage->TableList + dwDataType);
}

static inline BYTE* db_GetActiveMemAddr(DbTable *ptTable)
{   
    MemUnit *MU = &ptTable->MU;
    assert (MU->MLHdr != NULL);

    return MU->MLHdr->MemAddr;
}

static inline DWORD db_GetDataId(DbTable *ptTable)
{  
    DWORD dwDataId;
    
    MemUnit *MU = &ptTable->MU;
    assert (MU->MLHdr != NULL);

    ptTable->dwInitDataNum++;
    dwDataId = (MU->dwUnitNum-1) * ptTable->dwMaxDataNum + ptTable->dwInitDataNum;   

    return dwDataId;
}

static inline HashNode* db_NewNode(DbTable* ptDataTable)
{
    BYTE *DAddr;
	DWORD dwNodeLen;
	HashNode* ptHashNode;

	if(ptDataTable->dwInitDataNum >= ptDataTable->dwMaxDataNum)
	{
		return NULL;
	}

	dwNodeLen  = sizeof(HashNode) + ptDataTable->dwDataLen+ptDataTable->dwKeyLen;
    DAddr      = db_GetActiveMemAddr (ptDataTable);
	ptHashNode = (HashNode*)(DAddr + dwNodeLen*ptDataTable->dwInitDataNum);

	ptHashNode->dwDataId = db_GetDataId(ptDataTable);
	//ptHashNode->pKeyArea  = KeyArea(ptHashNode);
	//ptHashNode->pDataArea = DataArea(ptHashNode, ptDataTable->dwKeyLen);
	
	return ptHashNode;
}


static inline DWORD db_FormatDataNode(DbTable* ptDataTable)
{
	HashNode* ptHashNode;
	HashNode* ptHashNodeTail;
	DataManage* pDataManage;
	DWORD dwDataNum;

	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}

	pDataManage = &ptDataTable->tIdleDataTable;

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
	
	return R_SUCCESS;
}

static inline HashNode* db_ID2Node (DbTable *ptTable, DWORD ID)
{
    DWORD dwNodeLen;
    MemUnit *MU = &ptTable->MU;
    
    DWORD dwSeq = ID/MU->dwNodeNum;
    MemList *ML = MU->MLHdr;
    while (dwSeq != 0 && ML != NULL)
    {
        ML = ML->Nxt;
        dwSeq--;
    }

    assert (dwSeq == 0);
    dwNodeLen  = sizeof(HashNode) + ptTable->dwDataLen+ptTable->dwKeyLen;
    DWORD dwInnerId = ID%MU->dwNodeNum; 
    
    return (HashNode*)(ML->MemAddr + dwNodeLen*(dwInnerId-1));  
}

static inline VOID db_ExtendDataMem(DbTable *ptTable)
{
    DWORD dwRet;
    MemUnit *MU = &ptTable->MU;
 
    ULONG ulMemSize = (sizeof(HashNode) + ptTable->dwDataLen + ptTable->dwKeyLen)*(ptTable->dwMaxDataNum+1);
    MemList *ML = (MemList *)db_Malloc (ulMemSize + sizeof (MemList));
    assert (ML != NULL);

    ML->MemAddr   = (BYTE *)(ML + 1);
    ML->Nxt = MU->MLHdr;      
    MU->MLHdr = ML;
    
    MU->dwNodeNum = ptTable->dwMaxDataNum; 
    MU->dwUnitNum++;

    ptTable->dwInitDataNum = 0;
    dwRet = db_FormatDataNode(ptTable);
    assert (dwRet != R_FAIL);

    //printf ("db_ExtendDataMem:dwUnitNum = %u, dwNodeNum = %u \r\n", MU->dwUnitNum, MU->dwNodeNum);

    return;
}

static inline VOID db_DelTable (DbTable *ptTable)
{
    MemUnit *MU = &ptTable->MU;

    MemList *ML = MU->MLHdr;
    MemList *Next;
    while (NULL != ML)
    {
        Next = ML->Nxt;
        free (ML);
        ML = Next;           
    }

    if (NULL != ptTable->ptHashPail)
    {
        free (ptTable->ptHashPail);
    }
    memset (ptTable, 0, sizeof (DbTable));
    
    return;
}

static inline DWORD db_HashKey(BYTE* pKey, DWORD dwKeyLen)
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

static inline HashNode* db_QueryInsidePail(HashPail* ptHashPail, BYTE* pKey, DWORD dwKeyLen)
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


static inline VOID db_InsertNode2Pail(DbTable* ptDataTable, HashNode* ptHashNode)
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


static inline VOID db_DeleteNodeOfPail(DbTable* ptDataTable, HashNode* ptHashNode)
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


static inline HashNode* db_GetIdleNode(DbTable* ptDataTable)
{
	HashNode* ptHashNode;
	DataManage* pIdleManage;
	DataManage* pBusyManage;

	if(NULL == ptDataTable)
	{
		return NULL;
	}

	pIdleManage = &ptDataTable->tIdleDataTable;
	mutex_lock(&ptDataTable->tIdleTableLock);
    if (0 == pIdleManage->dwCurNodeNum)
    {
        db_ExtendDataMem (ptDataTable);
    }    
	assert(0 != pIdleManage->dwCurNodeNum);
    
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
    mutex_unlock(&ptDataTable->tIdleTableLock);


	pBusyManage = &ptDataTable->tBusyDataTable;
    mutex_lock(&ptDataTable->tBusyTableLock);
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
    mutex_unlock(&ptDataTable->tBusyTableLock);
	
	memset(DataArea(ptHashNode, ptDataTable->dwKeyLen), 0, sizeof(ptDataTable->dwDataLen));
	
	return ptHashNode;
}


static inline VOID db_FreeBusyNode(DbTable* ptDataTable, HashNode* ptBusyNode)
{
	DataManage* pIdleManage;
	DataManage* pBusyManage;
	
	if(NULL == ptDataTable || NULL == ptBusyNode)
	{
		return;
	}

	pBusyManage = &ptDataTable->tBusyDataTable;
    mutex_lock(&ptDataTable->tBusyTableLock);
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
	}
    mutex_unlock(&ptDataTable->tBusyTableLock);

	pIdleManage = &ptDataTable->tIdleDataTable;
	mutex_lock(&ptDataTable->tIdleTableLock);
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
	mutex_unlock(&ptDataTable->tIdleTableLock);

	return;
}

static inline VOID db_MoveNode2BusyEnd(DbTable* ptDataTable, HashNode* ptBusyNode)
{
    DataManage* pBusyManage;

    if(NULL == ptDataTable || NULL == ptBusyNode)
	{
        return;
	}

	pBusyManage = &ptDataTable->tBusyDataTable;
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
	DbTable* ptDataTable;
	DWORD dwDataId;
	
	if(NULL == ptDelReq)
	{
		return R_FAIL;
	}
	
	ptDataTable = db_Type2Table(ptDelReq->dwDataType);
	if(NULL == ptDataTable)
	{
		return R_FAIL;
	}
	
    dwDataId = ptDelReq->dwDataId;
	if(dwDataId == 0 || dwDataId > ptDataTable->dwMaxDataNum)
	{
		return R_FAIL;
	}
	
    ptHashNode = db_ID2Node (ptDataTable, dwDataId);
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}

	if(0 != ptDataTable->dwKeyLen)
	{
		db_DeleteNodeOfPail(ptDataTable, ptHashNode);
	}

	db_FreeBusyNode(ptDataTable, ptHashNode);
    
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
	if(dwDataId == 0 || dwDataId > ptDataTable->dwMaxDataNum)
	{
		return R_FAIL;
	}
	
	
    ptHashNode = db_ID2Node (ptDataTable, dwDataId);
	if(NULL == ptHashNode)
	{
		return R_FAIL;
	}  

	pQueryAck->dwDataId  = ptHashNode->dwDataId;
	pQueryAck->pDataAddr = DataArea(ptHashNode, ptDataTable->dwKeyLen);

	return R_SUCCESS;
}


DWORD DbCreateTable(DWORD dwDataType, DWORD dwDataLen, DWORD dwKeyLen)
{
	DWORD dwAvgThrCap;
	ULONG ulMemSize;
	DbTable* ptCurTable;

    DbTableManage *DbManage = g_tTableManage;

	ptCurTable = DbManage->TableList + dwDataType;		

    ptCurTable->dwDataType = dwDataType;
	ptCurTable->dwDataLen  = dwDataLen;
	ptCurTable->dwKeyLen   = dwKeyLen;
    ptCurTable->dwMaxDataNum  = M_BASE_DATA_NUM;
	
	ptCurTable->dwCreateNum = 0;
	ptCurTable->dwDeleteNum = 0;

	db_ExtendDataMem (ptCurTable);

	mutex_lock_init(&ptCurTable->tIdleTableLock);
    mutex_lock_init(&ptCurTable->tBusyTableLock);

	ptCurTable->dwPailNum  = db_PailNum(ptCurTable->dwMaxDataNum);
	ptCurTable->ptHashPail = (HashPail*)db_Malloc(sizeof(HashPail)*ptCurTable->dwPailNum);
	if(NULL == ptCurTable->ptHashPail)
	{
		return R_FAIL;
	}

	return R_SUCCESS;
}

VOID InitDb (VOID *Addr)
{
    if (Addr == NULL)
    {
        g_tTableManage = (DbTableManage *)db_Malloc (sizeof (DbTableManage));
        assert (g_tTableManage != NULL);
    }
    else
    {
        g_tTableManage = (DbTableManage *)Addr;
    }
    
    return;
}

VOID* GetDbAddr ()
{
    return g_tTableManage;
}

VOID DelDb ()
{
    DbTable* ptTable;
    DWORD dwType = DB_TYPE_BEGIN;
    DbTableManage *DbManage = g_tTableManage;		

    while (dwType < DB_TYPE_END)
    {
        ptTable = DbManage->TableList + dwType;
        db_DelTable (ptTable);
        
        dwType++;
    }

    return;
}


DWORD QueryDataNum (DWORD dwDataType)
{
    DbTable* ptDataTable;

    if(0 == dwDataType || dwDataType >= DB_TYPE_END)
    {
        DEBUG ("Error datatype[%u]\r\n", dwDataType);
        return 0;
    }

    ptDataTable = db_Type2Table(dwDataType);
    if(NULL == ptDataTable)
    {
        DEBUG ("ptDataTable == NULL[%u]\r\n", dwDataType);
        return 0;
    }

    return ptDataTable->tBusyDataTable.dwCurNodeNum;
}

#ifdef __cplusplus
}
#endif


