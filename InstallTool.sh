
export SDI_PATH=`pwd`

cd $SDI_PATH/CComponent/SDA
cp build/bin/sda /usr/bin
if [ -d "~/.local/bin" ]; then
	cp build/bin/sda ~/.local/bin/
fi
cp $LLVM_PATH/build/lib/llvmSDIpass.so /usr/lib/

cd $SDI_PATH/DynAnalyzer
cp libDynA* /usr/lib/
cp difaEngine /usr/bin/
if [ -d "~/.local/bin" ]; then
	cp difaEngine ~/.local/bin/
fi
