
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

typedef VOID (*ProcData) (VOID *Data);
typedef VOID (*DelData) (VOID *Data);

VOID ListVisit (List *L, ProcData Proc);
VOID ListDel (List *L, DelData Del);




#endif 
