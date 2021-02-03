//===- ModuleSet.h -- ModuleSet class-----------------------------------------//
//
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

#ifndef _MODULESET_H_
#define _MODULESET_H_

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "common/SoftPara.h"

#define MAX_MODULE_NUM (128)

typedef std::vector<llvm::Function*> T_FunVector;
typedef std::set<llvm::Function*> T_FunSet;

typedef std::map<const llvm::Function*, llvm::Function*> T_FunDeclToDefMap;
typedef std::map<const llvm::Function*, T_FunVector> T_FunDefToDeclsMap;
typedef std::map<std::string, llvm::Function*> T_NameToFunDefMap;
typedef std::map<std::string, std::set<llvm::Function*>>T_NameToFunDeclsMap;

typedef T_FunVector::iterator func_iterator;
typedef std::vector<llvm::GlobalVariable*>::iterator  global_iterator;

typedef std::set<std::string> T_StringSet;

class GlobalSet
{
public:
    GlobalSet ()
    {
    }
    
    global_iterator begin ()
    {
        return m_GlbSet.begin();
    }
    
    global_iterator end ()
    {
        return m_GlbSet.end();
    }

    VOID AddGlobalVal (llvm::GlobalVariable* Val)
    {
        m_GlbSet.push_back (Val);
    }

private:
    std::vector<llvm::GlobalVariable*> m_GlbSet;
        
};


class FunctionSet
{
private:
    T_FunVector m_FuncVector;
    T_FunSet m_FunDecls, m_FunDefs;
    T_FunDeclToDefMap  m_FunDeclToDefMap;
    T_FunDefToDeclsMap m_FunDefToDeclsMap;

private:

    VOID GetFuncNameSet(T_StringSet &NameSet, T_FunSet& FuncSet);
    VOID GetDefAndDeclFunc (T_StringSet& DefAndDeclNameSet);
    
    VOID BuildFuncDeclToDefMap (T_StringSet &DefAndDeclNameSet);
    VOID BuildFuncDefToDeclMap (T_StringSet &DefAndDeclNameSet);

public:
    FunctionSet () { }
    VOID AddFunction (llvm::Function* Func);
    VOID BuildFuncMap ();

    llvm::Function *GetDefinition(const llvm::Function *Func) const 
    { 
        T_FunDeclToDefMap::const_iterator it = m_FunDeclToDefMap.find(Func);
        if (it == m_FunDeclToDefMap.end())
        {
            return NULL;
        }
 
        return it->second;
    }

    const T_FunVector& GetDeclaration(const llvm::Function *fun) const 
    {
        T_FunDefToDeclsMap::const_iterator it = m_FunDefToDeclsMap.find(fun);
        assert(it != m_FunDefToDeclsMap.end() && "has no declaration?");
        
        return it->second;
    }

    func_iterator begin() 
    {
        return m_FuncVector.begin();
    }

    func_iterator end()
    {
        return m_FuncVector.end();
    }
};

class ModuleSet
{
private:
    
    DWORD m_ModuleNum;
    llvm::LLVMContext *m_LlvmCtx;
    std::unique_ptr<llvm::Module> *m_Modules;
    std::map<DWORD, std::string> m_IdToName;
    std::map<std::string, std::string> m_FuncToModule;
    std::vector<std::string> m_ModulePathVec;

    FunctionSet m_FuncSet;
    GlobalSet   m_GlobalSet;

    llvm::Function *m_EntryFunc;

private:

    VOID InitModuleSet ();

    VOID loadModules(const std::vector<std::string> &modulePathVec);
    VOID loadOneModule(std::string &ModulePath, DWORD Id);

    VOID InitFmMap();

    VOID LoadModuleOnDemand (std::string FuncName);
    std::string GetModulePathByFname(std::string FuncName);
    BOOL IsModuleExist(std::string ModulePath);

    inline bool IsEntryFunction (const llvm::Function * Func);

public:
    
    inline DWORD GetModuleNum() const 
    {
        return m_ModuleNum;
    }

    llvm::Module* GetModule(DWORD Id) const 
    {
        assert(Id < m_ModuleNum && "Out of range.");
        
        return m_Modules[Id].get();
    }

    llvm::Function* GetDefinition(const llvm::Function *fun) const 
    {        
        return m_FuncSet.GetDefinition(fun);
    }

    const T_FunVector& GetDeclaration(const llvm::Function *fun) const 
    {
        return m_FuncSet.GetDeclaration(fun);
    }

    func_iterator func_begin() 
    {
        return m_FuncSet.begin();
    }

    func_iterator func_end()
    {
        return m_FuncSet.end();
    }

    global_iterator global_begin() 
    {
        return m_GlobalSet.begin();
    }

    global_iterator global_end()
    {
        return m_GlobalSet.end();
    }

    ModuleSet(const std::vector<std::string> &ModulePathVec);   
    ModuleSet(llvm::Module &Nodule);
    
    ModuleSet() 
    {
        m_ModuleNum = 0;
        m_LlvmCtx   = NULL;
        m_Modules   = NULL;
        m_EntryFunc = NULL;
    }

    ~ModuleSet() 
    {
        m_ModuleNum = 0;
        if (m_LlvmCtx != NULL)
        {
            delete m_LlvmCtx;
            m_LlvmCtx = NULL;
        }

        if (m_Modules != NULL)
        {
            delete m_Modules;
            m_Modules = NULL;
        }     
    }

    VOID PreProcess ();

    inline llvm::Function* GetEntryFunction ()
    {
        return m_EntryFunc;
    }

};

class ModuleManage 
{

private:
    static ModuleSet *m_ModuleSet;

public:
 
    ModuleManage(const std::vector<std::string> &ModulePathVec) 
    {
        if (m_ModuleSet == NULL)
        {
            m_ModuleSet = new ModuleSet(ModulePathVec);
        }
    }

    ModuleManage(llvm::Module &Mod) 
    {
        if (m_ModuleSet == NULL)
        {
            m_ModuleSet = new ModuleSet(Mod);
        }
    }

    ModuleManage()
    {
        if (m_ModuleSet == NULL)
        {
            m_ModuleSet = new ModuleSet();
        }
    }

    static void DelModuleSet() 
    {
        if (m_ModuleSet != NULL)
        {
            delete m_ModuleSet;
            m_ModuleSet = NULL;
        }
    }

    DWORD GetModuleNum() const 
    {
        return m_ModuleSet->GetModuleNum();
    }

    llvm::Module* GetModule(DWORD Id) const 
    {
        return m_ModuleSet->GetModule(Id);
    }

    llvm::Function* GetDefinition(const llvm::Function *fun) const 
    {
        return m_ModuleSet->GetDefinition(fun);
    }

    const T_FunVector& GetDeclaration(const llvm::Function *fun) const 
    {
        return m_ModuleSet->GetDeclaration(fun);
    }

    func_iterator func_begin() 
    {
        return m_ModuleSet->func_begin();
    }
 
    func_iterator func_end()
    {
        return m_ModuleSet->func_end();
    }

    global_iterator global_begin() 
    {
        return m_ModuleSet->global_begin();
    }
 
    global_iterator global_end()
    {
        return m_ModuleSet->global_end();
    }

    inline llvm::Function* GetFunction(llvm::StringRef FunName) const 
    {
        llvm::Function* Func;
        DWORD MdNum = GetModuleNum();
        
        for (DWORD Id = 0; Id  < MdNum; ++Id ) 
        {
            llvm::Module *mod = m_ModuleSet->GetModule(Id);
            
            Func = mod->getFunction(FunName);
            if(Func && !Func->isDeclaration()) 
            {
                return Func;
            }
        }
        
        return NULL;
    }

    inline llvm::Function* GetEntryFunction ()
    {
        assert(m_ModuleSet != NULL);
        return m_ModuleSet->GetEntryFunction ();
    }
};


#endif
