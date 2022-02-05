
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
		sda -file $bc -criterion c_criterion.xml -guard=0
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

target=cvxopt
Action=$1

cd ../../
ROOT=`pwd`

####### cvxopt dependence #######
if [ ! -d "$ROOT/SuiteSparse" ]; then
    wget http://faculty.cse.tamu.edu/davis/SuiteSparse/SuiteSparse-4.5.3.tar.gz
    tar -xf SuiteSparse-4.5.3.tar.gz
fi
export CVXOPT_SUITESPARSE_SRC_DIR=$ROOT/SuiteSparse
export PythonPath=$(PythonInstallPath)

# 1. build and translate python modules
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

if [ "$Action" == "build" ]; then
    if [ ! -d "$target" ]; then
    	git clone https://github.com/Daybreak2019/cvxopt.git
    fi
    
	rm -rf $CASE_PATH
	if [ ! -f "$target/function_def.pkl" ]; then
		python -m pyinspect -g $target
		cp -f function_def.pkl $target/
		#cp -f $target"_gen_criterion.xml" $target/gen_criterion.xml
	fi
	python -m pyinspect -c -E $SCRIPTS/ExpList -d $target
fi

# 2. build and SDA
cp criterion.xml $CASE_PATH/
cd $CASE_PATH
if [ "$Action" == "build" ]; then
	rm -rf build
	python setup-sda.py build
	export DIS_GLBTAINT=1
	SdaAnalysis
	unset DIS_GLBTAINT
fi

# 3. build again and install the instrumented software
if [ "$Action" == "build" ]; then
    rm -rf build
    rm -rf `find $PythonPath -name $target`
	
    python setup-instm.py install
    #rename
    TargetPath=`find $PythonPath -name $target`
    cp -rf $TargetPath $PythonPath/site-packages/$target
    rm -rf $TargetPath
        	
    GenMap $SCRIPTS $CASE_PATH $target
fi

# 5. run the cases
Analyze ()
{
    cp $SCRIPTS/plugins.ini /tmp/difg/ -rf
    cd $CASE_PATH
    ALL_TESTS=`ls *tt.py`

    for Case in $ALL_TESTS
    do
        StartTime=`date '+%s'`
    
        python -m pyinspect -C py_criterion.xml -t $Case
        
        difaEngine

        EndTime=`date '+%s'`
        TimeCost=`expr $EndTime - $StartTime`
        echo "[$Case]@@@@@ time cost: $TimeCost [$StartTime, $EndTime]"
    done
}

Analyze

