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

enum DB_TYPE
{
    DB_TYPE_DEFAULT=1,
    DB_TYPE_END
};

typedef struct tag_DbReq
{
    BYTE* pKeyCtx;
	DWORD dwKeyLen;
	DWORD dwDataType;
	DWORD dwThreadNo;
	DWORD dwDataId;
}DbReq;

typedef struct tag_DbAck
{
    BYTE* pDataAddr;
	DWORD dwDataId;
	DWORD dwRev;
}DbAck;


DWORD db_CreateDataByKey(DbReq* ptCreateReq, DbAck* pCreateAck);
DWORD db_QueryDataByKey(DbReq* ptQueryKey, DbAck* pQueryAck);


DWORD db_CreateDataNonKey(DbReq* ptCreateReq, DbAck* pCreateAck);
DWORD db_QueryDataByID(DbReq* ptQueryReq, DbAck* pQueryAck);


DWORD db_DeleteDataByID(DbReq* ptDelReq);


#endif
