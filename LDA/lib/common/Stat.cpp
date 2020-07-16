//===- Bitmap.cpp - bit map for quick search and check  -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

#include "common/Stat.h"

using namespace std;
StatUnit* Stat::m_StatUnit = NULL;


VOID StatUnit::InitTime (std::string StrName)
{
    TimeUnit *Tu = GetTimeUnit (StrName);
    if (Tu == NULL)
    {
        Tu = new TimeUnit(StrName);
        assert (Tu != NULL);

        m_TimeUnit[StrName] = Tu;
    }

    Tu->m_StartTime = CLOCK_IN_MS();
    Tu->m_EndTime   = 0;

    Tu->m_MemUse = Stat::GetPhyMemUse();

    return;
}

double StatUnit::GetTime (std::string StrName)
{
    TimeUnit *Tu = GetTimeUnit (StrName);
    if (Tu == NULL)
    {
        return 0;
    }

    Tu->m_EndTime = CLOCK_IN_MS();

    return (Tu->m_EndTime - Tu->m_StartTime);
}

DWORD StatUnit::GetMemUse (std::string StrName)
{
    TimeUnit *Tu = GetTimeUnit (StrName);
    if (Tu == NULL)
    {
        return 0;
    }

    return (Stat::GetPhyMemUse() - Tu->m_MemUse);
}


VOID StatUnit::IncStatNum (std::string StrName, DWORD Num)
{
    NumUnit *Nu = GetNumUnit (StrName);
    if (Nu == NULL)
    {
        Nu = new NumUnit(StrName);
        assert (Nu != NULL);

        m_NumUnit[StrName] = Nu;
    }

    Nu->m_NumStatistics += (Num?Num:1);
    return;
}

DWORD StatUnit::GetStatNum (std::string StrName)
{
    NumUnit *Nu = GetNumUnit (StrName);
    if (Nu == NULL)
    {
        return 0;
    }

    return Nu->m_NumStatistics;
}


VOID Stat::StartTime (std::string StrName)
{
    m_StatUnit->InitTime (StrName);
}

VOID Stat::EndTime (std::string StrName)
{
    double Time = m_StatUnit->GetTime (StrName);
    DWORD MemUsage = m_StatUnit->GetMemUse(StrName);
    printf ("%s cost-time: %0.2lf (ms), memory-usage: %u (KB)\r\n", StrName.c_str(), Time, MemUsage);
}
	
VOID Stat::IncStatNum (std::string StrName, DWORD Num)
{
    m_StatUnit->IncStatNum (StrName, Num);
}

DWORD Stat::GetStatNum (std::string StrName)
{
    DWORD Num = m_StatUnit->GetStatNum (StrName);
    printf ("%-16s has count: %d \r\n", StrName.c_str(), Num);

    return Num;
}


