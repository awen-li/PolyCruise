

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	Target=$1
	
	sda -dir ./ -pre=1
	BC_FILES=`find ./ -name "*.preopt.bc"`
	Index=1
	for bc in $BC_FILES
	do		
		#result=$(echo $bc | grep "[tT]est")
		result=$(echo $bc | grep $Target)
		if [[ "$result" == "" ]]; then
		    let Index++
    		continue
		fi
		
		echo "@@@@@ [$Index].......................sda -file $bc......................."
		sda -file $bc -criterion ../criterion.xml	
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


target=libtiff

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/$target
SCRIPTS=$ROOT/scripts/$target

cd $CASE_PATH

export CC="clang -O3 -emit-llvm -flto"
export CXX="clang -O3 -emit-llvm -flto"
export LDSHARED="clang -O3 -flto -shared"
#export RANLIB=/bin/true

./configure
make clean && make

rm -rf /tmp/difg/LdaBin*
SdaAnalysis bmp2tiff.0.0.preopt.bc


# 3. build again and install the instrumented software
export CFLAGS="-O3 -Xclang -load -Xclang llvmSDIpass.so"
export CXXFLAGS="-O3 -Xclang -load -Xclang llvmSDIpass.so"
export CC="clang"
export CXX="clang"
export LDFLAGS="-lDynAnalyze"
export LDSHARED="clang -O3 -flto -shared -pthread -lDynAnalyze"
#export RANLIB=/bin/true

./configure
make clean && make