//===- MultiTask.h - multiple tasks directory ------------------===//
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
#ifndef _MULTITASK_H
#define _MULTITASK_H
#include "common/BasicMacro.h"
#include <pthread.h>

enum TASK_STATUS {INIT = 0, RUNNING, READY_EXIT, EXITED};

typedef VOID* TaskFunc(VOID* Arg);

class Task
{
private:
    std::queue<VOID*> m_DataQueue;
    pthread_t m_Tid;
    TaskFunc *Func;
    TASK_STATUS m_Status;

private:
    inline VOID* PopData ()
    {
        if (m_DataQueue.empty())
        {
            return NULL;
        }
        
        VOID *Data = m_DataQueue.front();
        m_DataQueue.pop();

        return Data;
    }

public:
    Task (TaskFunc T);

    ~Task ()
    {
    }

    VOID SetStatus (TASK_STATUS Status)
    {
        m_Status = Status;
    }

    TASK_STATUS GetStatus ()
    {
        return m_Status;
    }

    VOID AddData (VOID *Data)
    {
        m_DataQueue.push (Data);
    }

    bool Finish ()
    {
        return !m_DataQueue.empty();
    }

    VOID Run ();
};


class MultiTask
{
private:
    DWORD m_TaskNum;
    std::vector<Task*> m_TaskAry;

private:
    VOID Wait ();
    
public:
    MultiTask (DWORD TaskNum, TaskFunc T);

    ~MultiTask ()
    {
    }

    VOID Run (std::vector<VOID *> &DataSet);   
};


#endif 
