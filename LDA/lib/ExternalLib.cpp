//===- ExternalLib.cpp -- External function packet ---------------------------------//
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

#include <algorithm>
#include "ExternalLib.h"


using namespace llvm;
using namespace std;


vector<LibTaintBits> ExternalLib::m_FTaintBits = 
{
    {"atoi", TAINT_ARG0, TAINT_RET},
    {"atof", TAINT_ARG0, TAINT_RET}, 
    {"atol", TAINT_ARG0, TAINT_RET},  
    {"atoll", TAINT_ARG0, TAINT_RET},
    {"tolower", TAINT_ARG0, TAINT_RET}, 
    {"toupper", TAINT_ARG0, TAINT_RET}, 
    {"towlower", TAINT_ARG0, TAINT_RET}, 
    {"towupper", TAINT_ARG0, TAINT_RET}, 
    {"llvm.bswap.i16", TAINT_ARG0, TAINT_RET},  
    {"llvm.bswap.i32", TAINT_ARG0, TAINT_RET},  
    {"htons", TAINT_ARG0, TAINT_RET},
    {"ntohs", TAINT_ARG0, TAINT_RET},
    {"dup2", TAINT_ARG0, TAINT_ARG1}, 
    {"div", TAINT_ARG0, TAINT_RET}, 
    {"dup", TAINT_ARG0, TAINT_RET},
    {"strdup", TAINT_ARG0, TAINT_RET}, 
    {"strndup", TAINT_ARG0, TAINT_RET},
    {"strtok", TAINT_ARG0, TAINT_RET}, 
    {"strtok_r", TAINT_ARG0, TAINT_RET},
    {"realloc", TAINT_ARG0, TAINT_RET},
    {"llvm.memcpy.i32", TAINT_ARG1, TAINT_ARG0}, 
    {"llvm.memcpy.p0i8.p0i8.i32", TAINT_ARG1, TAINT_ARG0},  
    {"llvm.memcpy.i64", TAINT_ARG1, TAINT_ARG0}, 
    {"llvm.memcpy.p0i8.p0i8.i64", TAINT_ARG1, TAINT_ARG0},  
    {"llvm.memmove.i32", TAINT_ARG1, TAINT_ARG0},  
    {"llvm.memmove.p0i8.p0i8.i32", TAINT_ARG1, TAINT_ARG0}, 
    {"llvm.memmove.i64", TAINT_ARG1, TAINT_ARG0},  
    {"llvm.memmove.p0i8.p0i8.i64", TAINT_ARG1, TAINT_ARG0},  
    {"memccpy", TAINT_ARG1, TAINT_ARG0},  
    {"memmove", TAINT_ARG1, TAINT_ARG0}, 
    {"bcopy", TAINT_ARG0, TAINT_ARG1}, 
    {"stpcpy", TAINT_ARG1, TAINT_ARG0},  
    {"strcat", TAINT_ARG1, TAINT_ARG0},  
    {"strcpy", TAINT_ARG1, TAINT_ARG0},   
    {"strncat", TAINT_ARG1, TAINT_ARG0},   
    {"strncpy", TAINT_ARG1, TAINT_ARG0},  
    {"strtod", TAINT_ARG0, TAINT_RET},  
    {"strtof", TAINT_ARG0, TAINT_RET},  
    {"strtol", TAINT_ARG0, TAINT_RET}, 
    {"strtold", TAINT_ARG0, TAINT_RET},
    {"strtoll", TAINT_ARG0, TAINT_RET}, 
    {"strtoul", TAINT_ARG0, TAINT_RET},
    {"strtoull", TAINT_ARG0, TAINT_RET},
    {"sprintf", TAINT_ARG0|TAINT_ARG1|TAINT_ARG2|TAINT_ARG3, TAINT_ARG0}
    
};


VOID ExternalLib::InitExtLib ()
{
    for (auto it  = m_FTaintBits.begin(), ie =  m_FTaintBits.end(); it !=ie; it++)
    {
        LibTaintBits *lgt = &(*it);
        m_ExtFuncMap[lgt->FName] = lgt;
    }

    return;
}



