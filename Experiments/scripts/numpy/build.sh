
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
		if [ $second == 200 ]; then
			ps -ef | grep difaEngine | awk '{print $2}' | xargs kill -9
			break
		fi	
	done
	sleep 1
}

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./ -pre=1
	BC_FILES=`find ./ -name *.preopt.bc`
	for bc in $BC_FILES
	do
		echo ".......................sda -file $bc......................."
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
    
    pyMap=$SCRIPTS/Pymap.ini
    if [ -f "$pyMap" ]; then
    	cp $pyMap $CASE_PATH
    else
        echo "...................start generating Pymap.ini ............................."
    	INSTALL_PATH=$PWD"/build/testenv/lib/python3.7/site-packages/numpy"
        find $INSTALL_PATH -name "*.py" > "$target.ini"
        python -m pyinspect -M "$target.ini" pyList	
    fi
    
    return
}

target=numpy
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
	fi
	python -m pyinspect -c -E $SCRIPTS/ExpList -d $target
fi

# 2. build and SDA
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
if [ "$Action" == "build" ]; then
	rm -rf build
	rm -rf /tmp/difg/LdaBin*
	python setup-lda.py build_ext --inplace

	SdaAnalysis
fi

# 3. build again and install the instrumented software
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-instm.py build_ext --inplace
fi

# 4. generate file maping
GenMap $SCRIPTS $CASE_PATH $target

# 5. run the cases
Analyze ()
{
	Index=1
	CaseList=`cat case_list.txt`
	for curcase in $CaseList
	do
		if [ $Index != $INDEX ]; then
			let Index++
			continue
		fi	
	    DelShareMem
	    difaEngine &
	    StartTime=`date '+%s'`
		echo "[$Index].......................run case $curcase......................."
		export case_name=$curcase
		python -m pyinspect -C ./gen_criterion.xml -t runtests.py -v -m full &
		
		Wait difaEngine
		EndTime=`date '+%s'`
		TimeCost=`expr $EndTime - $StartTime`
		echo "[$Index]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
		
		let Index++
		export INDEX=$Index
	done
}

Analyze

