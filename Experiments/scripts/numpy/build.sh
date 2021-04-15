
Wait ()
{
	process=$1
	while true
	do
		count=`ps -ef | grep $process | grep -v "grep" | wc -l`
		if [ 0 == $count ];then
			break
		else
			sleep 1
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

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

python -m pyinspect -c -E $SCRIPTS/ExpList -d $target

# 2. build and instrument C modules
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
rm -rf build
rm -rf /tmp/difg/LdaBin*
python setup-lda.py build_ext --inplace

SdaAnalysis

# 3. build again and install the instrumented software
rm -rf build
python setup-instm.py build_ext --inplace


# 4. generate file maping
GenMap $SCRIPTS $CASE_PATH $target

# 5. run the cases
Analyze ()
{
	CaseList=`cat case_list.txt`
	for curcase in $CaseList
	do
	    DelShareMem
	    difaEngine &
		echo ".......................run case $curcase......................."
		export case_name=$curcase
		python -m pyinspect -C ./gen_criterion.xml -t runtests.py -v -m full
		
		Wait difaEngine
	done
}

Analyze

