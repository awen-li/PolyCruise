
Wait ()
{
	second=1
	process=$1
	while true
	do
		count=`ps -ef | grep $process | grep -v "grep" | wc -l`
		if [ 0 == $count ];then
			break
		else
			sleep 1
		fi
		
		let second++
		if [ $second == 300 ]; then
			ps -ef | grep difaEngine | awk '{print $2}' | xargs kill -9
			ps -ef | grep python | awk '{print $2}' | xargs kill -9
			break
		fi	
	done
	sleep 1
}

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

GenMap ()
{
    SCRIPTS=$1
    CASE_PATH=$2
    target=$3
    INSTALL_PATH=$4
    
    echo $INSTALL_PATH
    pyMap=$SCRIPTS/Pymap.ini
    if [ -f "$pyMap" ]; then
    	cp $pyMap $CASE_PATH
    else
        echo "...................start generating Pymap.ini ............................."
    	if [ ! -n "$INSTALL_PATH" ]; then
    		INSTALL_PATH=`find /usr/lib/anaconda3/lib/python3.7/ -name $target`
    		if [ ! -n "$INSTALL_PATH" ]; then
    			echo "!!!!!!!!INSTALL_PATH of $target is NULL, need to specify a install path............."
    			exit 0
    		fi
    	fi
        find $INSTALL_PATH -name "*.py" > "$CASE_PATH/$target.ini"
        
        cd $CASE_PATH
        python -m pyinspect -M "$target.ini" pyList
        cd -
        
        cp $CASE_PATH/Pymap.ini $pyMap    
    fi

    return
}

target=japronto
Action=$1

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

if [ "$Action" == "build" ]; then
	rm -rf $CASE_PATH
	if [ ! -f "$target/function_def.pkl" ]; then
		python -m pyinspect -g $target
		cp -f function_def.pkl $target/
		cp -f $target"_gen_criterion.xml" $target/gen_criterion.xml
	fi
	python -m pyinspect -c -E $SCRIPTS/ExpList -d $target
fi

# 2. build and SDA
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-lda.py build
	SdaAnalysis
fi

# 3. build again and install the instrumented software
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-instm.py install
	
	GenMap $SCRIPTS $CASE_PATH $target $CASE_PATH
fi

# 5. run the cases
#integration_tests/test_drain.py integration_tests/test_noleak.py integration_tests/test_perror.py integration_tests/test_reaper.py integration_tests/test_request.py\
TestCase=(examples/1_hello/hello.py examples/2_async/async.py examples/3_router/router.py examples/4_request/request.py examples/8_template/template.py examples/7_extend/extend.py\
          examples/6_exceptions/exceptions.py)
CaseNum=${#TestCase[*]}
Index=1
for Case in ${TestCase[@]}
do
	echo "[$Index/$CaseNum]======================= Execute the case $Case ======================="
	DelShareMem
	difaEngine &
	StartTime=`date '+%s'`
	
	python -m pyinspect -C ../../criterion.xml -t $Case &
	
	Wait difaEngine
        EndTime=`date '+%s'`
        TimeCost=`expr $EndTime - $StartTime`
        echo "[$Index]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
done
