
PythonInstallPath ()
{
	PyBin=`which python`
	IsAnaconda=`echo $PyBin | grep anaconda`
	
	PyVersion=`python -c 'import platform; major, minor, patch = platform.python_version_tuple(); print(str(major)+"."+str(minor))'`
	if [ ! -n "$IsAnaconda" ]; then
	    echo "/usr/lib/python$PyVersion"
	else
	    echo "/usr/lib/anaconda3/lib/python$PyVersion"
	fi
}

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./build -pre=1
	BC_FILES=`find ./build -name *.preopt.bc`
	for bc in $BC_FILES
	do
		sda -file $bc -criterion c_criterion.xml -guard=0 -disglobal=1
	done
}


GenMap ()
{
    SCRIPTS=$1
    CASE_PATH=$2
    target=$3
    
    INSTALL_PATH=`find $PythonPath/ -name $target`
    if [ ! -n "$INSTALL_PATH" ]; then
    	echo "!!!!!!!!INSTALL_PATH of $target is NULL, need to specify a install path............."
    	exit 0
    fi
    		
    find $INSTALL_PATH -name "*.py" > "$CASE_PATH/$target.ini"
        
    cd $CASE_PATH
    python -m pyinspect -M "$target.ini" pyList
    cd -
        
    cp $CASE_PATH/Pymap.ini $pyMap
    return
}

DelShareMem ()
{
	ShareId=`ipcs -m | grep 0xc3b3c5d0 | awk '{print $2}'`
	if [ -n "$ShareId" ]; then
		ipcrm -m $ShareId
	fi	
}

BuildTarget ()
{
    # clean
    if [ -d "$CASE_PATH" ]; then
        rm -rf $CASE_PATH
    fi
	
    ################################################
    # 1. build and translate python modules
    ################################################
    #1.1 clone the code from github
    if [ ! -d "$target" ]; then
    	git clone https://github.com/Daybreak2019/cvxopt.git
    fi
    
    #1.2 collect all definitions of functions in pythob module
    if [ ! -f "$target/function_def.pkl" ]; then
        python -m pyinspect -g $target
        cp -f function_def.pkl $target/
    fi
	
    #1.3 rewrite python modules
    python -m pyinspect -c -E $SCRIPTS/ExpList -d $target

    ################################################
    # 2. build the whole project and conduct SDA
    ################################################
    cd $CASE_PATH
    python setup-sda.py build
    SdaAnalysis

    ################################################
    # 3.  build again with instrumentation
    ################################################
    rm -rf build
    rm -rf `find $PythonPath -name $target`
    
	#3.1 build with instrumentation
    python setup-instm.py install
    #rename
    TargetPath=`find $PythonPath -name $target`
    cp -rf $TargetPath $PythonPath/site-packages/$target
    rm -rf $TargetPath
    
    #3.2 generation the mapping between installing and source paths  	
    GenMap $SCRIPTS $CASE_PATH $target
}

############################################################
# cvxopt: version = 1.2.6
# python version: 3.7
# OS: Ubuntu18.04
# LLVM: 7.0
############################################################
target=cvxopt
Action=$1

cd ../../
export ROOT=`pwd`
export CASE_PATH=$ROOT/Temp/$target
export SCRIPTS=$ROOT/scripts/$target
export PythonPath=$(PythonInstallPath)

################################################
# dependence of cvxopt
################################################
if [ ! -d "$ROOT/SuiteSparse" ]; then
    wget http://faculty.cse.tamu.edu/davis/SuiteSparse/SuiteSparse-4.5.3.tar.gz
    tar -xf SuiteSparse-4.5.3.tar.gz
fi
export CVXOPT_SUITESPARSE_SRC_DIR=$ROOT/SuiteSparse


################################################
# build the target with PolyCruise
################################################
if [ "$Action" == "build" ]; then
    BuildTarget
fi

################################################
# Run the cases
################################################
mv /tmp/difg/plugins.ini /tmp/difg/plugins.ini-back
cp $SCRIPTS/plugins.ini /tmp/difg/ -rf
cd $CASE_PATH
ALL_TESTS=`ls *tt.py`

for Case in $ALL_TESTS
do
    StartTime=`date '+%s'`
        
    echo
    echo
    echo "********************* Running the script ---- <$Case> ---- *********************"
    DelShareMem
       
    python -m pyinspect -C py_criterion.xml -t $Case
        
    difaEngine

    EndTime=`date '+%s'`
    TimeCost=`expr $EndTime - $StartTime`
    echo "[$Case]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
done

# recover the environment
mv /tmp/difg/plugins.ini-back /tmp/difg/plugins.ini

