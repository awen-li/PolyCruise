//===- Stat.h - Statistics information    -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#ifndef _STAT_H_
#define _STAT_H_
#include "common/BasicMacro.h"

#define TIMEINTERVAL 1000
#define CLOCK_IN_MS() (clock() / (CLOCKS_PER_SEC / TIMEINTERVAL))

typedef enum
{
	E_TIME_UNIT = 0,
	E_NUM_UNIT  = 1,
}STAT_TYPE;

struct TimeUnit
{
	std::string m_StatName;
	double m_StartTime;
    double m_LastRead;
	double m_EndTime;

    DWORD m_MemUse;

    TimeUnit(std::string& StatName)
    {
        m_StatName = StatName;
    }
};

struct NumUnit
{
	std::string m_StatName;
	DWORD m_NumStatistics;

    NumUnit(std::string& StatName)
    {
        m_StatName = StatName;
        m_NumStatistics = 0;
    }
};

class StatUnit
{
private:
	std::map<std::string, TimeUnit*> m_TimeUnit;
	std::map<std::string, NumUnit*> m_NumUnit;

private:
    inline TimeUnit* GetTimeUnit(std::string Name)
    {
        auto it = m_TimeUnit.find(Name);
        if (it == m_TimeUnit.end())
        {
            return NULL;
        }

        return it->second;
    }

    inline NumUnit* GetNumUnit(std::string Name)
    {
        auto it = m_NumUnit.find(Name);
        if (it == m_NumUnit.end())
        {
            return NULL;
        }

        return it->second;
    }
	
public:

    VOID InitTime (std::string StrName);
    double GetTime (std::string StrName);
    DWORD GetMemUse (std::string StrName);

    VOID IncStatNum (std::string StrName, DWORD Num = 0);
    DWORD GetStatNum (std::string StrName);
};

class Stat
{
private:
    static StatUnit *m_StatUnit;

public:
    Stat ()
    {
        if (m_StatUnit == NULL)
        {
            m_StatUnit = new StatUnit();
            assert (m_StatUnit != NULL);
        }
    }
    
    ~Stat (){}

    static VOID StartTime (std::string StrName);
	static VOID EndTime (std::string StrName);
	
    static VOID IncStatNum (std::string StrName, DWORD Num = 0);
    static DWORD GetStatNum (std::string StrName);

    static VOID Release()
    {
        if (m_StatUnit != NULL)
        {
            delete m_StatUnit;
            m_StatUnit = NULL;
        } 
    }

    static DWORD GetPhyMemUse ()
    {
        pid_t pid = getpid();

        std::string FileName = "/proc/" + std::to_string(pid) + "/status";
        FILE *F = fopen (FileName.c_str(), "r");
        assert (F != NULL);

        char Buf[256] = {0};
        while (!feof(F))
        {
            assert (fgets (Buf, sizeof(Buf), F) != NULL);
            if (strstr(Buf, "VmRSS"))
            {
                break;
            }
        }
        fclose(F);

        DWORD MemSize = 0;
        char ItemName[128];
        sscanf (Buf, "%s %u", ItemName, &MemSize);

        return MemSize;
    }
};

#endif 
