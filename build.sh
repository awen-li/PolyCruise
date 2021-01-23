
export LDI_PATH=`pwd`

#1. build CComponent
echo ""
echo "@@@@@@@@@@@@@@@ build CComponent:LDA @@@@@@@@@@@@@@@"
cd $LDI_PATH/CComponent/LDA
./build.sh
cp build/bin/ldi /usr/bin


#2. build LDIpass
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build CComponent:LDIpass @@@@@@@@@@@@@@@"
cd $LDI_PATH/CComponent/LDIpass
LDIpass=$LLVM_PATH/llvm-7.0.0.src/lib/Transforms/LDIpass
if [ ! -d $LDIpass ]
then
cd $LLVM_PATH/llvm-7.0.0.src/lib/Transforms/
ln -s $LDI LDIpass
echo "add_subdirectory(LDIpass)" >> CMakeLists.txt
fi
cd $LLVM_PATH/build
make
cp $LLVM_PATH/build/lib/llvmLDIpass.so /usr/lib/

#3. build DynA
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build DynAnalyzer @@@@@@@@@@@@@@@"
cd $LDI_PATH/DynAnalyzer
make -f makefile.so clean && make -f makefile.so
cp libDynA* /usr/lib/

#4. buld PyComponent:PyTrace
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build PyComponent:PyTrace @@@@@@@@@@@@@@@"
cd $LDI_PATH/PyComponent/PyTrace
./makePyTrace.sh


#5. install PyComponent:PyInspect module
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build PyComponent:PyInspect @@@@@@@@@@@@@@@"
cd $LDI_PATH/PyComponent
pip3 install .

#6. install pyinspect tool
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ Install pyinspect tool @@@@@@@@@@@@@@@"
PyVersion=`python -c 'import platform; major, minor, patch = platform.python_version_tuple(); print(str(major)+"."+str(minor))'`
PYTHON_PATH=/usr/local/lib/python$PyVersion/
cp $LDI_PATH/Script/pyinspect.py $PYTHON_PATH
