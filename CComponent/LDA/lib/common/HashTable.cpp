//===- HashTable.cpp - conmon hash table function api -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

#include "common/Hashtable.h"

using namespace std;

HashTable::HashTable(std::string Name, DWORD KeyLen, DWORD DataLen)
{
	m_strName   = Name;
	m_dwKeyLen  = KeyLen;
	m_dwDataLen = DataLen;
	
	m_dwPailNum  = 65535;
    
	m_ptHashPail = (HashPail*)new HashPail[m_dwPailNum];
}

HashTable::~HashTable ()
{
    if (m_ptHashPail)
    {
        delete m_ptHashPail;
    }

    HashNode* HshNode;
    HashNode* NxtNode;

    HshNode = m_BusyList;
    while (HshNode != NULL)
    {
        NxtNode = HshNode->pDataNxt;
        delete HshNode;
        
        HshNode = NxtNode;   
    }

    HshNode = m_IdleList;
    while (HshNode != NULL)
    {
        NxtNode = HshNode->pDataNxt;
        delete HshNode;
        
        HshNode = NxtNode;   
    }

    return;
    
}


DWORD HashTable::HashKey(BYTE* pKey, DWORD dwKeyLen)
{
	DWORD dwi;
	DWORD dwIndex = 5381;
	BYTE* pCurKey = pKey;

	for(dwi = 0; dwi < dwKeyLen; dwi++)
	{
		dwIndex = ((dwIndex<<5) + dwIndex)  + *pCurKey;
		pCurKey++;
	}

	return dwIndex;
}


HashNode* HashTable::QueryPail(HashNode* NodeHdr, BYTE* pKey, DWORD dwKeyLen)
{
	while(NULL != NodeHdr)
	{
		if(0 == memcmp(KeyArea(NodeHdr), pKey, dwKeyLen))
		{
			return NodeHdr;
		}

		NodeHdr = NodeHdr->pPailNxt;
	}

	return NULL;
}


VOID HashTable::InsertPail(HashNode* ptHashNode)
{
	HashNode* ptNodeHdr;
	HashPail* ptHash;

	ptHash = m_ptHashPail + ptHashNode->dwPailIndex;

	ptNodeHdr = ptHash->pHashHdr;
	if(NULL != ptNodeHdr)
	{
		ptHashNode->pPailPre = NULL;
        ptHashNode->pPailNxt = ptNodeHdr;

		ptNodeHdr->pPailPre = ptHashNode;
	}
	else
	{
		ptHashNode->pPailPre = NULL;
	    ptHashNode->pPailNxt = NULL;
	}

	m_ptHashPail->pHashHdr = ptHashNode;

	return;
}


VOID HashTable::DeletePail(HashNode* ptHashNode)
{
	HashPail* ptHash;

	ptHash = m_ptHashPail + ptHashNode->dwPailIndex;
	if(ptHashNode == ptHash->pHashHdr)
	{
		ptHash->pHashHdr = ptHashNode->pPailNxt;
		if(NULL != ptHash->pHashHdr)
		{
			ptHash->pHashHdr->pPailPre = NULL;
		}		
	}
	else
	{
		ptHashNode->pDataPre->pDataNxt = ptHashNode->pPailNxt;
		if(NULL != ptHashNode->pPailNxt)
		{
			ptHashNode->pPailNxt->pPailPre = ptHashNode->pDataPre;
		}
	}

	ptHashNode->pPailNxt = NULL;
	ptHashNode->pPailPre = NULL;
	ptHashNode->dwPailIndex = 0;

	return;

}


HashNode* HashTable::NewNode()
{
	DWORD dwNodeLen;
	HashNode* ptHashNode;

	dwNodeLen = sizeof(HashNode) + m_dwDataLen + m_dwKeyLen;    
	ptHashNode = (HashNode*)new BYTE[dwNodeLen];


	ptHashNode->dwDataId = ++m_dwCreateNum;
	
	return ptHashNode;
}


HashNode* HashTable::GetIdleNode()
{
	HashNode* HshNode;

    /* get from idle or new */
	if(0 != m_IdleList->dwCurNodeNum)
	{
		HshNode = m_IdleList->pNodeHdr;
        if(m_IdleList->dwCurNodeNum > 1)
		{
			m_IdleList->pNodeHdr = HshNode->pDataNxt;
			m_IdleList->dwCurNodeNum--;
		}
		else
		{
			m_IdleList->dwCurNodeNum = 0;
			m_IdleList->pNodeHdr = NULL;
			m_IdleList->pNodeTail = NULL;
		}
	}
	else
	{
        HshNode = NewNode();
	}

	/* insert busy list */
	if(m_BusyList->dwCurNodeNum > 0)
	{
		HshNode->pDataNxt = NULL;
		HshNode->pDataPre = m_BusyList->pNodeTail;
		
		m_BusyList->pNodeTail->pDataNxt = HshNode;
		m_BusyList->pNodeTail = HshNode;
		m_BusyList->dwCurNodeNum++;
	}
	else
	{
		m_BusyList->pNodeHdr  = HshNode;
		m_BusyList->pNodeTail = HshNode;
		
		HshNode->pDataNxt = NULL;
		HshNode->pDataPre = NULL;
		
		m_BusyList->dwCurNodeNum = 1;
	}
	
	memset(DataArea(HshNode, m_dwKeyLen), 0, sizeof(m_dwDataLen));
	
	return HshNode;
}


VOID HashTable::RelBusyNode(HashNode* BusyNode)
{
    assert (m_BusyList->dwCurNodeNum != 0);

    if(m_BusyList->dwCurNodeNum > 1)
    {
        if(BusyNode != m_BusyList->pNodeHdr &&
           BusyNode != m_BusyList->pNodeTail)
        {
            BusyNode->pDataPre->pDataNxt = BusyNode->pDataNxt;
            BusyNode->pDataNxt->pDataPre = BusyNode->pDataPre;
        }
        else
        {
            if(BusyNode == m_BusyList->pNodeHdr)
            {
                m_BusyList->pNodeHdr = BusyNode->pDataNxt;
                m_BusyList->pNodeHdr->pDataPre = NULL;
            }
            else
            {
                m_BusyList->pNodeTail = BusyNode->pDataPre;
                m_BusyList->pNodeTail->pDataNxt = NULL;				
            }
        }
			
        m_BusyList->dwCurNodeNum--;
    }
    else
    {
        m_BusyList->pNodeHdr = m_BusyList->pNodeTail = NULL;
        m_BusyList->dwCurNodeNum = 0;
	}
	

	if(m_IdleList->dwCurNodeNum > 0)
	{
		m_IdleList->pNodeTail->pDataNxt = BusyNode;
		m_IdleList->pNodeTail = BusyNode;
			
		m_IdleList->dwCurNodeNum++;
	}
	else
	{		
		m_IdleList->pNodeHdr  = BusyNode;
		m_IdleList->pNodeTail = BusyNode;
		m_IdleList->dwCurNodeNum = 1;
	}

	return;
}

DWORD HashTable::Insert(BYTE* Key, DWORD KeyLen, BYTE* Data, DWORD DataLen)
{
    HashNode* Node;

	if(m_dwKeyLen != KeyLen && m_dwDataLen != DataLen)
	{
		return AF_FAIL;
	}

	Node = GetIdleNode();
	if(Node == NULL)
	{
		return AF_FAIL;
	}
	memcpy(KeyArea(Node), Key, KeyLen);

	Node->dwPailIndex = HashKey(Key, KeyLen)%m_dwPailNum;

	InsertPail(Node);
    memcpy(DataArea(Node, m_dwKeyLen), Data, DataLen);
	
	return AF_SUCCESS;
}

DWORD HashTable::Query(BYTE* Key, DWORD KeyLen, BYTE** Data)
{
    HashNode* Node;
	DWORD dwHashIndex;

	if(m_dwKeyLen != KeyLen)
	{
		return AF_FAIL;
	}

	dwHashIndex =HashKey(Key, KeyLen)%m_dwPailNum;
 	Node = QueryPail(m_ptHashPail+dwHashIndex, Key, KeyLen);
	if(Node == NULL)
	{
		return AF_FAIL;
	}

	*Data = (BYTE*)Node;
	return AF_SUCCESS;
}

DWORD HashTable::Remove(BYTE* Key, DWORD KeyLen)
{
	HashNode* Node;

    if (Query (Key, KeyLen, &Node) == AF_FAIL)
    {
        return AF_FAIL;
    }

	DeletePail(Node);

    RelBusyNode(Node);  

	return AF_SUCCESS;
}

