
#1. build LDA
cd LDA
./build.sh
cp build/bin/ldi /usr/bin
cd -

#2. build DynA
cd DynA
make -f makefile.so clean && make -f makefile.so
cp libDynA* /usr/lib/
cd -

#3. build LDIpass
cd LDIpass
LDI=`pwd`
LDIpass=$LLVM_PATH/llvm-7.0.0.src/lib/Transforms/LDIpass
if [ ! -d $LDIpass ]
then
cd $LLVM_PATH/llvm-7.0.0.src/lib/Transforms/
ln -s $LDI
echo "add_subdirectory(LDIpass)" >> CMakeLists.txt
fi
cd $LLVM_PATH/build
make
cp $LLVM_PATH/build/lib/llvmLDIpass.so /usr/lib/
cd $LDI && cd ..

#4. buld PyTrace
cd PyTrace
./makePyTrace.sh
cd -

#5. install PyInspect module
pip3 install .

#6. install pyinspect tool
PyVersion=`python -c 'import platform; major, minor, patch = platform.python_version_tuple(); print(str(major)+"."+str(minor))'`
PYTHON_PATH=/usr/local/lib/python$PyVersion/
cp Script/pyinspect.py $PYTHON_PATH
