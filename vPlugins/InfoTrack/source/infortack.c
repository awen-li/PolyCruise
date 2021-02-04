/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include "infotrack.h"

static inline VOID InitPluginCtx (Plugin *PlgCtx)
{
    /* source -> a List of Node (path) */
    DWORD Ret = DbCreateTable(PlgCtx->DataHandle, sizeof (ItCtx), sizeof (Node*));
    assert (Ret != R_FAIL);
    
    PlgCtx->InitStatus = TRUE;
    return;
}

static inline ItCtx* GetSrcContext (DWORD Handle, Node *Source)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (Node *);
    Req.pKeyCtx    = (BYTE *)(&Source);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey (&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS); 

        ItCtx *Ctx = (ItCtx *)(Ack.pDataAddr);
        List *LastVisit = &Ctx->LastVisit;
        ListInsert(LastVisit, Source);
        DEBUG("@@@@@@@ INSERT source: [%u]%p\r\n", LastVisit->NodeNum, LastVisit->Header->Data);
    }

    return (ItCtx *)(Ack.pDataAddr);
}

VOID PrintSink (char *Data)
{
    printf ("infotrack -- sink: %s\r\n", Data);
    return;
}

static inline DWORD IsSink (List *SinkList, Node *DstNode)
{
    DifNode *DN = GN_2_DIFN (DstNode);
    if (R_EID2ETY(DN->EventId) !=  EVENT_CALL)
    {
        return FALSE;        
    }

    EventMsg *EM   = &DN->EMsg;
    LNode *ValNode = EM->Def.Header;
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type != VT_FUNCTION)
        {
            ValNode = ValNode->Nxt;
            continue;
        }

        LNode *SinkNode = SinkList->Header;
        while (SinkNode != NULL)
        {
            char *Function = (char *)SinkNode->Data;
            if (strcmp (Function, Val->Name) == 0)
            {
                DEBUG ("@@@@@@@@@@@@ Reach sink: ");
                ViewEMsg (&DN->EMsg);
                return TRUE;
            }

            SinkNode = SinkNode->Nxt;
        }

        ValNode = ValNode->Nxt;
    }

    return FALSE;
}


static inline VOID ProcSource (DWORD DataHandle, Node *Source, List *SinkList)
{
    ItCtx *Ctx = GetSrcContext (DataHandle, Source);
    assert (Ctx != NULL);

    List *LastVisit = &Ctx->LastVisit;

    DEBUG ("\tIN [%u] Header = %p \r\n", LastVisit->NodeNum, LastVisit->Header->Data);
    while (1)
    {
        DWORD Balance = TRUE;
        
        DWORD NodeNum = LastVisit->NodeNum;
        LNode *Last = LastVisit->Header;
        assert (Last != NULL);
        while (NodeNum > 0)
        {
            Node *N = (Node *)Last->Data;
            DifNode *SrcN = GN_2_DIFN (N);
            DEBUG ("SRCnode -> EventId = %u (%p) ", R_EID2ETY(SrcN->EventId), SrcN);
            ViewEMsg (&SrcN->EMsg);

            
            List *OutEdge = &N->OutEdge;
            LNode *LE = OutEdge->Header;
            DWORD DifFlg = FALSE;
            while (LE != NULL)
            {
                Edge *E = (Edge *)LE->Data;
                DifEdge *DE = GE_2_DIFE (E);
                if (!(DE->EdgeType & EDGE_DIF))
                {
                    LE = LE->Nxt;
                    continue;
                }

                Node *DstNode = E->Dst;
                if (IsSink (SinkList, DstNode))
                {
                    ListInsert(&Ctx->Sinks, DstNode);       
                }
                else
                {
                    DifNode *DstN = GN_2_DIFN (DstNode);
                    DEBUG ("DSTnode -> EventId = %u (%p) ", R_EID2ETY(DstN->EventId), DstN);
                    ViewEMsg (&DstN->EMsg);
                    ListInsert(LastVisit, DstNode);
                }

                DifFlg = TRUE;
                LE = LE->Nxt;
            }

            if (DifFlg)
            {
                ListRemove(LastVisit, Last);
                Last = LastVisit->Header;
                Balance = FALSE;
            }
            else
            {
                Last = Last->Nxt;
            }
     
            NodeNum--;
        }

        if (Balance == TRUE)
        {
            DEBUG ("$$$$$$$$$$$$$$ Reach balance......");
            break;
        }
    }

    DEBUG ("\tOUT [%u] Header = %p \r\n", LastVisit->NodeNum, LastVisit->Header->Data);
    return;
}

void InfoTrack (DifAgent *DifA, Plugin *PlgCtx)
{
    DbReq Req;
    DbAck Ack;

    printf ("Entry InfoTrack\r\n");
    if (PlgCtx->InitStatus == FALSE)
    {
        InitPluginCtx (PlgCtx);
        ListVisit(&PlgCtx->SinkList, (ProcData)PrintSink);
    }

    /* Visit all sources, and get context of source: (incremental) */
    DWORD SrcNum = QueryDataNum (DifA->Sources);
    Req.dwDataType = DifA->Sources;
    while (SrcNum > 0)
    {
        Req.dwDataId = SrcNum;
        DWORD Ret = QueryDataByID(&Req, &Ack);
        assert (Ret != R_FAIL);
        
        Node *Source = *((Node **)(Ack.pDataAddr));
        ProcSource (PlgCtx->DataHandle, Source, &PlgCtx->SinkList);

        SrcNum--;
    }
    
    return;
}