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
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"

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
    inline Instruction* GetInst ()
    {
        return m_Inst;
    }
    
    inline bool IsRet ()
    {
        return (m_InstOp == Instruction::Ret);
    }

    inline bool IsIcmp ()
    {
        return (m_InstOp == Instruction::ICmp);
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

    
    inline Function* GetCallee ()
    {
        return m_CallFunc;
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

    inline bool IsBitCast ()
    {     
        return (m_InstOp == Instruction::BitCast);
    }

    inline bool IsGep ()
    {     
        return (m_InstOp == Instruction::GetElementPtr);
    }

    inline bool IsIntrinsic ()
    {
        return isa<IntrinsicInst>(m_Inst);
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

    inline bool IsConst (Value *V)
    {
        return (isa<Constant>(V) && !dyn_cast<GlobalValue>(V));
    }

    inline Value* GetValue (unsigned Index)
    {
        if (m_Use.size() == 0)
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

    inline Value* GetBaseAddr (Value *V)
    {
        if (GEPOperator* gepo = dyn_cast<GEPOperator>(V))
        {
            return gepo->getPointerOperand();
        }
        else
        {
            return V;
        }
    }

    

private:
    inline VOID SetUse (Value *Use)
    {
        Value *Base = GetBaseAddr (Use);
        m_Use.push_back (Base);
        
        return;
    }

    inline Function* GetCallee(const Instruction *Inst) 
    {
        if (!isa<CallInst>(Inst) && !isa<InvokeInst>(Inst))
        {
            return NULL;
        }
        
        CallSite Cs(const_cast<Instruction*>(Inst));
        
        return dyn_cast<Function>(Cs.getCalledValue()->stripPointerCasts());
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
                if (OpNum > 0)
                {
                    SetUse(m_Inst->getOperand (0));
                }
                break;              
            }
            case Instruction::Ret:
            {
                if (m_Inst->getNumOperands() != 0) 
                {
                    SetUse(m_Inst->getOperand (0));
                }
                
                break;
            }
            case Instruction::Store:
            {
                m_Def = m_Inst->getOperand(1);
                SetUse(m_Inst->getOperand (0));
                break;
            }
            case Instruction::Call:
            case Instruction::Invoke: 
            {
                m_CallFunc = GetCallee (m_Inst);               
                OpNum--;
                
                unsigned Index = 0;                
                while (Index < OpNum)
                {
                    SetUse(m_Inst->getOperand (Index));
                    Index++;
                }

                if (!m_Inst->getType ()->isVoidTy())
                {
                    m_Def = m_Inst;
                }

                break;
            }
            case Instruction::GetElementPtr:
            {
                auto *gepo = cast<GEPOperator>(m_Inst);
                //m_Def = gepo->getPointerOperand();
            }
            case Instruction::PHI:
            case Instruction::Load:     
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
                if (m_Def == NULL)
                {
                    m_Def = m_Inst;
                }

                unsigned Index = 0;                
                while (Index < OpNum)
                {
                    SetUse(m_Inst->getOperand (Index));
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