# PCA: Memory Leak Detection using Partial Call-Path Analysis.


# Introduction
![PCA](https://github.com/Daybreak2019/PCA/blob/master/image/intro.PNG)
***
Essentially, *PCA* is a well implemented data-dependence analyzer targeting c program, in which we applied our optimization such as 
partial call-path analysis and integer encoding in reaching-definition computation to improve efficiency and effectiveness.
*PCA-Mem* is a case study for evaluation, our tool also supports many other data-dependence based applications such as taint analysis, bug detection and so on.

# Installation
Here we present two ways to fetch and use *PCA*:

#### Use *PCA* through Virtual Machine
1. Download PCA-VM.zip (the virtual disk image) through the [link](https://drive.google.com/file/d/12eMHiYnqYPwjgpd6BjKtmmiT73y9lGC4/view?usp=sharing).
2. Launch the virtual machine in Virtual Box(username/password: pca/pca, root/pca).
3. Open a terminal, and entry the directory /home/pca/PCA.

#### Use *PCA* through source code compilation.
1. Check prerequisites (UNIX: Ubuntu 16.04 LTS or Ubuntu 18.04 LTS).
2. Download source code through the [link](https://github.com/Daybreak2019/PCA).
3. Enter directory PCA/llvm7 and run installLLVM.sh, which will install LLVM7 and configure environment variables automatically.
4. Enter directory PCA and build PCA with script build.sh.

# Usage
We implemented a case study of *PCA: PCA-Mem*. This part shows how to run *PCA-Mem* against target program for memory leak detection.

#### Compile target program
To enable data-dependence analysis based on LLVM, target program needs to be compiled with clang and gold-plugin (details referred to [here](https://llvm.org/docs/GoldPlugin.html). 
1. For the simple subject used in the demo, the command line forthis step is:
- clang -flto leak.c -c -o leak.bc

2. For the Slurm system subject used in the demo, the commandline for this step is:
- Specify environment variables for the compiler:
  - export CC="clang -flto"
  - export CXX="clang++ -flto"
  - export RANLIB=/bin/true
- Compile Slurm: 
  - ./configure && make

#### Run *PCA-Mem* against simple program
In this step, we present how to run memory leak detection with PCA-Mem and generate the
data-dependence graph (DDG) (for visual understanding purposes).

![case1](https://github.com/Daybreak2019/PCA/blob/master/image/case1.PNG)
***

A small test case (leak.bc) is shown as Figure above, there are two partial free defects obviously, which are detected by *PCA-Mem* (with command: PCA-Mem -file leak.bc) as expected (shown as Figure below).

![case1](https://github.com/Daybreak2019/PCA/blob/master/image/case1_res.PNG)
***

We present a parameter (--dump-DDG) to generate DDG.dot, which can be open by [GVEdit](https://graphviz.org/download/) as below (ICFG is painted with black color while DDG with red), in this case we could verify the correctness of DDG manually while debugging.

![case1](https://github.com/Daybreak2019/PCA/blob/master/image/case1_DDG.png)

#### Run *PCA-Mem* against [Slurm](https://slurm.schedmd.com/download.html)
For large scale program like Slurm (Version 15.08.7), which usually contains multiple modules, *PCA-Mem* conducts two-step analysis:
1. Pre-process: Compute dependencies between modules (e.g., PCA-Mem -dir Slurm -pre=1). 
2. Program analysis: Link all necessary IR of modules for the target executable and Perform data dependence analysis and memory leak detection sequentially (e.g., PCA-Mem -file Slurm/salloc.bc). 

        

