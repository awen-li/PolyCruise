
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
    	INSTALL_PATH="/usr/lib/anaconda3/lib/python3.7/site-packages/salt-3003rc1+1108.gb011a4ed6b-py3.7.egg/salt"
        find $INSTALL_PATH -name "*.py" > "$target.ini"
        python -m pyinspect -M "$target.ini" pyList	
    fi
    
    return
}

target=salt
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
	rm -rf /tmp/difg/LdaBin*
	python setup.py install
fi


# 4. generate file maping
GenMap $SCRIPTS $CASE_PATH $target

GenOneTestCases ()
{
	Case=$1
	
	rm -rf case_list.txt
	export case_dump=case_list.txt

	python -m pytest $Case
	unset case_dump

}

# 5. run the cases
Analyze ()
{
    GenOneTestCases tests/runtests./py 
    
	Type=$1
	Index=1
	CaseList=`cat case_list.txt`
	for curcase in $CaseList
	do
		if [ -n $INDEX ] && [ $Index != $INDEX ]; then
			let Index++
			continue
		fi	
	    DelShareMem
	    difaEngine &
	    StartTime=`date '+%s'`
		echo "[$Index].......................run case $curcase......................."
		export case_name=$curcase
		
		if [ "$Type" == "ORG" ]; then
			python setup.py test
		else
			python -m pyinspect -C ./gen_criterion.xml -t tests/runtests.py 
		fi
	
		Wait difaEngine
		EndTime=`date '+%s'`
		TimeCost=`expr $EndTime - $StartTime`
		echo "[$Index]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
		
		let Index++
		export INDEX=$Index
	done
}

Analyze

