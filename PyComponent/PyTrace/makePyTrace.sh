if [ -d build ] 
then
	rm -rf build
fi

cp $SDI_PATH/DynAnalyzer/include ./ -rf
cp $SDI_PATH/DynAnalyzer/libDynAnalyze.so /usr/lib/

python setup.py build

pip3 install .
