/***********************************************************
 * Author: Wen Li
 * Date  : 9/9/2020
 * Describe: DifEngine.c  
 * History:
   <1> 9/9/2020 , create
************************************************************/
#include "MacroDef.h"
#include "DifGraph.h"

static DifAgent DifA;

VOID InitDif ()
{
    DifAgent *DA = &DifA;
    
    if (DA->DifGraph == NULL)
    {
        DA->DifGraph = CreateGraph (DB_TYPE_DIF_NODE, DB_TYPE_DIF_EDGE);
        assert (DA->DifGraph != NULL);
    }

    DWORD Ret;
    Graph *DifGraph = DA->DifGraph;
    
    Ret = DbCreateTable(DifGraph->NDBType, sizeof (Node)+sizeof (DifNode), sizeof (ULONG));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DifGraph->EDBType, sizeof (Edge), sizeof (Edge));
    assert (Ret != R_FAIL);

    DA->FuncHandle = DB_TYPE_DIF_FUNC;
    Ret = DbCreateTable(DA->FuncHandle, FUNC_NAME_LEN, sizeof (ULONG));
    assert (Ret != R_FAIL);

    return;
}

static inline VOID AddCallFunc (EventMsg *EMsg)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = DifA.FuncHandle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&EMsg->EventId);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    char *Name = (char *)(Ack.pDataAddr);
    strncpy (Name, EMsg->Def->Var.Name, FUNC_NAME_LEN);
    printf ("Insert function: %s \t\n", Name);

    return;
}

static inline VOID DelDifNode (Node *GN)
{
    DifNode* DifN = GN_2_DIFN (GN);

    DelEventMsg (DifN->EMsg);
    DifN->EMsg = NULL;

    return;        
}


VOID DeInitDif ()
{
    DifAgent *DA = &DifA;
    
    if (DA->DifGraph != NULL)
    {
        VisitAllNode (DA->DifGraph, DelDifNode);
        
        free (DA->DifGraph);
        DA->DifGraph = NULL;
    }

    DelDb ();

    return;
}


DWORD IsEventExist (ULONG Event)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&Event);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId != 0)
    {
        return TRUE;
    }

    return FALSE;
}

static inline DifNode* AddDifNode (ULONG Event)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&Event);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    Node *N    = (Node *)(Ack.pDataAddr);
    N->EventID = Event;

    AddNode (DifGraph, N);

    return GN_2_DIFN (N);
}

VOID DifEngine (ULONG Event, char *Msg)
{
    printf ("[DIF]%lx: %s \r\n", Event, Msg);

    EventMsg *EMsg = DecodeEventMsg (Event, Msg);
    ViewEMsg (EMsg);

    DifNode* DifN = AddDifNode (Event);
    DifN->EMsg = EMsg;

    if (R_EID2IID(EMsg->EventId) != 0)
    {
    }
    else
    {
        AddCallFunc (EMsg);       
    }
    

    return;
}




