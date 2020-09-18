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
#include "Db.h"
#include "EventMsg.h"
#include "List.h"

typedef struct tag_Node 
{
    ULONG EventID;
    
    List InEdge;
    List OutEdge;

    VOID *Ndata;
}Node;


typedef struct tag_Edge 
{
    Node *Src;
    Node *Dst;

    VOID *Edata;
}Edge;



typedef struct tag_Graph 
{
    List NodeList;
    List EdgeList;

    DWORD NDBType;
    DWORD EDBType;

    Node *Root;
   
}Graph;


Graph *CreateGraph (DWORD NDBType, DWORD EDBType);
VOID AddNode (Graph *G, Node *N);
VOID AddEdge (Graph *G, Edge* E);
VOID DelGraph (Graph *G);






#endif 
