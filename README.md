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
#### 3.1 Use PolyCruise on PyCBench
To evaluation our approach, we developed a micro-benchmark called ![PyCBench](https://github.com/Daybreak2019/LDI/tree/master/PyCBench).

To test PolyCruise on all the micro-benchmarks, please execute the following commands:
```
cd PolyCruise/PyCBench && ./RunTest.sh
```

#### 3.2 Use PolyCruise on Real-world Python-C programs


        

