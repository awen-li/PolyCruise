//===-- DifGraph.h - Dyn information flow graph ---------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _DIFGRAPH_H_
#define _DIFGRAPH_H_
#include "EventMsg.h"
#include "Graph.h"

enum 
{
    EDGE_CG   = 1,
    EDGE_RET  = 2,
    EDGE_CF   = 4,
    EDGE_DIF  = 8,
    EDGE_THRC = 16
};

#define USE_MAP_NUM (8)
typedef struct tag_UseMap
{
    BYTE UseMap[USE_MAP_NUM];
    DWORD MapNum;   
}UseMap;

typedef struct tag_EventKey
{
    ULONG Event;
    DWORD ThreadId;
    DWORD Rev;
}EventKey;

typedef struct tag_DifNode
{
    ULONG EventId;
    EventMsg EMsg;
}DifNode;

typedef struct tag_DifEdge
{
    DWORD EdgeType;
}DifEdge;

typedef struct tag_DifAgent
{
    DWORD NodeHandle;
    DWORD EdgeHandle;
    DWORD FDifHandle;
    DWORD ThrHandle;
    DWORD GlvHandle;
}DifAgent;



#define DIFN_2_GN(DIFN)    (Node*)((BYTE*)DIFN - sizeof(Node))
#define GN_2_DIFN(GN)      (DifNode*)(GN + 1)

#define DIFE_2_GE(DIFE)    (Node*)((BYTE*)DIFE - sizeof(Edge))
#define GE_2_DIFE(GE)      (DifEdge*)(GE + 1)

#define FUNC_NAME_LEN      (64)
#define VAR_NAME_LEN       (32)

#endif // _DIFENGINE_H_