/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Plugin: detect buffer overflow
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

VOID PrintSink (char *Data)
{
    DEBUG ("Incmp -- sink: %s\r\n", Data);
    return;
}

static inline unsigned DbKey (unsigned long Eid)
{
    unsigned Fid  = R_EID2FID(Eid);
    unsigned Lang = R_EID2LANG(Eid);

    return (Fid | (Lang<<28));
}

static inline List* GetStrlenList (DWORD DataHandle, DifNode *DstNode)
{
    DbReq Req;
    DbAck Ack;

    DWORD Key = DbKey (DstNode->EventId);

    Req.dwDataType = DataHandle;
    Req.dwKeyLen   = sizeof (DWORD);
    Req.pKeyCtx    = (BYTE*)(&Key);

    // Query first
    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS);
    }

    return (List *)(Ack.pDataAddr);
}


static inline char* GetIntValue (List *ValueList)
{
    LNode *Header = ValueList->Header;
    while (Header != NULL)
    {
        Variable *V = (Variable *)Header->Data;
        if (V->Type == VT_INTEGER)
        {
            return V->Name;
        }

        Header = Header->Nxt;
    }

    return NULL;
}


static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    if (R_EID2ETY(DstNode->EventId) !=  EVENT_CALL)
    {
        return FALSE;        
    }

    DEBUG ("DeInCmp: %lx \r\n", SrcNode->EventId);

    EventMsg *EM   = &DstNode->EMsg;
    LNode *ValNode = EM->Def.Header;
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type != VT_FUNCTION)
        {
            ValNode = ValNode->Nxt;
            continue;
        }

        List *SinkList  = &Plg->SinkList;
        LNode *SinkNode = SinkList->Header;
        while (SinkNode != NULL)
        {
            char *Function = (char *)SinkNode->Data;
            if (strcmp (Function, Val->Name) == 0)
            {
                if (strcmp (Function, "strlen") == 0)
                {
                    List *StrlenList = GetStrlenList (Plg->DataHandle, DstNode);
                    ListInsert(StrlenList, DstNode);
                    DEBUG ("=====>[Incmp] Insert strlen... \r\n");
                    return FALSE;
                }
                else
                {
                    List *StrlenList = GetStrlenList (Plg->DataHandle, DstNode);
                    DEBUG ("=====>[Incmp] get strlenList[%u]... \r\n", StrlenList->NodeNum);
                    if (StrlenList->NodeNum != 0)
                    {
                        LNode *Header = StrlenList->Header;
                        while (Header != NULL)
                        {
                            DifNode *Nd  = (DifNode*)Header->Data;
                            char *DefInt = GetIntValue(&Nd->EMsg.Def);
                            char *UseInt = GetIntValue(&DstNode->EMsg.Use);

                            DEBUG ("=====>[Incmp] DefInt = %s, UseInt = %s \r\n", DefInt, UseInt);

                            if (DefInt != NULL &&
                                UseInt != NULL &&
                                strcmp (UseInt, DefInt) == 0)
                            {
                                return TRUE;
                            }
                            Header = Header->Nxt;
                        }
                    }
                    else
                    {
                        unsigned UseNum = DstNode->EMsg.Use.NodeNum;
                        LNode *UseHdr = DstNode->EMsg.Use.Header;

                        DEBUG ("=====>[Incmp] UseNum = %u, EventId = %lx \r\n", UseNum, DstNode->EventId);
                        while (UseHdr != NULL)
                        {
                            Variable *V = (Variable *)UseHdr->Data;
                            if (V->Type == VT_GLOBAL && UseNum == 2)
                            {
                                return TRUE;
                            }
                            UseHdr = UseHdr->Nxt;
                        }
                        
                        return FALSE;
                    }
                }             
            }

            SinkNode = SinkNode->Nxt;
        }

        ValNode = ValNode->Nxt;
    }

    return FALSE;
}

void InitIncmp (Plugin *Plg)
{
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;

    /* DB */
    DWORD Ret = DbCreateTable(Plg->DataHandle, sizeof (List), sizeof (DWORD));
    assert (Ret != R_FAIL);
    
    return;
}

