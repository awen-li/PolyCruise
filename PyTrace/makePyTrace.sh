if [ -d build ] 
then
	rm -rf build
fi

cp ../DynA/include ./ -rf
cp ../DynA/libDynAnalyze.so /usr/lib/

python ./setup.py build

cp build/lib.linux-x86_64-3.6/PyTrace.cpython-36m-x86_64-linux-gnu.so ../PyInspect/PyTrace.so
