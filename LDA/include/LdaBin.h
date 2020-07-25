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

#define F_NAME_LEN  (64)

struct CSTaintBin
{
	unsigned InstID;
	unsigned InTaintBits;
	unsigned OutTaintBits;
	unsigned CalleeNum;
	//char FuncName[F_NAME_LEN][]
};

struct FldaBin
{
    char FuncName[F_NAME_LEN];
    unsigned FuncId;
	unsigned TaintInstNum;
	unsigned TaintCINum;
	//unsigned InstID[]
	//unsigned TaintCI[] 
};

struct LdaBin
{
    unsigned Version;
	unsigned FuncNum;
	//FldaBin[]
};

#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDABIN_H