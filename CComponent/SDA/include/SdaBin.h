//===-- LdaBin.h - theformat of the bin file for lda ----------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDABIN_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDABIN_H
#include <set>
#include "llvm/ADT/DenseMap.h"

struct CSTaintBin
{
    unsigned long InstID;
    unsigned InTaintBits;
    unsigned OutTaintBits;
    unsigned CalleeNum;
    //unsigned NameLen;
    //char FuncName[NameLen]
};

struct FldaBin
{
    unsigned NameLen;  
    unsigned FuncId;
    unsigned TotalInstNum;
    unsigned BlockNum;
    unsigned TaintInstNum;
    unsigned TaintCINum;
    //char FuncName[NameLen];
    //unsigned long InstID[TaintInstNum]
    //CSTaintBin TaintCI[TaintCINum] 
};

struct LdaBin
{
    unsigned Version;
	unsigned FuncNum;
	//FldaBin[]
};

#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDABIN_H