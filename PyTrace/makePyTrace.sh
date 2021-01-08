if [ -d build ] 
then
	rm -rf build
fi

cp ../DynA/include ./ -rf
cp ../DynA/libDynAnalyze.so /usr/lib/

python ./setup.py build

pip3 install .
