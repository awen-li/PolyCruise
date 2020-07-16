//===- ComQueue.h - common queue with FIFO --------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//               The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_QUEUE_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_QUEUE_H
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>

using namespace std;

template<class Data> class ComQueue 
{
private:
    set<Data> m_Set;
    queue<Data> m_Queue;

public:
    ComQueue() {}

    ~ComQueue() {}

    inline unsigned Size()
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



#endif  //LLVM_LIB_TRANSFORMS_INSTRUMENTATION_QUEUE_H