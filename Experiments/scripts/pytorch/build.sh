

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./ -pre=1
	BC_FILES=`find ./ -name *.preopt.bc`
	BC_LIST=()
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
    	INSTALL_PATH=`find /usr/local/lib/python3.7/ -name $target`
        find $INSTALL_PATH -name "*.py" > "$target.ini"
        python -m pyinspect -M "$target.ini" pyList	
    fi
    
    return
}

target=pytorch

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
export CMAKE_PREFIX_PATH=${CONDA_PREFIX:-"$(dirname $(which conda))/../"}
export CC="clang -emit-llvm -flto -pthread"
export CXX="clang++ -emit-llvm -flto -pthread"
export LDSHARED="clang -flto -shared -pthread -lm"
export RANLIB=/bin/true
python setup.py develop

SdaAnalysis


# 3. build again and install the instrumented software
rm -rf build
export CMAKE_PREFIX_PATH=${CONDA_PREFIX:-"$(dirname $(which conda))/../"}
export CC="clang -emit-llvm -flto -pthread -Xclang -load -Xclang llvmSDIpass.so"
export CXX="clang++ -emit-llvm -flto -pthread  -Xclang -load -Xclang llvmSDIpass.so"
export LDSHARED="clang -flto -shared -pthread -lm -lDynAnalyze"
export RANLIB=/bin/true
python setup.py develop


# 4. generate file maping
#GenMap $SCRIPTS $CASE_PATH $target

# 5. run the cases
TestCase=()
CaseNum=${#TestCase[*]}
Index=1
for Case in ${TestCase[@]}
do
	echo "[$Index/$CaseNum]======================= Execute the case $Case ======================="
	DelShareMem
	difaEngine &
	
	python -m pyinspect -C ../../criterion.xml -t $Case &
	sleep 60
	let Index++
	
	killall python     2> /dev/null
	killall difaEngine 2> /dev/null
	sleep 15
done
