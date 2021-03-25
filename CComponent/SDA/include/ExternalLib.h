//===- ExternalLib.h -- External function packet ---------------------------------//
//
//
// Copyright (C) <2019-2024>  <Wen Li>
//

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


#ifndef _EXTERNALLIB_H_
#define _EXTERNALLIB_H_
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallSite.h"
#include "common/BasicMacro.h"

using namespace std;

struct LibTaintBits
{
    string FName;
    unsigned InTaintBit;
    unsigned OutTaintBit;

    LibTaintBits (string Name, unsigned In, unsigned Out)
    {
        FName = Name;
        InTaintBit = In;
        OutTaintBit = Out;
    }
};

class ExternalLib 
{
private:
    static vector<LibTaintBits> m_FTaintBits;

    map<string, LibTaintBits*> m_ExtFuncMap;
    unsigned m_IsInit;
    
public:
    
    ExternalLib()
    {
        m_IsInit = 0;
    }

    ~ ExternalLib()
    {
    }

private:
    VOID InitExtLib ();

public:
    void AddFuncSds (string FuncName, unsigned InTaintBit, unsigned OutTaintBit);
    
    inline unsigned ComputeTaintBits (string FuncName, unsigned InTaintBits)
    {
        if (m_IsInit == 0)
        {
            InitExtLib ();
        }
        
        auto It = m_ExtFuncMap.find(FuncName);
        if (It == m_ExtFuncMap.end())
        {
            return TAINT_UNKNOWN;
        }

        LibTaintBits *Ltb = It->second;
        if (Ltb->InTaintBit & InTaintBits)
        {
            return Ltb->OutTaintBit;
        }
        else
        {
            return TAINT_NONE;
        }
    }
    

    
};



#endif 
