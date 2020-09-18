/***********************************************************
 * Author: Wen Li
 * Date  : 9/09/2020
 * Describe: Graph.c - Graph implementation
 * History:
   <1> 9/09/2020 , create
************************************************************/
#include "Graph.h"

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

    free (G);
    return;
}


Graph *CreateGraph (DWORD NDBType, DWORD EDBType)
{
    Graph *G = malloc (sizeof (Graph));
    assert (G != NULL);

    memset (G, 0, sizeof (Graph));

    G->EDBType = EDBType;
    G->NDBType = NDBType;

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


