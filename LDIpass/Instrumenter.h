//===-- Instrumenter.h - instruction level instrumentation ----------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "Queue.h"
#include "HookFunc.h"
#include "LLVMInst.h"
#include "BS.h"
#include "SymDependence.h"


using namespace llvm;
using namespace std;


class Instrumenter
{
private:
    Module *m_Module;

    Constant *m_TackFunc;
    Constant *m_InitFunc;
    Constant *m_ExitFunc;

    Instruction* m_Source;
    set<Value *> m_Criterion;
    
    Instruction* m_Sink;

    int m_Type;


public:
    
    Instrumenter(Module *M)
    {
        m_Module = M;

        m_Source    = NULL;

        m_Sink = NULL;

        IBSFunc Ibs;
        m_TackFunc = Ibs.geHookFunction (M);
        assert (m_TackFunc != NULL);
        m_InitFunc = Ibs.geInitFunction (M);
        m_ExitFunc = Ibs.geExitFunction (M);

        m_Type = 4;
    } 

    void RunInstrm ()
    {
        VisitFunction ();
    }
    
private:
        
    inline void AddTrackCall (Instruction* Inst)
    {
        assert (isa<CallInst>(Inst) || isa<InvokeInst>(Inst));
        errs()<<"AddTrackCall: "<<*Inst<<"\r\n";

        IRBuilder<> Builder(Inst);

        Value *Args[4];

        // call
        Args[0] = Builder.CreateGlobalStringPtr("begin-call", "");

        // parameter
        unsigned Index = 0;
        unsigned OpNum = Inst->getNumOperands ()-1;
        while (Index < OpNum)
        {
            Args[Index+1] = Inst->getOperand (Index);
            Index++;
        }
        Type *I32ype = IntegerType::getInt32Ty(m_Module->getContext());
        Args[3] = ConstantInt::get(I32ype, 0, true);

        // create
        //CallInst::Create (m_TackFunc, Args, None, "", Inst);
        Builder.CreateCall(m_TackFunc, Args);

        return;
    }

    inline void AddTrackDefault (Instruction* Inst)
    {
        IRBuilder<> Builder(Inst);

        Value *Args[3];
        Type *I32ype = IntegerType::getInt32Ty(m_Module->getContext());
        Args[0] = ConstantInt::get(I32ype, 0, true);
        Args[1] = ConstantInt::get(I32ype, 0, true);
        Args[2] = ConstantInt::get(I32ype, 0, true);
        Builder.CreateCall(m_TackFunc, Args);

        return;
    }

    inline void AddTrackCallEnd (BS *bs)
    {
        Instruction *Site = bs->GetBsSite();
        assert (Site != NULL);
        errs()<<"AddTrackCallEnd: "<<*Site<<"\r\n";

        IRBuilder<> Builder(Site);
        
        Value *Args[4];

        unsigned Index = 0;
        // end-call
        Args[Index++] = Builder.CreateGlobalStringPtr("end-call", "");

        // in-put
        for (auto In = bs->in_begin (); In != bs->in_end (); In++)
        {
            Args[Index++] = *In;
            errs()<<"InType: "<<*((*In)->getType ())<<"\r\n";
        }

        // out-put
        for (auto Out = bs->out_begin (); Out != bs->out_end (); Out++)
        {
            Args[Index++] = *Out;
            errs()<<"OutType: "<<*((*Out)->getType ())<<"\r\n";
        }

        Type *I32ype = IntegerType::getInt32Ty(m_Module->getContext());
        while (Index < 4)
        {       
            Args[Index++] = ConstantInt::get(I32ype, 0, true);
        }

        Builder.CreateCall(m_TackFunc, Args);
        return;        
    }

    inline void BsInstrument (BS *bs)
    {
        // instrument call
        for (auto It = bs->begin (); It != bs->end(); It++)
        {
            Instruction *CI = *It;
            AddTrackCall (CI);
        }

        // instrument current call results
        AddTrackCallEnd (bs);
    }

    void VisitInst (BasicBlock *B)
    {
        for (auto It = B->begin (); It != B->end (); It++)
        {
            Instruction *Inst = &(*It);
            
            LLVMInst LI (Inst);
            if (LI.IsPHI () || LI.IsIntrinsic())
            {
                continue;
            }

            AddTrackDefault (Inst);
        }
    }

    void VisitBlock (Function *F)
    {
        ComQueue<BasicBlock*> Q;
        set <BasicBlock*> Visited;
        BasicBlock* BH = &F->getEntryBlock();

        Q.InQueue (BH);
        Visited.insert (BH);
        while (!Q.IsEmpty ())
        {
            BasicBlock *B = Q.OutQueue ();
            for (BasicBlock* sucBB : successors(B)) 
            {
                if (Visited.find (sucBB) != Visited.end())
                {
                    continue;
                }
                
                Q.InQueue (sucBB);
                Visited.insert (sucBB);
            }

            errs() <<"Block:  "<<B<<"\r\n";
            VisitInst (B);

            //BS bs (B);
            //BsInstrument (&bs);
        }
    }


    inline void GetCriterion (Function* F)
    {
        map <string, unsigned> Func2Ta;

        switch (m_Type)
        {
            case 0:
            {
                Func2Ta ["compress"] = 1;
                Func2Ta ["uncompress"] = 1;
                Func2Ta ["copyFileName"] = 1;
                Func2Ta ["containsDubiousChars"] = 1;
                Func2Ta ["fileExists"] = 1;
                Func2Ta ["notAStandardFile"] = 1;
                Func2Ta ["countHardLinks"] = 1;
                Func2Ta ["saveInputFileMetaInfo"] = 1;
                Func2Ta ["uncompressStream"] = 1;
                Func2Ta ["applySavedTimeInfoToOutputFile"] = 1;
                break;
            }
            case 1:
            {
                Func2Ta ["GetKey"] = 1 | (1<<8);
                Func2Ta ["CheckKey"] = 1;
                break;
            }
            case 2:
            {
                Func2Ta ["BZ2_bzWrite"] = (1<<2);
                Func2Ta ["BZ2_bzCompress"] = (1<<0);
                Func2Ta ["handle_compress"] = (1<<0);
                Func2Ta ["BZ2_compressBlock"] = (1<<0);
                Func2Ta ["BZ2_blockSort"] = (1<<0);
                Func2Ta ["bsPutUChar"] = (1<<0);
                
                break;
            }
            case 3:
            {           
                break;
            }
            case 4:
            {
                Func2Ta ["BZ2_bzWriteOpen"] = (1<<1);
                Func2Ta ["BZ2_bzWrite"] = (1<<1);
                Func2Ta ["BZ2_bzWriteClose64"] = (1<<1);
                break;
            }
            default:
            {
                break;
            }
        }

        auto It = Func2Ta.find (F->getName ().data());
        if (It == Func2Ta.end())
        {
            return;
        }

        errs()<<"--->"<<m_Type<<" GetCriterion for "<<F->getName ()<<"\r\n";
        unsigned ArgBit = It->second;

        unsigned Index = 0;
        for (Function::arg_iterator itr = F->arg_begin(); itr != F->arg_end(); ++itr) 
        {
            if (ArgBit & (1 << Index))
            {
                m_Criterion.insert (&(*itr));
                errs ()<<"Func: "<<F->getName ()<<"Criterion: argument"<<Index<<"\r\n";
                break;
            }
            Index++;
        }
        
    }

    inline bool IsWhereSource (Function *F)
    {  
        switch (m_Type)
        {
            case 0:
            {
                /* in main */
                if (strcmp("main", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 1:
            {
                /* in main */
                if (strcmp("main", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 2:
            {
                /* compress */
                if (strcmp("compress", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 3:
            {
                /* compress */
                if (strcmp("compress", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 4:
            {
                /* compress */
                if (strcmp("compress", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            default:
            {
                break;                
            }
        }

        return false;
    }

    inline bool IsSourceFunc (Function *F)
    {   
        switch (m_Type)
        {
            case 0:
            {
                /* in main */
                if (strcmp("addFlagsFromEnvVar", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 1:
            {
                /* in main */
                if (strcmp("strtol", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 2:
            {
                /* compress */
                if (strcmp("fread", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 3:
            {
                /* compress */
                if (strcmp("fopen64", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            case 4:
            {
                /* compress */
                if (strcmp("fdopen", F->getName ().data()) == 0)
                {
                    return true;
                }
                break;
            }
            default:
            {
                break;                
            }
        }

        return false;
        
    }


    
    inline void InitSource ()
    {
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it)
        {
            Function *F = &*it;  
            if (F->isIntrinsic() || F->isDeclaration ())
            {
                continue;
            }

            if (!IsWhereSource (F))
            {
                continue;
            }

            for (inst_iterator ItI = inst_begin(*F), Ed = inst_end(*F); ItI != Ed; ++ItI) 
            {
                Function *Callee = NULL;
                Instruction *Inst = &*ItI;
                
                LLVMInst LI (Inst);
                if (!LI.IsCall (&Callee))
                {
                    continue;
                }

                if (Callee == NULL)
                {
                    continue;
                }

                errs()<<"from "<<F->getName ()<<" call "<<Callee->getName ()<<"\r\n";

                if (!IsSourceFunc (Callee))
                {
                    continue;
                }

                errs()<<"-------------->"<<Callee->getName ()<<"\r\n";

                
                if (m_Type >= 3)
                {
                    m_Source = Inst;
                    m_Criterion.insert (LI.GetDef ());
                }
                else
                {
                    for (auto It = LI.begin (); It != LI.end(); It++)
                    {
                        Value *U = *It;
                        if (U->hasName ())
                        {
                            errs ()<<"==> Add Source: "<<U->getName ()<<"\r\n";
                        }
                        else
                        {
                            errs ()<<"==> Add Source: "<<U<<"\r\n";
                        }

                        m_Source = Inst;
                        m_Criterion.insert(U);
                        break;                        
                    }
                }
            }
        }
    }

    inline void InitSink ()
    {
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it)
        {
            Function *F = &*it;  
            if (F->isIntrinsic() || F->isDeclaration ())
            {
                continue;
            }
        }
    }
    

    void VisitFunction ()
    {
        InitSource ();
  
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *F = &*it;  
            if (F->isIntrinsic() || F->isDeclaration ())
            {
                continue;
            }

            GetCriterion (F);

            //VisitBlock (F);
            errs() <<"Process "<<F->getName ()<<", m_Criterion = "<<m_Criterion.size()<<"\r\n";
            SymDependence SD (F, m_Criterion);

            for (auto it = SD.begin(); it != SD.end (); it++)
            {
                Instruction *Inst = *it;
                if (Inst->getOpcode() == Instruction::ICmp)
                {
                    continue;
                }
                AddTrackDefault (Inst);
            }
        }

        return;
    }
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
