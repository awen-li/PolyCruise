//===- HashTable.h -conmon hash table function api ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// visit directory.
///
//===----------------------------------------------------------------------===//
#ifndef _HASHTABLE_H
#define _HASHTABLE_H
#include "common/BasicMacro.h"

struct HashNode
{
	HashNode* pPailNxt; 
	HashNode* pPailPre; 

	HashNode* pDataNxt;
	HashNode* pDataPre; 

	//BYTE* pKeyArea;   
	//BYTE* pDataArea;  

	DWORD dwDataId;   
	DWORD dwPailIndex;

#define KeyArea(node)            ((BYTE*)(node+1))
#define DataArea(node, keylen)   ((BYTE*)(node+1) + keylen)
};


struct HashPail
{
	HashNode* pHashHdr;
};

struct DataList
{
    HashNode* pNodeHdr;
	HashNode* pNodeTail;
	DWORD dwCurNodeNum;
};


class HashTable
{
private:
    std::string m_strName;
	DWORD m_dwKeyLen;
    DWORD m_dwDataLen;	
	
	HashPail* m_ptHashPail;  
	
	DataList m_BusyList;   
	DataList m_IdleList;  

	DWORD m_dwPailNum;
	
private:
    DWORD Init();
    HashNode* NewNode();
	
    DWORD HashKey(BYTE* pKey, DWORD dwKeyLen);
	
    HashNode* QueryPail(HashNode* NodeHdr, BYTE* pKey, DWORD dwKeyLen);
    VOID InsertPail(HashNode* ptHashNode);
    VOID DeletePail(HashNode* ptHashNode);

    
    HashNode* GetIdleNode();
    VOID RelBusyNode(HashNode* BusyNode);

public:
    HashTable (std::string Name, DWORD KeyLen, DWORD DataLen);
    ~HashTable ();
	
    DWORD Insert(BYTE* Key, DWORD KeyLen, BYTE* Data, DWORD DataLen);

    DWORD Query(BYTE* Key, DWORD KeyLen, BYTE** Data);

    DWORD Remove(BYTE* Key, DWORD KeyLen);

};



#endif 
