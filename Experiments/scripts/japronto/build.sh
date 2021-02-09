
export CASE_PATH=`cd ../../japronto && pwd`

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./build -pre=1
	BC_FILES=`find ./build -name *.preopt.bc`
	for bc in $BC_FILES
	do
		sda -file $bc -criterion ../criterion.xml
	done
}

# 1. install C module of the case
cp ../../criterion.xml $CASE_PATH/
cp ./setup-*.py $CASE_PATH/
cd $CASE_PATH
rm -rf build
python setup-lda.py build
SdaAnalysis
rm -rf build
python setup-instm.py build
cd $CASE_PATH


