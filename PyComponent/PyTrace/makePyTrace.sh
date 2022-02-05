if [ -d build ] 
then
	rm -rf build
fi

cp $POLYC_PATH/DynAnalyzer/include ./ -rf
cp $POLYC_PATH/DynAnalyzer/libDynAnalyze.so /usr/lib/

python setup.py build

pip3 install . --upgrade
