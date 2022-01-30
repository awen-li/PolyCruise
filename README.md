# PolyCruise: A Cross-Language Dynamic Information Flow Analysis.


# Introduction
We present PolyCruise, a framework that enables holistic dynamic information flow analysis (DIFA) across heterogeneous languages hence security applications empowered by DIFA (e.g., vulnerability discovery) for multilingual software. PolyCruise combines a light language-specific analysis that computes symbolic dependencies in each language unit with a language-agnostic online data flow analysis guided by those dependencies, in a way that overcomes language heterogeneity.

# Installation
## 1. requiremtns
#### 1.1 Setup the environment manually
PolyCruise is tested on Ubuntu18.04, LLVM7.0 and Python3.7 (and Python3-dev).
An avaiable package to install LLVM7.0 with support of gold plugin can be found [here](https://github.com/Daybreak2019/PCA/tree/master/llvm7).

#### 1.2 Reuse the environment from docker image
We build a [docker image](https://hub.docker.com/repository/docker/daybreak2019/polycruise/tags?page=1&ordering=last_updated) with all dependences ready.
Please use the command ```docker pull daybreak2019/polycruise:1.0``` to pull the image to local storage.

## 2. build PolyCruise
After cloning the code from GitHub, using the following command to build the whole project.

```cd PolyCruise && ./build.sh```

## 3. Usage
#### 3.1 Steps of applying PolyCruise on Python programs with C bindings.
- S1: Rewrite python modules to SSA forms.
Use pyinspect with '-c' (compile) and '-d' (destination) for SSA translation:
```
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
Then build the whole project again for static instrumentation:
```
python setup-instrm.py install
```

- S4: Run the cases of the target:
```
difaEngine &
python -m pyinspect -C <your-criterion.xml> -t <your-case> &
```

An example to cover all the steps above can be found [here](https://github.com/Daybreak2019/PolyCruise/blob/master/PyCBench/DynamicInvocation/1_leak_PyClang/build.sh).

#### 3.2 Run PolyCruise on PyCBench
To evaluation our approach, we developed a micro-benchmark called ![PyCBench](https://github.com/Daybreak2019/LDI/tree/master/PyCBench).

To test PolyCruise on all the micro-benchmarks, please execute the following commands:
```
cd PolyCruise/PyCBench && ./RunTest.sh
```

#### 3.3 Use PolyCruise on Real-world Python-C programs


        

