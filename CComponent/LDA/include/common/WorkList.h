//===- BitQueue.h - work queue with bitmap(FIFO) --------------------------===//
//
//               The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _WORKLIST_H_
#define _WORKLIST_H_
#include "common/Bitmap.h"

using namespace std;

class BitQueue 
{
#define BIT_SIZE (400000)
private:
    std::queue<DWORD> m_List;
    Bitmap *m_Bitmap;

public:
    BitQueue() 
    {
        m_Bitmap = new Bitmap (BIT_SIZE);
    }

    ~BitQueue() 
    {
        delete m_Bitmap;
        m_Bitmap = NULL;
    }
    
    inline VOID InQueue(DWORD elem) 
    {
        if (!m_Bitmap->CheckBit(elem)) 
        {
            m_List.push(elem);
            m_Bitmap->SetBit (elem, 1);
        }
    }
        
    inline DWORD OutQueue() 
    {
        assert(!m_List.empty() && "Trying to dequeue an empty queue!");
        
        DWORD Ret = m_List.front();
        m_List.pop();
        
        m_Bitmap->SetBit(Ret, 0);
        
        return Ret;
    }
    
    inline bool IsEmpty() const 
    { 
        return m_List.empty(); 
    }

    inline DWORD Size()
    {
        return m_List.size();
    }
};

template<class Data> class ComQueue 
{
private:
    set<Data> m_Set;
    queue<Data> m_Queue;

public:
    ComQueue() {}

    ~ComQueue() {}

    inline DWORD Size()
    {
        return m_Queue.size();
    }

    inline bool IsEmpty() const 
    { 
        return m_Queue.empty(); 
    }

    inline bool IsInQueue(Data data) const 
    { 
        if (m_Set.find (data) == m_Set.end())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    inline bool InQueue(Data data) 
    {
        if (IsInQueue (data))
        {
            return false;
        }
        
        m_Queue.push(data);
        m_Set.insert(data);
        return true;
    }
        
    inline Data OutQueue() 
    {
        assert(!m_Set.empty() && "Trying to dequeue an empty queue!");
        
        Data data = m_Queue.front();
        m_Queue.pop();
        
        m_Set.erase(data);
        
        return data;
    }
};



#endif 
