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
    set<Value*> m_UseSet;

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

    inline bool IsAlloca ()
    {
        return (m_InstOp == Instruction::Alloca);
    }

    inline bool IsCall ()
    {
        if (m_InstOp == Instruction::Call ||
            m_InstOp == Instruction::Invoke)
        {
            return true;
        }

        return false;
    }

    inline bool IsUnReachable ()
    {
        return (m_InstOp == Instruction::Unreachable);
    }

    inline bool IsAdd ()
    {
        /* ADD + SUB */
        return (m_InstOp == Instruction::Add ||
                m_InstOp == Instruction::FAdd ||
                m_InstOp == Instruction::Sub ||
                m_InstOp == Instruction::FSub);
    }

    inline bool IsMul ()
    {
        return (m_InstOp == Instruction::Mul ||
                m_InstOp == Instruction::FMul);
    }

    
    inline bool IsDiv ()
    {
        return (m_InstOp == Instruction::UDiv ||
                m_InstOp == Instruction::SDiv ||
                m_InstOp == Instruction::FDiv);
    }

    inline bool IsPHI ()
    {
        return (m_InstOp == Instruction::PHI);
    }

    inline bool IsLoad ()
    {
        return (m_InstOp == Instruction::Load);
    }

    inline bool IsStore ()
    {
        return (m_InstOp == Instruction::Store);
    }

    inline bool IsBR ()
    {
        return (m_InstOp == Instruction::Br);
    }

    inline bool IsGep ()
    {
        return (m_InstOp == Instruction::GetElementPtr);
    }

    inline bool IsCmp ()
    {
        return (m_InstOp == Instruction::ICmp);
    }

    inline bool IsConst (Value *V)
    {
        return (isa<Constant>(V) && !dyn_cast<GlobalValue>(V));
    }

    inline bool IsIntrinsic ()
    {
        return isa<DbgInfoIntrinsic>(m_Inst);
    }

    inline bool IsInstrinsicDbgInst() 
    {
        if (llvm::isa<llvm::DbgInfoIntrinsic>(m_Inst))
        {
            return true;
        }
        
        if (m_CallFunc != NULL)
        {
            string FuncName = m_CallFunc->getName().data ();
            if (FuncName.find ("llvm.lifetime") != FuncName.npos)
            {
                return true;
            }
        }
        
        return false;
    }

    inline Value* GetUse (unsigned Index)
    {
        if (m_Use.size() < Index+1)
        {
            return NULL;
        }

        return m_Use[Index];
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

    inline string GetCallName ()
    {
        if (m_CallFunc == NULL)
        {
            return "";
        }
        else
        {
            return m_CallFunc->getName ().data();
        }
    }

    inline Function* GetCallee ()
    {
        return m_CallFunc;
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

    inline void SetUse (unsigned OpNum)
    {
        unsigned Index = 0;                
        while (Index < OpNum)
        {
            Value *U = m_Inst->getOperand (Index);
            auto It = m_UseSet.find (U);
            if (It == m_UseSet.end ())
            {
                if (GEPOperator* gepo = dyn_cast<GEPOperator>(U))
                {
                    m_Use.push_back (gepo->getPointerOperand());
                }
                else
                {
                    m_Use.push_back (U);
                }
            }
            Index++;
        }

        return;
    }
    
    inline void Decode ()
    {
        unsigned OpNum = m_Inst->getNumOperands ();
        
        switch (m_InstOp) 
        {
            case Instruction::Alloca:
            {
                break;
            }
            case Instruction::Br:
            {
                SetUse (OpNum);
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
                
                m_CallFunc = Func;
                SetUse (OpNum-1);

                if (!m_Inst->getType ()->isVoidTy())
                {
                    m_Def = m_Inst;
                }

                break;
            }
            case Instruction::PHI:
            {
                m_Def = m_Inst;
                if (m_Def->getType()->isPointerTy())
                {
                    m_Use.push_back(m_Def);
                }
                else
                {
                    SetUse (OpNum);
                }
                break;
            }
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
                SetUse (OpNum);
                break;
            }
            default: 
            {
                return ;
            }
        }     
    }   
};


#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LLVMINST_H