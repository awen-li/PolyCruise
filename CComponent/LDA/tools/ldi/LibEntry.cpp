//===-- LibEntry.cpp - get entry function for libraries -------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Fts.h"
#include "Lda.h"


using namespace llvm;
using namespace std;


static inline void GetCalledFunc (ModuleManage *Mm, set <Function*> *CalledFunc)
{  
    for (auto It = Mm->func_begin (); It != Mm->func_end (); It++)
    {
        Function *Func  = *It;
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

        for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
        {
            Instruction *Inst = &*itr.getInstructionIterator();
            LLVMInst LI (Inst);
            if (!LI.IsCall ())
            {
                continue;
            }

            Function *Callee = LI.GetCallee ();
            if (Callee == NULL)
            {
                continue;
            }

            CalledFunc->insert (Callee);
        }
    }
    
    return;
}


void GetLibEntry (ModuleManage *Mm, set <Function*> *Entry)
{
    set <Function*> CalledFunc;
    set <Function*> DeclFunc;

    GetCalledFunc (Mm, &CalledFunc);
    for (auto It = Mm->func_begin (); It != Mm->func_end (); It++)
    {
        Function *Func  = *It;
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

        auto ItC = CalledFunc.find (Func);
        if (ItC != CalledFunc.end ())
        {
            continue;
        }
       
        Entry->insert (Func);
        errs()<<"Add entry function: "<<Func->getName ()<<"\r\n";
    }

    return;
}

