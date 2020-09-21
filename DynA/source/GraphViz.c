/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: GraphViz.c - Graph Virtualization
 * History:
   <1> 9/18/2020 , create
************************************************************/
#include "TypeDef.h"
#include "DifGraph.h"


static inline VOID WriteHeader (FILE *F, char *GraphName) 
{
    fprintf(F, "digraph \"%s\"{\n", GraphName);
    fprintf(F, "\tlabel=\"%s\";\n", GraphName); 

    return;
}

static inline char* GetNodeLabel(char *Buffer, Node *N) 
{
    DifNode *DN  = GN_2_DIFN (N);
    EventMsg *EM = &DN->EMsg;

    DWORD FID = R_EID2FID (DN->EventId);
    sprintf (Buffer, "F%u: ", FID);

    LNode *Def = EM->Def.Header;
    while (Def != NULL)
    {
        Variable *Var = (Variable *) Def->Data;
        assert (Var != NULL);

        if (Var->Type == VT_FUNCTION && R_EID2IID (DN->EventId) == 0)
        {
            strcat (Buffer, Var->Name);
            return Buffer;
        }
        else
        {
            strcat (Buffer, Var->Name);
        }

        Def = Def->Nxt;
        if (Def != NULL)
        {
            strcat (Buffer, ", ");
        }
    }

    strcat (Buffer, " = ");
    LNode *Use = EM->Use.Header;
    while (Use != NULL)
    {
        Variable *Var = (Variable *) Use->Data;
        assert (Var != NULL);
        strcat (Buffer, Var->Name);

        Use = Use->Nxt;
        if (Use != NULL)
        {
            strcat (Buffer, ", ");
        }
    }

    return Buffer;
}


static inline char* GetNodeAttributes(char *Buffer, Node *N) 
{
    DifNode *DN  = GN_2_DIFN (N);
    
    strcat (Buffer, "color=black");

    return Buffer;
}

static inline char* GetEdgeLabel(char *Buffer, DWORD EdgeType) 
{
    switch (EdgeType)
    {
        case EDGE_CALL:
        {
            strcat (Buffer, "CALL");
            break;
        }
        case EDGE_RET:
        {
            strcat (Buffer, "RET");
            break;
        }
        case EDGE_CF:
        {
            strcat (Buffer, "CF");
            break;
        }
        case EDGE_DIF:
        {
            strcat (Buffer, "DIF");
            break;
        }
        default:
        {
            assert (0);
        }            
    }

    return Buffer;
}

static inline char* GetEdgeAttributes(char *Buffer, DWORD EdgeType) 
{
    switch (EdgeType)
    {
        case EDGE_CALL:
        {
            strcat (Buffer, "color=green");
            break;
        }
        case EDGE_RET:
        {
            strcat (Buffer, "color=yellow");
            break;
        }
        case EDGE_CF:
        {
            strcat (Buffer, "color=black");
            break;
        }
        case EDGE_DIF:
        {
            strcat (Buffer, "color=red");
            break;
        }
        default:
        {
            assert (0);
        }            
    }
    
    return Buffer;
}

static inline VOID WriteNodes(FILE *F, Node *N) 
{
    /* NodeID [color=grey,label="{......}"]; */
    DifNode *DN  = GN_2_DIFN (N);
    char Buffer1[256] = {0};
    char Buffer2[256] = {0};

    fprintf(F, "\tN%u [%s, label=\"{%s}\"]\n", 
            N->Id, GetNodeAttributes(Buffer1, N), GetNodeLabel (Buffer2, N));
    return;        
}


static inline VOID WriteEdge(FILE *F, Edge *E) 
{
    DifEdge *DE  = GE_2_DIFE (E);
    
    DWORD SrcId = E->Src->Id;
    DWORD DstId = E->Dst->Id;

    char Buffer1[256];
    char Buffer2[256];

    DWORD Bit = 0;
    DWORD EdgeType = DE->EdgeType;
    while (EdgeType != 0)
    {
        if (EdgeType & 1)
        {
            memset (Buffer1, 0, sizeof (Buffer1));
            memset (Buffer2, 0, sizeof (Buffer2));
        
            fprintf(F, "\tN%u -> N%u[%s,label=\"{%s}\"]\n", 
                    SrcId, DstId, GetEdgeAttributes (Buffer1, 1<<Bit), GetEdgeLabel(Buffer2, 1<<Bit));
            
        }

        EdgeType = EdgeType>>1;
        Bit++;
    }

    
    return; 
 
}


VOID WiteGraph (char *GName, Graph *G) 
{
    char GvName[256] = {0};

    snprintf (GvName, sizeof (GvName), "%s.dot", GName);
    FILE *F = fopen (GvName, "w");
    if (F == NULL)
    {
        return;
    }
    
    WriteHeader(F, GName);

    LNode *LN = G->NodeList.Header;
    while (LN != NULL)
    {
        Node *N = (Node*)LN->Data;      
        WriteNodes (F, N);

        LNode *LOE = N->OutEdge.Header;
        while (LOE != NULL)
        {
            WriteEdge (F, (Edge *)LOE->Data);

            LOE = LOE->Nxt;
        }

        LN = LN->Nxt;
    }

    fprintf(F, "}\n");
    fclose (F);
}   


