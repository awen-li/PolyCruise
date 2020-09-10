//===- Graph.h -- Graph definition---------------------------------------------//
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
#ifndef _GRAPH_H_
#define _GRAPH_H_
#include "MacroDef.h"

typedef struct tag_List
{
    VOID *data;
    struct tag_List *Next;
}List;


typedef struct tag_Node 
{
    ULONG EventID;
    
    List *InEdge;
    List *OutEdge;
}Node;


typedef struct tag_Edge 
{
    Node *Src;
    Node *Dst;
    
    DWORD Attr;
}Edge;



typedef struct tag_Graph 
{
    DWORD NodeNum;
    DWORD EdgeNum;

    List *NodeList;
    List *EdgeList;
   
}Graph;

#endif 
