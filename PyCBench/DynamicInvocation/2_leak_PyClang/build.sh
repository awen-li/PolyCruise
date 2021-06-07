
export CASE_PATH=`pwd`

DelShareMem ()
{
	ShareId=`ipcs -m | grep 0xc3b3c5d0 | awk '{print $2}'`
	if [ -n "$ShareId" ]; then
		ipcrm -m $ShareId
	fi	
}

DelShareMem
difaEngine -c CaseResults &

# 1. install C module of the case
rm -rf /tmp/difg/LdaBin*

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
export CASE1="hello"
cd Temp/$CASE/
cp CPython/build/lib.linux-x86_64-3.7/DemoTrace.cpython-37m-x86_64-linux-gnu.so Python/PyDemo.so
python -m pyinspect -C criterion.xml -i Python -t Python/Demo.py

