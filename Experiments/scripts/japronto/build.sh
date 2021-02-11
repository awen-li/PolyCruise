

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./build -pre=1
	BC_FILES=`find ./build -name *.preopt.bc`
	for bc in $BC_FILES
	do
		sda -file $bc -criterion ../../criterion.xml
	done
}

DelShareMem ()
{
	ShareId=`ipcs -m | grep 0xc3b3c5d0 | awk '{print $2}'`
	if [ -n "$ShareId" ]; then
		ipcrm -m $ShareId
	fi	
}

target=japronto

DelShareMem
difaEngine &

# 1. build and translate python modules
cd ../../
CASE_PATH=Temp/$target
SCRIPTS=scripts/$target

python -m pyinspect -c -E $SCRIPTS/ExpList -d $target

# 2. build and instrument C modules
pwd

cp criterion.xml $CASE_PATH/
cp $SCRIPTS/setup-*.py $CASE_PATH/
cd $CASE_PATH
rm -rf build
python setup-lda.py build
SdaAnalysis

# 3. build again and install the instrumented software
rm -rf build
python setup-instm.py install
cp misc integration_tests/ -rf


# 4. run the cases
python -m pyinspect -C ../../criterion.xml -t integration_tests/drain.py 






