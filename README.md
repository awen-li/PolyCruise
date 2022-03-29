# PolyCruise: A Cross-Language Dynamic Information Flow Analysis.


# Introduction
We present PolyCruise, a framework that enables holistic dynamic information flow analysis (DIFA) across heterogeneous languages hence security applications empowered by DIFA (e.g., vulnerability discovery) for multilingual software. PolyCruise combines a light language-specific analysis that computes symbolic dependencies in each language unit with a language-agnostic online data flow analysis guided by those dependencies, in a way that overcomes language heterogeneity.

# Installation
## 1. requirements
#### 1.1 Setup the environment manually
PolyCruise is tested on Ubuntu18.04, LLVM7.0 and Python3.7 (and Python3-dev).
An avaiable package to install LLVM7.0 with support of gold plugin can be found [here](https://github.com/Daybreak2019/PCA/tree/master/llvm7).

#### 1.2 Reuse the environment from docker image (strongly recommanded)
We build a [docker image](https://hub.docker.com/repository/docker/daybreak2019/polycruise/tags?page=1&ordering=last_updated) with all dependences ready (i.e., all the dependencies required for running PolyCruise itself; for subject systems, currently only the dependencies for one real-world subject Cvxopt are included).
Please use the command ```docker pull daybreak2019/polycruise:1.1``` to pull the image to local storage.

## 2. build PolyCruise
After cloning the code from GitHub, using the following command to build the whole project.

```cd PolyCruise && ./build.sh```

## 3. Usage
#### 3.1 Steps of applying PolyCruise on Python programs with C bindings.
- S1: Rewrite python modules to SSA forms and collect function definitions in Python.
Use pyinspect with '-c' (compile) and '-d' (destination) for SSA translation:
```
# gen all defs in the project
python -m pyinspect -g <project-dir>
# recompile and rewrite the project
python -m pyinspect -c -d <project-dir>
```

- S2: Execute SDA on C bindings.

S2-1: Generate LLVM-IR with clang. One way is to specify following environments to the 'setup.py' and saved as 'setup-sda.py':
```
os.environ["CC"]  = "clang -emit-llvm"
os.environ["CXX"] = "clang"
os.environ["LDSHARED"] = "clang -flto -shared"
```

S2-2: Build the whole project with 'setup-sda.py':
```
python setup-sda.py build
```

S2-3: Execute SDA.

With specified criterions, we run SDA on all BCs (LLVM-IR) using following commands:
```
sda -dir ./build -pre=1
BC_FILES=`find ./build -name *.preopt.bc`
for bc in $BC_FILES
do
    sda -file $bc -criterion <your-path>/criterion.xml
done
```

- S3: Instrument C bindings

Specify following environments to the 'setup.py' and saved as 'setup-instrm.py'
```
os.environ["CC"]  = "clang -emit-llvm -Xclang -load -Xclang llvmSDIpass.so"
os.environ["CXX"] = "clang -emit-llvm -Xclang -load -Xclang llvmSDIpass.so"
os.environ["LDSHARED"] = "clang -flto -pthread -shared -lDynAnalyze"
```
Then install the project with instrumentation:
```
python setup-instrm.py install

#after installation, we maintain a maping between the source to installing path
find <install-path> -name "*.py" > "<your-path>/<your-project>.ini"
python -m pyinspect -M <your-project>.ini <your-source-list>
```

- S4: Run the cases of the target:
```
difaEngine &
python -m pyinspect -C <your-criterion.xml> -t <your-case> &
```

#### 3.2 Run PolyCruise on PyCBench
To evaluation our approach, we developed a micro-benchmark called ![PyCBench](https://github.com/Daybreak2019/LDI/tree/master/PyCBench).

To test PolyCruise on all the micro-benchmarks, please execute the following commands:
```
cd PolyCruise/PyCBench && ./RunTest.sh
```

-[Example-information leakage](https://github.com/Daybreak2019/PolyCruise/tree/master/PyCBench/DynamicInvocation/1_leak_PyClang):
```
[OUTPUT]:
@@@@@@@@@@@@@@@@@@@[66][deleak]Reach sink,  EventId = 5 -- <Function:Getpasswd,  Inst:21> 
                [G (7FFFF7E700E3,0)] [P (E0AB40,0)] 
                 ---->case: deleak Getpasswd 21 
===> Add source [9:2]2540004000000007 -> 0x7ffff7f44cd0 
Infor:  show->pwdtesthello

@@@@@@@@@@@@@@@@@@@[66][deleak]Reach sink,  EventId = 5 -- <Function:Trace,  Inst:21> 
                [U (v8,0)] [U (New,0)] 
                 ---->case: deleak Trace 21 
----> __exit__................, TracedStmts =  535
@@@@@ Ready to exit, total memory: 1166724 (K)!
Run successful.....
entry CheckCases ... CaseResults
LoadCases -> deleak:Getpasswd:21
LoadCases -> deleak:Trace:21
@@@@CASE-TEST PASS -> deleak-Getpasswd:21
@@@@CASE-TEST PASS -> deleak-Trace:21
@@@@@ GenSsPath -> Souece[2], Sink[2]......
[1 ][deleak] Path: [F (Getpasswd,0)] [P (E0AB40,0)] 
                [C]PwdInfo -> 
                [C]Pass -> 
                [C]Getpasswd: [F (printf,0)] 
[2 ][deleak] Path: [F (Demo.__init__,0)] [A (value,0)] [U (v2,0)] 
                [PY]DemoTr -> 
                [PY]Demo.__init__ -> 
                [PY]Trace: [F (print,0)] 
```


#### 3.3 Run PolyCruise on Real-world programs
To run PolyCruise on a real-world program (e.g., [cvxopt](https://github.com/Daybreak2019/cvxopt)), we need to setup the environment (dependences solving) for it first, and this task can sometimes be tedious and time-consuming.

When all dependences are sovled, we can follow the steps in Section 3.1 to prepare a script to integrate all necessary commands.  
As an example, we provide a script of [cvxopt](https://github.com/Daybreak2019/PolyCruise/blob/master/Experiments/scripts/cvxopt/build.sh) for reference.
Then we can use the following command to run PolyCruise on cvxopt:
```
# the parameter "build" indicate the script to compile and instrument cvxopt before running the tests
cd PolyCruise/Experiments/scripts/cvxopt
./build.sh build
```

## 4. Vulnerabilities detected on real-world programs

PolyCruise enabled the discovery of the first batch of 8 cross-language CVEs: CVE-2021-33430, CVE-2021-34141, CVE-2021-41495, CVE-2021-41496, CVE-2021-41497, CVE-2021-41498, CVE-2021-41499, CVE-2021-41500.

Refer to the [list of discovered vulnerabilities and PoCs here](https://github.com/Daybreak2019/PolyCruise/tree/master/Experiments/PoC) for details.

        

