/***********************************************************
 * Author: Wen Li
 * Date  : 9/09/2020
 * Describe: Graph.c - Graph implementation
 * History:
   <1> 9/09/2020 , create
************************************************************/
#include "Graph.h"

static DWORD GraphHandle = 0;

static inline void AddOutEdge (Node *N, Edge* Out)
{
    ListInsert (&N->OutEdge, Out);
    return;    
}


static inline void AddInEdge (Node *N, Edge* In)
{
    ListInsert (&N->InEdge, In);
    return;    
}

static inline VOID DelNode (VOID *Data)
{
    Node *N = (Node *)Data;
    
    ListDel (&N->InEdge, NULL);
    memset (&N->InEdge, 0, sizeof (N->InEdge));
    
    ListDel (&N->OutEdge, NULL);
    memset (&N->OutEdge, 0, sizeof (N->OutEdge));

    return;
}

static inline VOID DelEdge (VOID *Data)
{
    return;
}


VOID DelGraph (Graph *G)
{
    ListDel (&G->NodeList, DelNode);
    ListDel (&G->EdgeList, DelEdge);

    return;
}


Graph *GetGraph (DWORD ThreadId, DWORD NDBType, DWORD EDBType)
{
    DWORD Ret;
    DbReq Req;
    DbAck Ack;
    Graph *G;
    
    if (GraphHandle == 0)
    {
        GraphHandle = DB_TYPE_DIF_GRAPH;
        Ret = DbCreateTable(GraphHandle, sizeof (Graph), sizeof (DWORD));
        assert (Ret != R_FAIL);
    }

    Req.dwDataType = GraphHandle;
    Req.dwKeyLen   = sizeof (DWORD);
    Req.pKeyCtx    = (BYTE*)(&ThreadId);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS);
    }

    G = (Graph *)(Ack.pDataAddr); 
    G->EDBType = EDBType;
    G->NDBType = NDBType;
    G->ThreadId= ThreadId;

    return G;
}


VOID AddNode (Graph *G, Node *N)
{
    if (G->Root == NULL)
    {
        G->Root = N;
    }

    ListInsert (&G->NodeList, N);

    return;
}


VOID AddEdge (Graph *G, Edge* E)
{
    ListInsert (&G->EdgeList, E);

    AddInEdge (E->Dst, E);
    AddOutEdge (E->Src, E);
    
    return;
}


Node *GetLastNode (Graph *G)
{
    LNode *Tail = G->NodeList.Tail;
    if (Tail == NULL)
    {
        return NULL;
    }
    
    return (Node *)Tail->Data;
}


DWORD GetGraphNum ()
{
    if (GraphHandle == 0)
    {
        return 0;
    }

    return QueryDataNum(GraphHandle);
}

Graph *GetGraphById (DWORD GraphId)
{
    DWORD Ret;
    DbReq Req;
    DbAck Ack;
    Graph *G;
    
    if (GraphHandle == 0)
    {
        return NULL;
    }

    Req.dwDataType = GraphHandle;
    Req.dwDataId   = GraphId;

    Ack.dwDataId = 0;
    (VOID)QueryDataByID (&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        return NULL;
    }
    
    if (Ack.dwDataId != GraphId)
    {
        DEBUG ("Data not consistent: IN: %d, OUT: %d\r\n", GraphId, Ack.dwDataId);
        return NULL;
    }

    return (Graph *)(Ack.pDataAddr);
}

