

#include <llvm-c/Core.h>  
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>	 
#include <llvm/Support/FileSystem.h>	 
#include <llvm/Bitcode/BitcodeWriterPass.h>  
#include <llvm/IR/LegacyPassManager.h>		 
#include <llvm/Support/Signals.h>	 
#include <llvm/IRReader/IRReader.h>	 
#include <llvm/Support/ToolOutputFile.h>  
#include <llvm/Support/PrettyStackTrace.h>  
#include <llvm/IR/LLVMContext.h>		 
#include <llvm/Support/SourceMgr.h>  
#include <llvm/Bitcode/BitcodeWriterPass.h>
#include <llvm/IR/DataLayout.h>
#include "common/Bitmap.h"
#include "common/SoftPara.h"
#include "common/Stat.h"
#include "common/VisitDir.h"
#include "Fts.h"
#include "Sda.h"


using namespace llvm;
using namespace std;


static cl::opt<std::string>
InputDirectory("dir", cl::desc("<input bitcode file directory>"), cl::value_desc("directory"));

static cl::opt<std::string>
InputFilename("file", cl::desc("<input bitcode file>"), cl::init("-"), cl::value_desc("filename"));

static cl::opt<std::string>
InputCriterion("criterion", cl::desc("<definition of criterion in xml >"), cl::init("-"), cl::value_desc("criterion"));

static cl::opt<std::string>
PreProcess("pre", cl::desc("<preprocess before analysis >"), cl::init("0"), cl::value_desc("switch"));

static cl::opt<std::string>
GuardAll("guard", cl::desc("<try to compute SDA to all APIs in the target>"), cl::init("1"), cl::value_desc("switch"));


void GetLibEntry (ModuleManage *Mm, set <Function*> *Entry);
void GetAPIEntry (ModuleManage *Mm, set <Function*> *Entry, set <string> *EntryAPIs);
VOID LoadCriterion (char *XmlDoc, ModuleManage *Mm, set <Source*> *SS, set <string> *EntryAPIs);
VOID LoadFuncSds (char *XmlDoc /* /tmp/difg/function_sds.xml */);


VOID GetModulePath (vector<string> &ModulePathVec)
{   
    if (InputFilename == "" || InputDirectory == "-")
    {
        errs() << "InputFilename is NULL \n ";
        return;
    }

    if (InputFilename != "-" && InputFilename != "")
    {
        std::string ModuleName = InputFilename;
        ModulePathVec.push_back (ModuleName);
    }
    else
    {  
        std::string ModuleDir = InputDirectory;       
        ModulePath ModulePt;        
        ModulePt.Visit(ModuleDir.c_str(), &ModulePathVec);
    }

    return;
}

string GetCaseName (string FileName)
{   
    INT Pos = FileName.find(".bc");
    if(Pos == -1)
    {
        return NULL;
    }

    return FileName.substr(0, Pos);
}

VOID Preprocess (vector<string> &ModulePathVec)
{
    if (llaf::GetParaValue (PARA_PREPROFESS) != "1")
    {
        return;
    }

    Stat::StartTime ("LoadModule");
    ModuleManage ModuleMng (ModulePathVec);
    Stat::EndTime ("LoadModules");

    exit (0);
}

VOID RunPasses (vector<string> &ModulePathVec)
{
    Stat::StartTime ("LoadModule");
    ModuleManage ModuleMng (ModulePathVec);
    Stat::EndTime ("LoadModules");

    StField Sf;
    Sf.AddStFields ("struct.bz_stream", 0);
    Sf.AddStFields ("struct.bz_stream", 8);
    Sf.AddStFields ("struct.EState", 0);

    set <Source *> SS;
    set <string> EntryAPIs;
    if (InputCriterion != "")
    {
        std::string Criterion = InputCriterion;
        LoadCriterion ((char*)Criterion.c_str(), &ModuleMng, &SS, &EntryAPIs);
    }

    Sda sda (&ModuleMng, &SS, &Sf);
    set <Function*> Entry;
    if (GuardAll == "1")
    {
        GetLibEntry (&ModuleMng, &Entry);
    }
    else
    {
        GetAPIEntry (&ModuleMng, &Entry, &EntryAPIs);
        sda.SetGuard(0);
    }

    for (auto It = Entry.begin (); It != Entry.end (); It++)
    {
        sda.AddEntry (*It);
    }

    printf ("Start to compute SDA.........\r\n");
    sda.Compute ();
    //printf("Total Memory usage:%u (K)\r\n", Stat::GetPhyMemUse ());

    return;
}

VOID GetParas(int argc, char ** argv)
{ 
    cl::ParseCommandLineOptions(argc, argv, "call graph analysis\n");

    if (PreProcess != "")
    {
        std::string Para  = PARA_PREPROFESS;
        std::string Value = PreProcess;
        llaf::SetParaValue (Para, Value);    
    }

    return;
}

int main(int argc, char ** argv) 
{ 
    vector<string> ModulePathVec;
    Stat st;

    PassRegistry &Registry = *PassRegistry::getPassRegistry();

    initializeCore(Registry);
    initializeScalarOpts(Registry);
    initializeIPO(Registry);
    initializeAnalysis(Registry);
    initializeTransformUtils(Registry);
    initializeInstCombine(Registry);
    initializeInstrumentation(Registry);
    initializeTarget(Registry);

    LoadFuncSds ((char *)"/tmp/difg/function_sds.xml");
    GetParas(argc, argv);
  
    GetModulePath(ModulePathVec);
    if (ModulePathVec.size() == 0)
    {
        errs()<<"get none module paths!!\n";
        return 0;
    }

    Preprocess (ModulePathVec);

    Stat::StartTime("SDA");
    RunPasses (ModulePathVec);
    Stat::EndTime("SDA");
    
    return 0;
}

