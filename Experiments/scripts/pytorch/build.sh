
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
		if [ $second == 60 ]; then
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
	BC_FILES=`find ./ -name *.preopt.bc | grep torch`
	Index=1
	for bc in $BC_FILES
	do	
		echo "@@@@@ [$Index].......................sda -file $bc......................."
		sda -file $bc -criterion ../../criterion.xml	
		let Index++
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
    	INSTALL_PATH=`find /usr/local/lib/python3.7/ -name $target`
        find $INSTALL_PATH -name "*.py" > "$target.ini"
        python -m pyinspect -M "$target.ini" pyList	
    fi
    
    return
}

target=pytorch
Action=$1

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

if [ "$Action" == "build" ]; then
	rm -rf $CASE_PATH
	if [ ! -f "$target/function_def.pkl" ]; then
		python -m pyinspect -E $SCRIPTS/ExpList -g $target
		cp -f function_def.pkl $target/
	fi
	python -m pyinspect -c -E $SCRIPTS/ExpList -d $target
fi

# 2. build and instrument C modules
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
if [ "$Action" == "build" ]; then
	rm -rf build
	export CMAKE_PREFIX_PATH=${CONDA_PREFIX:-"$(dirname $(which conda))/../"}
	export CC="clang -emit-llvm -flto -pthread"
	export CXX="clang++ -emit-llvm -flto -pthread"
	export LDSHARED="clang -flto -shared -pthread -lm"
	export RANLIB=/bin/true
	python setup.py develop

	SdaAnalysis
fi


# 3. build again and install the instrumented software
if [ "$Action" == "build" ]; then
	rm -rf build
	export CMAKE_PREFIX_PATH=${CONDA_PREFIX:-"$(dirname $(which conda))/../"}
	export CC="clang -emit-llvm -flto -pthread -Xclang -load -Xclang llvmSDIpass.so"
	export CXX="clang++ -emit-llvm -flto -pthread -lDynAnalyze -Xclang -load -Xclang llvmSDIpass.so"
	export LDSHARED="clang -flto -shared -pthread -lm -lDynAnalyze"
	export RANLIB=/bin/true
	python setup.py develop
fi

# 4. generate file maping
#GenMap $SCRIPTS $CASE_PATH $target

# 5. run the cases
Analyze ()
{
	echo "" > $SCRIPTS/build.log
	Index=1
	CaseList1=`find ./ -name "test*py"` 
	CaseList2=`find ./ -name "*test.py"`
	CaseList=$CaseList1" "$CaseList2
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
		python -m pyinspect -C ./gen_criterion.xml -t $curcase 
		
		Wait difaEngine
		EndTime=`date '+%s'`
		TimeCost=`expr $EndTime - $StartTime`
		echo "[$Index]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"

		let Index++
		export INDEX=$Index
	done
}
Analyze