

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./ -pre=1
	BC_FILES=`find ./ -name *.preopt.bc`
	BC_LIST=(_simd.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/core/_struct_ufunc_tests.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc
	         ./numpy/core/_operand_flag_tests.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/core/_rational_tests.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/core/_umath_tests.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc
	         ./numpy/fft/_pocketfft_internal.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/_bounded_integers.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/_pcg64.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc
	         ./numpy/random/_mt19937.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/_generator.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/mtrand.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc
	         ./numpy/random/bit_generator.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/_common.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc ./numpy/random/_sfc64.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc
	          )
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

target=numpy

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

python -m pyinspect -c -E $SCRIPTS/ExpList -d $target
exit 0

# 2. build and instrument C modules
cp criterion.xml $CASE_PATH/
cp $SCRIPTS/setup-*.py $CASE_PATH/
cp $SCRIPTS/site.cfg-lda $CASE_PATH/site.cfg
cd $CASE_PATH
#rm -rf build
#python setup-lda.py build_ext --inplace

#SdaAnalysis

# 3. build again and install the instrumented software
rm -rf build
cp $SCRIPTS/site.cfg-instm $CASE_PATH/site.cfg
python setup-instm.py build_ext --inplace


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
