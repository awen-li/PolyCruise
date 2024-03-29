
/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: List.h - List implementation
 * History:
   <1> 7/24/2020 , create
************************************************************/
#ifndef _LIST_H_
#define _LIST_H_
#include "MacroDef.h"


#ifdef __cplusplus
extern "C"{
#endif 


typedef struct tag_LNode
{
    VOID *Data;
	struct tag_LNode *Nxt;
    struct tag_LNode *Pre;
}LNode;

typedef struct tag_List
{
    LNode *Header;
    LNode *Tail;

    DWORD NodeNum;
}List;

VOID ListInsert (List *L, VOID *N);
VOID ListRemove (List *L, LNode *N);
List* ListAllot ();

typedef BOOL (*CompData) (VOID *Ldata, VOID *Target);
typedef VOID (*ProcData) (VOID *Data);
typedef VOID (*DelData) (VOID *Data);

VOID ListVisit (List *L, ProcData Proc);
VOID ListDel (List *L, DelData Del);
BOOL ListSearch (List *L, CompData Proc, VOID *Data);


#ifdef __cplusplus
}
#endif


#endif 
