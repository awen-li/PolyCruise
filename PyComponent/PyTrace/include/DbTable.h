/***********************************************************
 * Author: Wen Li
 * Date  : 9/01/2020
 * Describe: DbTable.h - specific table define for memory database 
 * History:
   <1> 9/01/2020 , create
************************************************************/
#ifndef _DBTATBL_H_
#define _DBTATBL_H_
#include "MacroDef.h"

#define M_BASE_DATA_NUM        (4 * 1024)


typedef struct tag_HashNode
{
	struct tag_HashNode* pPailNxt; 
	struct tag_HashNode* pPailPre; 

	struct tag_HashNode* pDataNxt;
	struct tag_HashNode* pDataPre; 

	//BYTE* pKeyArea;                
	//BYTE* pDataArea;                

	DWORD dwDataId:24;                
	DWORD dwThrNo:8;                  
	DWORD dwPailIndex;               

#define KeyArea(node)            ((BYTE*)(node+1))
#define DataArea(node, keylen)   ((BYTE*)(node+1) + keylen)
}HashNode;


typedef struct tag_HashPail
{
	HashNode* pHashNodeHdr;      
}HashPail;

typedef struct tag_DataManage
{
    HashNode* pHashNodeHdr;
	HashNode* pHashNodeTail;
	DWORD dwCurNodeNum;
	DWORD dwRev;
}DataManage;

typedef struct tag_MemList
{
    BYTE *MemAddr;
    struct tag_MemList *Nxt;
}MemList;

typedef struct tag_MemUnit
{
    DWORD dwUnitNum;
    DWORD dwNodeNum;
    
    MemList *MLHdr;    
}MemUnit;

typedef struct tag_DbTable
{
	DataManage tBusyDataTable;      
	DataManage tIdleDataTable;
    
	mutex_lock_t tIdleTableLock;
    mutex_lock_t tBusyTableLock; 

    HashPail* ptHashPail;          

	DWORD dwDataType;                     
	DWORD dwDataLen;                      

	DWORD dwPailNum;                     
	DWORD dwMaxDataNum;

	DWORD dwInitDataNum;
	DWORD dwKeyLen;                      

	DWORD dwCreateNum;
	DWORD dwDeleteNum;

    MemUnit MU;
}DbTable;


typedef struct tag_DbTableManage
{
    DbTable TableList[DB_TYPE_END];
    DWORD TableNum;
}DbTableManage;


#endif
