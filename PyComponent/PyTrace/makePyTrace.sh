if [ -d build ] 
then
	rm -rf build
fi

cp $LDI_PATH/DynAnalyzer/include ./ -rf
cp $LDI_PATH/DynAnalyzer/libDynAnalyze.so /usr/lib/

python setup.py build

pip3 install .
