Action="None"
if [ $# != 0 ] 
then
Action=$1
fi

echo $Action
if [ "$Action" == "lda" ]
then
export ENV_LDFLAGS="-flto"
export ENV_CFLAGS="-emit-llvm"
else
export ENV_LDFLAGS="/usr/lib/libDynAnalyze.so"
export ENV_CFLAGS="-Xclang -load -Xclang llvmLDIpass.so"
fi

cd C
make clean && make
cd -

cd CPython
rm -rf build
python setup.py build
cd -

if [ "$Action" == "lda" ]
then
ldi -dir ../ -pre=1
ldi -file CPython/PyDemo.so.0.0.preopt.bc
cp LdaBin.bin /tmp/
else
cp CPython/build/lib.linux-x86_64-3.6/Trace.cpython-36m-x86_64-linux-gnu.so Python/PyDemo.so
fi
