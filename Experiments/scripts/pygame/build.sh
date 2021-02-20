

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./build -pre=1
	BC_FILES=`find ./build -name *.preopt.bc`
	for bc in $BC_FILES
	do
		echo "...SdaAnalysis -> $bc"
		sda -file $bc -criterion ../../criterion.xml
	done
	
	echo "...Finish SdaAnalysis....................."
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

target=pygame

DelShareMem
difaEngine &

# 1. build and translate python modules
cd ../../
ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

python -m pyinspect -c -E $SCRIPTS/ExpList -d $target

# 2. build and instrument C modules
cp criterion.xml $CASE_PATH/
cp $SCRIPTS/setup-*.py $CASE_PATH/
cd $CASE_PATH
rm -rf build
python setup-lda.py build
SdaAnalysis

# 3. build again and install the instrumented software
rm -rf build
python setup-instm.py install


# 4. generate file maping
#GenMap $SCRIPTS $CASE_PATH $target

# 5. run the cases
python -m pyinspect -C ../../criterion.xml -t examples/font_viewer.py 






