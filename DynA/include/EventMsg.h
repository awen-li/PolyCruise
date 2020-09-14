//===- EventMsg.h -- Event message definition---------------------------------------------//
//
//
// Copyright (C) <2019-2024>  <Wen Li>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//
#ifndef _EVENTMSG_H_
#define _EVENTMSG_H_
#include "MacroDef.h"
#include "Event.h"

typedef struct tag_Variable
{
    BYTE Type;
    char *Name;
}Variable;

typedef struct tag_VarList
{
    Variable Var;
    struct tag_VarList *Next;
}VarList;


typedef struct tag_EventMsg
{
    ULONG EventId;

    VarList *Def;
    VarList *Use;  
}EventMsg;


EventMsg *DeEventMsg (ULONG EventId, char *Msg);

#endif 
