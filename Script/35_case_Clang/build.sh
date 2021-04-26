
export CASE_PATH=`pwd`

DelShareMem ()
{
	ShareId=`ipcs -m | grep 0xc3b3c5d0 | awk '{print $2}'`
	if [ -n "$ShareId" ]; then
		ipcrm -m $ShareId
	fi	
}

DelShareMem
difaEngine -c CaseResults -s &

# 1. install C module of the case
rm -rf /tmp/difg/LdaBin*

echo "@@@@@@@@@ C component.........."
cd $CASE_PATH/C
make -f makefile-sda clean && make -f makefile-sda
sda -criterion ../criterion.xml -file demo.0.0.preopt.bc
make -f makefile-sdi clean && make -f makefile-sdi

# 3. run the case
echo "@@@@@@@@@ runing the case.........."
cd $CASE_PATH
export CASE1="8888888"
./C/demo


