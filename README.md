# PolyCruise: an extensible framework for dynamic data flow analysis across language.


# Introduction
PolyCruise is an extensible dynamic analysis framework targeting multilingual programs.  In the framework, we develop a novel approach called language-independent symbolic  dependence analysis (SDA) to guide the instrumentation of multilingual components. Moreover, the framework supports online dynamic information flow analysis across  language  boundaries and provides C plug-in support capabilities for vulnerability detection.

# Installation
## requiremtns
Our framework is tested on Ubuntu18.04, LLVM7.0 and Python3.6.
before build the framework, corresponding versions of LLVM and Python need to be installed.

## build the framework
cd PolyCruise && ./build.sh

# Usage
To evaluation our approach, we implement cross-language analysis for Python-C programs.
This is an example to show how to analyze a Python-C program step by step.
![example](https://github.com/Daybreak2019/LDI/blob/master/Script/1_case_PyClang/build.sh)



        

