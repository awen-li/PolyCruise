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
		if [ $second == 120 ]; then
			ps -ef | grep difaEngine | awk '{print $2}' | xargs kill -9
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

GenAllTestCases ()
{
	Target=$1
	CaseDir=$2
	
	cd $Target
	export case_dump=case_list.txt
	python -m unittest discover -s $1
	unset case_dump
	cd -
}

GenOneTestCases ()
{
	Target=$1
	Case=$2
	
	cd $Target
	rm -rf case_list.txt
	export case_dump=case_list.txt
	python -m unittest $Case
	unset case_dump
	cd -
}

target=cvxopt
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
	
	GenMap $SCRIPTS $CASE_PATH $target $CASE_PATH
fi

# 2. build and SDA
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-sda.py build
	SdaAnalysis
fi

# 3. build again and install the instrumented software
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-instm.py install
fi

# 5. run the cases
Analyze ()
{
	Index=1
	if [ ! -n "$INDEX" ]; then
		export INDEX=$Index
	fi
	
	CaseDir=tests
	ALL_TESTS=`find $CaseDir -name "*.py"`
	
	for Case in $ALL_TESTS
	do
		if [ $Index != $INDEX ]; then
			let Index++
			continue
		fi
		
		echo "[$Index].......................run case $Case......................."
		
		GenOneTestCases $CASE_PATH $Case
		CaseList=`cat case_list.txt`

		for curcase in $CaseList
		do

		    DelShareMem
		    difaEngine &
		    StartTime=`date '+%s'`
			
			echo "\t => Execute sub-case: $curcase."
			export case_name=$curcase
			python -m pyinspect -C ./gen_criterion.xml -t $Case
		
			Wait difaEngine
			EndTime=`date '+%s'`
			TimeCost=`expr $EndTime - $StartTime`
			echo "[$Index]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
		done
		
		let Index++
		export INDEX=$Index
		#exit 0
	done
}

Analyze

