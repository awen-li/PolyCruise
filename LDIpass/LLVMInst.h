//===-- LLVMInst.h - LLVM instruction decode ------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LLVMINST_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LLVMINST_H
#include <utility>
#include <vector>
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/CallSite.h"


using namespace llvm;
using namespace std;

typedef vector<Value*>::iterator use_iterator;
    
class LLVMInst
{
private:
    unsigned m_InstOp;
    Instruction* m_Inst;

    Value *m_Def;
    vector<Value*> m_Use;

    Function* m_CallFunc;
    
public:
    LLVMInst (Instruction* Inst)
    {
        m_Inst   = Inst;
        m_InstOp = Inst->getOpcode();
        m_Def    = NULL;
        m_CallFunc = NULL;
        
        Decode ();
    }

    ~LLVMInst ()
    {
    }


public:
    inline bool IsRet ()
    {
        return (m_InstOp == Instruction::Ret);
    }

    inline bool IsCall (Function** Func)
    {
        if (m_InstOp == Instruction::Call ||
            m_InstOp == Instruction::Invoke)
        {
            *Func = m_CallFunc;
            return true;
        }

        return false;
    }

    inline bool IsPHI ()
    {
        return (m_InstOp == Instruction::PHI);
    }

    inline bool IsLoad ()
    {
        return (m_InstOp == Instruction::Load);
    }

    inline bool IsBR ()
    {
        return (m_InstOp == Instruction::Br);
    }

    inline bool IsConst (Value *V)
    {
        return (isa<Constant>(V) && !dyn_cast<GlobalValue>(V));
    }

    inline bool IsIntrinsic ()
    {
        return isa<DbgInfoIntrinsic>(m_Inst);
    }

    inline Value* GetRetValue ()
    {
        if (m_Use.size() == 0)
        {
            return NULL;
        }

        return m_Use[0];
    }

    inline Value* GetDef ()
    {
        return m_Def;
    }

    inline use_iterator begin ()
    {
        return m_Use.begin();
    }

    inline use_iterator end ()
    {
        return m_Use.end();
    }

private:

    inline Function* GetCallee(const Instruction *Inst) 
    {
        if (!isa<CallInst>(Inst) && !isa<InvokeInst>(Inst))
        {
            return NULL;
        }
        
        CallSite Cs(const_cast<Instruction*>(Inst));
        
        return dyn_cast<Function>(Cs.getCalledValue()->stripPointerCasts());;
    }
    
    inline void Decode ()
    {
        unsigned OpNum = m_Inst->getNumOperands ();
        
        switch (m_InstOp) 
        {
            case Instruction::Alloca: 
            case Instruction::Br:
            {
                break;              
            }
            case Instruction::Ret:
            {
                if (m_Inst->getNumOperands() != 0) 
                {
                    m_Use.push_back(m_Inst->getOperand (0));
                }
                
                break;
            }
            case Instruction::Store:
            {
                m_Def = m_Inst->getOperand(1);
                m_Use.push_back(m_Inst->getOperand (0));
                break;
            }
            case Instruction::Call:
            case Instruction::Invoke: 
            {
                Function* Func = GetCallee (m_Inst);
                if (Func != NULL && Func->isIntrinsic ())
                {
                    return;
                }
                
                OpNum--;
                m_CallFunc = Func;
            }
            case Instruction::PHI:
            case Instruction::Load:
            case Instruction::GetElementPtr:      
            case Instruction::Trunc:
            case Instruction::ZExt:
            case Instruction::SExt:
            case Instruction::FPToUI:
            case Instruction::FPToSI:              
            case Instruction::UIToFP:
            case Instruction::SIToFP:
            case Instruction::FPTrunc:
            case Instruction::FPExt:           
            case Instruction::PtrToInt:
            case Instruction::IntToPtr:
            case Instruction::BitCast:
            case Instruction::AddrSpaceCast:
            case Instruction::Select: 
            case Instruction::VAArg: 
            case Instruction::ICmp:
            case Instruction::LShr:
            case Instruction::AShr:
            case Instruction::Shl:
            case Instruction::And:
            case Instruction::Or:
            case Instruction::Xor:
            case Instruction::Add:
            case Instruction::FAdd:
            case Instruction::Sub:
            case Instruction::FSub:
            case Instruction::Mul:
            case Instruction::FMul:
            case Instruction::SDiv:
            case Instruction::UDiv:
            case Instruction::URem:
            case Instruction::SRem:
            {
                m_Def = m_Inst;

                unsigned Index = 0;                
                while (Index < OpNum)
                {
                    Value *U = m_Inst->getOperand (Index);
                    if(IsConst(U))
                    {
                        Index++;
                        continue;                        
                    } 

                    m_Use.push_back(U);
                    Index++;
                }
            }
            default: 
            {
                return ;
            }
        }     
    }   
};


#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LLVMINST_H