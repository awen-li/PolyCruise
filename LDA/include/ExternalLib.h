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

typedef enum
{
    TAINT_NULL = 0,

}TAINT_TYPE;

struct LibTaintBits
{
    string FName;
    unsigned InTaintBit;
    unsigned OutTaintBit;
};

class ExternalLib 
{
private:
    static vector<LibTaintBits> m_FTaintBits;

    map<string, LibTaintBits*> m_ExtFuncMap;
    
public:
    
    ExternalLib()
    {
        InitExtLib ();
    }

    ~ ExternalLib()
    {
    }

private:

    TAINT_TYPE Search (string FuncName);
    VOID InitExtLib ();

    
};



#endif 
