//===- MultiTask.cpp - create multiple task to process -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

#include "common/MultiTask.h"

using namespace std;

VOID* TaskProc(VOID* Arg)
{
    Task *T = (Task *)Arg;

    T->Run ();

    return NULL;
}

Task::Task (TaskFunc *T)
{
    Func = T;
    m_Status = INIT;
    
    int Ret = pthread_create(&m_Tid, NULL, TaskProc, this);
    assert (Ret == 0);
}

VOID Task::Run ()
{
    m_Status = RUNNING;
    
    while (m_Status != READY_EXIT || !Finish())
    {
        while (VOID *Data = PopData ())
        {
            printf("[thread:%-4lu]", m_Tid);
            Func (Data);
        }
    }

    m_Status = EXITED;
}

MultiTask::MultiTask (DWORD TaskNum, TaskFunc T)
{
    m_TaskNum = TaskNum;
    assert (m_TaskNum != 0);

    DWORD Index = 0;
    while (Index < TaskNum)
    {
        Task *Tk = new Task (T);
        assert (Tk != NULL);

        m_TaskAry.push_back(Tk);

        Index++;
    }
}

VOID MultiTask::Wait ()
{
    DWORD Index = 0;
    DWORD ExitNum = 0;
    
    for (auto It = m_TaskAry.begin(), End = m_TaskAry.end(); It != End; It++)
    {
        Task *T = *It;

        T->SetStatus (READY_EXIT);
    }

    while (1)
    {
        ExitNum = 0;
        for (auto It = m_TaskAry.begin(), End = m_TaskAry.end(); It != End; It++)
        {
            Task *T = *It;

            ExitNum += (DWORD)(T->GetStatus() == EXITED);
        }

        if (ExitNum == m_TaskNum)
        {
            break;
        }        
    }
}


VOID MultiTask::Run (std::vector<VOID *> &DataSet)
{
    DWORD Index  = 0;
    DWORD TaskNo;
    
    for (auto It = DataSet.begin(), End = DataSet.end(); It != End; It++)
    {
        TaskNo = Index++ % m_TaskNum;

        Task *T = m_TaskAry[TaskNo];
        assert (T != NULL);
        
        T->AddData (*It);
    }

    Wait ();

    printf ("DataNum = %d, TaskNum = %d: All tasks finished....\r\n", Index, m_TaskNum);

    return;
}



