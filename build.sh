
EnvVerify ()
{
if [ $LLVM_PATH == "" ]; then
 echo "please set ENV variable LLVM_PATH"
 exit
fi

if [ $CLANG_PATH == "" ]; then
 echo "please set ENV variable CLANG_PATH"
 exit
fi

echo "Environment is checking ok!"
}

EnvVerify
export POLYC_PATH=`pwd`

#0. temp data directory
export DATA_DIR="/tmp/difg"
mkdir -p $DATA_DIR

#1. build CComponent
echo ""
echo "@@@@@@@@@@@@@@@ build CComponent:SDA @@@@@@@@@@@@@@@"
cd $POLYC_PATH/CComponent/SDA
./build.sh
cp build/bin/sda /usr/bin
if [ -d "~/.local/bin" ]; then
	cp build/bin/sda ~/.local/bin/
fi


#2. build LDIpass
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build CComponent:SDIpass @@@@@@@@@@@@@@@"
SDI=$POLYC_PATH/CComponent/SDIpass
SDIpass=$LLVM_PATH/llvm-7.0.0.src/lib/Transforms/SDIpass
if [ ! -d $SDIpass ]
then
cd $LLVM_PATH/llvm-7.0.0.src/lib/Transforms/
ln -s $SDI SDIpass
echo "add_subdirectory(SDIpass)" >> CMakeLists.txt
fi
cd $LLVM_PATH/build
make
cp $LLVM_PATH/build/lib/llvmSDIpass.so /usr/lib/


#3. build DynA
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build DynAnalyzer @@@@@@@@@@@@@@@"
cd $POLYC_PATH/DynAnalyzer
make -f makefile.so clean && make -f makefile.so CC=clang
make -f makefile.so clean && make -f makefile.so CC=clang++
cp libDynA* /usr/lib/
make clean && make
killall difaEngine
cp difaEngine /usr/bin/
if [ -d "~/.local/bin" ]; then
	cp difaEngine ~/.local/bin/
fi

cd $POLYC_PATH
cp Experiments/function_sds.xml $DATA_DIR/

#4. buld PyComponent:PyTrace
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build PyComponent:PyTrace @@@@@@@@@@@@@@@"
cd $POLYC_PATH/PyComponent/PyTrace
./makePyTrace.sh


#5. install PyComponent:PyInspect module
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ build PyComponent:PyInspect @@@@@@@@@@@@@@@"
cd $POLYC_PATH/PyComponent
pip3 install . --upgrade

#6. install pyinspect tool
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ Install pyinspect tool @@@@@@@@@@@@@@@"
PyVersion=`python -c 'import platform; major, minor, patch = platform.python_version_tuple(); print(str(major)+"."+str(minor))'`
PYTHON_PATH=/usr/lib/python$PyVersion
if [ -d "$PYTHON_PATH" ]; then
    cp $POLYC_PATH/PyComponent/pyinspect.py $PYTHON_PATH
fi
# anaconda environment
Anaconda=`which anaconda`
if [ -n "$Anaconda" ]; then
    PYTHON_PATH=/usr/lib/anaconda3/lib/python$PyVersion
    if [ -d "$PYTHON_PATH" ]; then
    	cp $POLYC_PATH/PyComponent/pyinspect.py $PYTHON_PATH/
    fi
fi

#Python sys.path set
PyPath=`python -c 'import sys; print(sys.path)'`
export PYTHONPATH=$PYTHON_PATH/site-packages:$PYTHONPATH
echo "@@@@ Original PY_path is $PyPath  ----> $PYTHONPATH" 


 
#7. install plugins
echo ""
echo ""
echo "@@@@@@@@@@@@@@@ Install vPlugins @@@@@@@@@@@@@@@"
vPlugins=$POLYC_PATH/vPlugins
cd $vPlugins
make clean && make
cp plugins.ini $DATA_DIR/
plugins=$(ls -l | awk '/^d/ {print $NF}')
for pdir in $plugins; 
do
	cd $vPlugins/$pdir
	cp $vPlugins/$pdir/*.so $DATA_DIR/
	cp $vPlugins/$pdir/*.sink $DATA_DIR/
done
