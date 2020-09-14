/***********************************************************
 * Author: Wen Li
 * Date  : 9/09/2020
 * Describe: Graph.c - Graph implementation
 * History:
   <1> 9/09/2020 , create
************************************************************/
#include "Graph.h"

typedef VOID (*DelData) (VOID *Data);

static inline VOID DelList (List *L, DelData Del)
{
    List *Next = L;

    while (L != NULL)
    {
        Next = L->Next;

        if (Del)
        {
            Del (L->data);
        }

        free (L);
        L = Next;
    }

    return;
}

static inline VOID VisitList (List *L, ProcData Pd)
{
    List *Next = L;

    while (L != NULL)
    {
        Next = L->Next;

        if (Pd)
        {
            Pd (L->data);
        }

        L = Next;
    }

    return;
}


static inline List* AllotList ()
{
    List *NL = (List *) malloc (sizeof (List));
    assert (NL != NULL);
    
    NL->data = NULL;

    return NL;
}

static inline void AddOutEdge (Node *N, Edge* Out)
{
    List *NL = AllotList ();
    NL->data = Out;

    NL->Next = N->OutEdge;
    N->OutEdge = NL;

    return;    
}


static inline void AddInEdge (Node *N, Edge* In)
{
    List *NL = AllotList ();
    NL->data = In;

    NL->Next = N->InEdge;
    N->InEdge = NL;

    return;    
}

static inline VOID DelNode (VOID *Data)
{
    Node *N = (Node *)Data;
    
    DelList (N->InEdge, NULL);
    N->InEdge = NULL;
    
    DelList (N->OutEdge, NULL);
    N->OutEdge = NULL;

    return;
}

static inline VOID DelEdge (VOID *Data)
{
    return;
}


static inline VOID DelGraph (Graph *G)
{
    DelList (G->NodeList, DelNode);
    DelList (G->EdgeList, DelEdge);
    
    return;
}


Graph *CreateGraph (DWORD NDBType, DWORD EDBType)
{
    Graph *G = malloc (sizeof (Graph));
    assert (G != NULL);

    G->EdgeList = NULL;
    G->NodeList = NULL;
    
    G->EdgeNum  = 0;
    G->NodeNum  = 0;

    G->EDBType = EDBType;
    G->NDBType = NDBType;

    return G;
}


Node* AddNode (Graph *G, Node *N)
{
    List *NL = AllotList ();

    NL->data = (VOID*)N;
    assert (NL->data != NULL);

    NL->Next = G->NodeList;
    G->NodeList = NL;
    G->NodeNum++;

    N->InEdge  = NULL;
    N->OutEdge = NULL;
    
    return N;
}


Edge* AddEdge (Graph *G, Node *S, Node *D)
{
    List *NL = AllotList ();
    
    NL->data = (VOID*)malloc (sizeof (Edge));
    assert (NL->data != NULL);

    NL->Next = G->EdgeList;
    G->EdgeList = NL;
    G->EdgeNum++;

    Edge* E = (Edge*) NL->data;
    E->Src = S;
    E->Dst = D;

    AddInEdge (D, E);
    AddOutEdge (S, E);
    
    return E;
}


VOID VisitAllNode (Graph *G, ProcData Pd)
{
    VisitList (G->NodeList, Pd);
    return;
}


