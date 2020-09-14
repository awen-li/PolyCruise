/***********************************************************
 * Author: Wen Li
 * Date  : 9/13/2020
 * Describe: EventMsg.c  
 * History:
   <1> 9/13/2020 , create
************************************************************/
#include "Event.h"
#include "EventMsg.h"

static inline DWORD Align4 (DWORD V)
{
    if (V & 0x3)
    {
        return ((V & 0xFFFFFFFC) + 4);
    }
    else
    {
        return V;
    }
}

static inline VarList *AllotVarList (char *Msg, DWORD NameLen, BYTE Type)
{
    VarList *VL = (VarList *)malloc (sizeof (VarList));
    assert (VL != NULL);
    
    Variable *FE = &VL->Var;
    FE->Name = (char *)malloc (Align4(NameLen));
    assert (FE->Name != NULL);

    FE->Type = Type;
    FE->Name = (char*)(FE + 1);
    strncpy (FE->Name, Msg, NameLen);

    return VL;
}

/* {main} */
static inline VOID DeFEvent (EventMsg *EM, char *Msg)
{
    DWORD Len = strlen (Msg);
    assert (Len > 2);

    VarList *VL = AllotVarList (Msg, Len-1, VT_FUNCTION);

    EM->Use = NULL;
    EM->Def = VL;

    return;    
}

static inline DWORD GetVarName (char *Msg)
{
    char *Pos = Msg;

    while (*Pos != MSG_VT)
    {
        Pos++;
    }

    return (Pos-Msg);    
}

static inline BYTE GetVarType (char *Msg)
{
    char *Pos = Msg;    
    assert (*Pos == MSG_VT);
        
    Pos++;
    return (BYTE)(*Pos);
}


/* {add:U=or:U,rem:U} */
static inline VOID DeEvent (EventMsg *EM, char *Msg)
{
    char *Pos = Msg;
    DWORD IsDef = TRUE;

    while (*Pos != MSG_END && 
           *Pos != 0)
    {
        DWORD NameLen = GetVarName (Pos);
        assert (NameLen != 0);

        BYTE Type = GetVarType (Pos+NameLen);
        assert (Type != 0);

        VarList *VL = AllotVarList (Pos, NameLen, Type);
        if (IsDef)
        {
            VL->Next = EM->Def;
            EM->Def = VL;
        }
        else
        {
            VL->Next = EM->Use;
            EM->Use = VL;
        }

        Pos += NameLen+2;
        if (*Pos == MSG_DF)
        {
            IsDef = FALSE;
        }
        Pos++;
    }

    return;    
}

static inline VOID DeBREvent (EventMsg *EM, char *Msg)
{

    return;    
}


/* {or:U} */ 
static inline VOID DeRETEvent (EventMsg *EM, char *Msg)
{
    char *Pos = Msg;

    while (*Pos != MSG_END && 
           *Pos != 0)
    {
        DWORD NameLen = GetVarName (Pos);
        assert (NameLen != 0);

        BYTE Type = GetVarType (Pos+NameLen);
        assert (Type != 0);

        VarList *VL = AllotVarList (Pos, NameLen, Type);
        
        VL->Next = EM->Use;
        EM->Use = VL;

        Pos += NameLen+2;
        Pos++;
    }

    return;        
}

EventMsg *DecodeEventMsg (ULONG EventId, char *Msg)
{
    assert (Msg[0] == MSG_BEGIN);
    Msg++;
    
    EventMsg *EM = (EventMsg *)malloc (sizeof (EventMsg));
    assert (EM != NULL);

    EM->EventId = EventId;
    EM->Def     = NULL;
    EM->Use     = NULL;

    BYTE EventType = R_EID2ETY(EventId);
    switch (EventType)
    {
        case EVENT_FENTRY:
        {
            DeFEvent (EM, Msg);
            break;
        }    
        case EVENT_NR:
        case EVENT_CALL:
        {
            DeEvent (EM, Msg);
            break;
        }
        case EVENT_BR:
        {
            DeBREvent (EM, Msg);
            break;
        }
        case EVENT_RET:
        {
            DeRETEvent (EM, Msg);
            break;
        }
        default:
        {
            assert (0);            
        }
    }

    return EM;
}


static inline VOID DelList (VarList *VL)
{
    VarList *Nxt;

    while (VL != NULL)
    {
        Nxt = VL->Next;
        
        Variable *V = &VL->Var;
        free (V->Name);
        V->Name = NULL;

        free (VL);
        VL = Nxt;
    }

    return;
}

void DelEventMsg (EventMsg *EM)
{
    DelList (EM->Def);
    DelList (EM->Use);
    
    return;
}


