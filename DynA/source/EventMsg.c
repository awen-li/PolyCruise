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

static inline Variable *AllotVariable (char *Msg, DWORD NameLen, BYTE Type)
{
    Variable *V = (Variable *)malloc (sizeof (Variable) + Align4(NameLen+1));
    assert (V != NULL);
    
    V->Type = Type;
    V->Name = (char*) (V + 1);
    strncpy (V->Name, Msg, NameLen);

    return V;
}

/* {main} */
static inline VOID DeFEvent (EventMsg *EM, char *Msg)
{
    DWORD Len = strlen (Msg);
    assert (Len > 2);

    Variable *V = AllotVariable (Msg, Len-1, VT_FUNCTION);
    ListInsert (&EM->Def, V);

    return;    
}


static inline DWORD GetVarName (char *Msg)
{
    char *Pos = Msg;

    while (*Pos != MSG_VT && 
           *Pos != MSG_MT &&
           *Pos != MSG_END)
    {
        Pos++;
    }

    return (Pos-Msg);    
}

static inline BYTE GetVarType (char *Msg)
{
    char *Pos = Msg;    
    if (*Pos == MSG_VT)
    {
        Pos++;
        return (BYTE)(*Pos);
    }
    else
    {
        return VT_FUNCTION;
    }  
}

/* {ThreadId:ThreadEntry} */
static inline VOID DeThrcEvent (EventMsg *EM, char *Msg)
{
    char *Pos = Msg;

    /* thread id */
    DWORD ThrIdLen = GetVarName (Pos);
    assert (ThrIdLen != 0);

    Variable *V = AllotVariable (Pos, ThrIdLen, VT_INTEGER);
    ListInsert (&EM->Def, V);

    /* thread name */
    Pos += ThrIdLen + 1;
    DWORD ThrEntryLen = GetVarName (Pos);
    assert (ThrEntryLen != 0);

    V = AllotVariable (Pos, ThrEntryLen, VT_FUNCTION);
    ListInsert (&EM->Def, V);
    
    /* thread para */
    Pos += ThrEntryLen + 1;
    DWORD ParaLen = GetVarName (Pos);
    assert (ParaLen != 0);
    
    V = AllotVariable (Pos, ParaLen, VT_GLOBAL);
    ListInsert (&EM->Use, V);

    return;    
}

/* {add:U=or:U,rem:U} */
static inline VOID DeEvent (EventMsg *EM, char *Msg)
{
    char *Pos = Msg;
    DWORD IsDef = (DWORD)(strchr (Msg, '=') != NULL);

    while (*Pos != MSG_END && 
           *Pos != 0)
    {
        DWORD NameLen = GetVarName (Pos);
        assert (NameLen != 0);

        BYTE Type = GetVarType (Pos+NameLen);
        assert (Type != 0);

        Variable *V = AllotVariable (Pos, NameLen, Type);
        if (IsDef)
        {
            ListInsert (&EM->Def, V);
        }
        else
        {
            ListInsert (&EM->Use, V);
        }

        if (Type != VT_FUNCTION)
        {
            Pos += NameLen+2;
        }
        else
        {
            Pos += NameLen;
        }
        
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

        Variable *V = AllotVariable (Pos, NameLen, Type);       
        ListInsert (&EM->Use, V);

        Pos += NameLen+2;
        Pos++;
    }

    return;        
}

VOID DecodeEventMsg (EventMsg *EM, ULONG EventId, char *Msg)
{
    assert (Msg[0] == MSG_BEGIN);
    Msg++;
    
    BYTE EventType = R_EID2ETY(EventId);
    switch (EventType)
    {
        case EVENT_FENTRY:
        {
            DeFEvent (EM, Msg);
            break;
        } 
        case EVENT_THRC:
        {
            DeThrcEvent (EM, Msg);
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
            DeEvent (EM, Msg);
            break;          
        }
    }

    return;
}


VOID DelVar (VOID *Data)
{
    Variable *V = (Variable *)Data;

    free (V);
    return;
}


void DelEventMsg (EventMsg *EM)
{
    ListDel (&EM->Def, DelVar);
    ListDel (&EM->Use, DelVar);
    
    return;
}


static inline void ViewVar (VOID *Data)
{
    Variable *V = (Variable *)Data;
    printf ("[%c (%s,%lx)] ", V->Type, V->Name, V->Addr);

    return;
}

void ViewEMsg (EventMsg *EM)
{
    printf ("[Definition]:");
    ListVisit (&EM->Def, ViewVar);
    printf ("\r\n");

    printf ("[Use]:");
    ListVisit (&EM->Use, ViewVar);
    printf ("\r\n\r\n");

    return;    
}

