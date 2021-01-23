//===- VisitDir.h - visit directory ------------------===//
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
#ifndef _VISITDIR_H
#define _VISITDIR_H
#include "common/BasicMacro.h"

struct VisitDir
{
private:
    char m_CurPath[1024];

    VOID PathVisit(const CHAR* strDir, VOID* Para1);

public:

    VisitDir()
    {
        memset (m_CurPath, 0, sizeof(m_CurPath));
    }
    
    virtual DWORD runOnFile (const CHAR* FileName, VOID* Para1) = 0;

    std::string getSuffix (const CHAR* FileName);
    VOID Visit(const CHAR* strDir, VOID* Para1);

    
};

struct BCCounter:public VisitDir
{
    BCCounter () {}

    DWORD runOnFile (const CHAR* FileName, VOID* Para1) override
	{
		DWORD BCCounter = *(DWORD*)Para1;
			
		if (!getSuffix(FileName).compare("bc"))
		{
			BCCounter++; 
		}

        *(DWORD*)Para1 = BCCounter;

		return AF_SUCCESS;
	}
};

class ModulePath:public VisitDir
{
public:
    ModulePath () {}

    DWORD runOnFile (const CHAR* FileName, VOID* Para1) override
	{
        CHAR Path[512];
		std::vector<std::string> *ModulePathVec = (std::vector<std::string> *)Para1;
			
		if (!getSuffix(FileName).compare("bc") && strstr(FileName, "preopt.bc") != NULL)
		{
            if (getcwd(Path, sizeof(Path)))
            {
                std::string BcPath(Path);
                BcPath.append("/");
                BcPath.append(FileName);       
    			
                ModulePathVec->push_back (BcPath);
            }
		}

		return AF_SUCCESS;
	}
};


#endif 
