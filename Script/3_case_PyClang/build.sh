
export CASE_PATH=`pwd`

# 1. install C module of the case
echo "@@@@@@@@@ C component.........."
cd $CASE_PATH/CPython
rm -rf build
python setup-lda.py build
sda -file build/lib.linux-x86_64-3.7/DemoTrace.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc -criterion ../criterion.xml
rm -rf build
python setup-instm.py build
cd $CASE_PATH

# 2. translate the python code
echo "@@@@@@@@@ Python component.........."
CASE=`basename $CASE_PATH`
cd ..
python -m pyinspect -E $CASE/ExpList -c -d $CASE

# 3. run the case
echo "@@@@@@@@@ runing the case.........."
export CASE1="8"
cd Temp/$CASE/
cp CPython/build/lib.linux-x86_64-3.7/DemoTrace.cpython-37m-x86_64-linux-gnu.so Python/PyDemo.so
python -m pyinspect -C criterion.xml -t Python/Demo.py

