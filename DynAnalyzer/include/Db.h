/***********************************************************
 * Author: Wen Li
 * Date  : 9/01/2020
 * Describe: DbCore.h - memory database 
 * History:
   <1> 9/01/2020 , create
************************************************************/

#ifndef _DB_H_
#define _DB_H_ 
#include "MacroDef.h"

#ifdef __cplusplus
extern "C"{
#endif 


enum DB_TYPE
{
    DB_TYPE_BEGIN=1,
    DB_TYPE_DIF_NODE=DB_TYPE_BEGIN,
    DB_TYPE_DIF_EDGE,
    DB_TYPE_DIF_FUNC,
    DB_TYPE_DIF_THR,
    DB_TYPE_DIF_GRAPH,
    DB_TYPE_DIF_GLV,
    DB_TYPE_DIF_SHARE,
    DB_TYPE_DIF_ADDRMAPING,
    DB_TYPE_DIF_SOURCES,

    DB_TYPE_DIF_PLUGIN_BEGIN=64,
    DB_TYPE_DIF_PATH_GEN,
    DB_TYPE_DIF_PLUGIN_END=128,
    DB_TYPE_END
};

typedef struct tag_DbReq
{
    BYTE* pKeyCtx;
	DWORD dwKeyLen;
	DWORD dwDataType;
	DWORD dwDataId;
}DbReq;

typedef struct tag_DbAck
{
    BYTE* pDataAddr;
	DWORD dwDataId;
	DWORD dwRev;
}DbAck;


DWORD CreateDataByKey(DbReq* ptCreateReq, DbAck* pCreateAck);
DWORD QueryDataByKey(DbReq* ptQueryKey, DbAck* pQueryAck);


DWORD CreateDataNonKey(DbReq* ptCreateReq, DbAck* pCreateAck);
DWORD QueryDataByID(DbReq* ptQueryReq, DbAck* pQueryAck);


DWORD DeleteDataByID(DbReq* ptDelReq);
DWORD DbCreateTable(DWORD dwDataType, DWORD dwDataLen, DWORD dwKeyLen);

DWORD QueryDataNum (DWORD dwDataType);

VOID DelDb ();
VOID InitDb (VOID *Addr);
VOID* GetDbAddr ();

#ifdef __cplusplus
}
#endif


#endif
