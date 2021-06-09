# PolyCruise: an extensible framework for dynamic data flow analysis across language.


# Introduction
PolyCruise is an extensible dynamic analysis framework targeting multilingual programs.  In the framework, we develop a novel approach called language-independent symbolic  dependence analysis (SDA) to guide the instrumentation of multilingual components. Moreover, the framework supports online dynamic information flow analysis across  language  boundaries and provides C plug-in support capabilities for vulnerability detection.

# Installation
## requiremtns
Our framework is tested on Ubuntu18.04, LLVM7.0 and Python3.7.
before build the framework, corresponding versions of LLVM and Python need to be installed.

## build the framework
cd PolyCruise && ./build.sh

# Usage
To evaluation our approach, we developed a micro-benchmark called ![PyCBench](https://github.com/Daybreak2019/LDI/tree/master/PyCBench).
To test PolyCruise on all the micro-benchmarks, please execute the following commands:
cd PyCBench && ./RunTest.sh



        

