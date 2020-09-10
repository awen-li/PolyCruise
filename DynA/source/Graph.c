/***********************************************************
 * Author: Wen Li
 * Date  : 9/09/2020
 * Describe: Graph.c - Graph implementation
 * History:
   <1> 9/09/2020 , create
************************************************************/
#include "Graph.h"

typedef VOID (*DelData) (VOID *Data);

static inline VOID DelList (List *L, DelData Dd)
{
    List *Next = L;

    while (L != NULL)
    {
        Next = L->Next;

        if (Dd)
        {
            Dd (L->data);
        }

        free (L);
        L = Next;
    }
}

static inline List* AllotList ()
{
    List *NL = (List *) malloc (sizeof (List));
    assert (NL != NULL);
    
    NL->data = NULL;

    return NL;
}

static inline Node* AllotNode (Graph *G)
{
    List *NL = AllotList ();

    NL->data = (VOID*)malloc (sizeof (Node));
    assert (NL->data != NULL);

    NL->Next = G->NodeList;
    G->NodeList = NL;
    G->NodeNum++;

    Node* N = (Node*) NL->data;
    N->InEdge  = NULL;
    N->OutEdge = NULL;
    
    return N;
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


static inline Edge* AllotEdge (Graph *G, Node *S, Node *D)
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


static inline VOID DelNode (VOID *Data)
{
    Node *N = (Node *)Data;
    
    DelList (N->InEdge, NULL);
    N->InEdge = NULL;
    
    DelList (N->OutEdge, NULL);
    N->OutEdge = NULL;

    free (N);

    return;
}

static inline VOID DelEdge (VOID *Data)
{
    free (Data);

    return;
}


static inline VOID DestroyGraph (Graph *G)
{
    DelList (G->NodeList, DelNode);
    DelList (G->EdgeList, DelEdge);
    
    return;
}


Graph *CreateGraph ()
{
    Graph *G = malloc (sizeof (Graph));
    assert (G != NULL);

    G->EdgeList = NULL;
    G->NodeList = NULL;
    G->EdgeNum  = 0;
    G->NodeNum  = 0;

    return G;
}




