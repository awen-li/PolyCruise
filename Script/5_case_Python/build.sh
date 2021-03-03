
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


# 2. translate the python code
rm -rf /tmp/difg/LdaBin*

echo "@@@@@@@@@ Python component.........."
CASE=`basename $CASE_PATH`
cd ..
python -m pyinspect -E $CASE/ExpList -c -d $CASE

# 3. run the case
echo "@@@@@@@@@ runing the case.........."
export CASE1="8"
cd Temp/$CASE/
python -m pyinspect -C criterion.xml -t Python/Demo.py

