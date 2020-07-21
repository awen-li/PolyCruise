//===- IBSpass.cpp - IBS pass definition  ---------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/DebugInfo.h>
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm-c/Core.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "Instrumenter.h"

using namespace llvm;
using namespace std;

struct IBSpass : public ModulePass 
{
    static char ID;
    
    IBSpass() : ModulePass(ID)
    {
    }
	    
    StringRef getPassName() const override 
    {
        return StringRef("IBSpass");
    }
	  	
    bool runOnModule(Module &M) override 
    {
        Instrumenter Instrm (&M);

        Instrm.RunInstrm();

        errs()<<"instrument module: " <<M.getName()<<"\r\n";
            
        return true;
   }
};


char IBSpass::ID = 0;
static RegisterPass<IBSpass> X("ibs", "full instrumentation");

static void registerCTPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) 
{
    PM.add(new IBSpass());
}
                               
static RegisterStandardPasses RegisterCTPass(PassManagerBuilder::EP_OptimizerLast, registerCTPass);

/* clang -Xclang -load -Xclang llvmIBSpass.so */



