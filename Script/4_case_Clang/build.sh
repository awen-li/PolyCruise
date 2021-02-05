
export CASE_PATH=`pwd`

# 1. install C module of the case
echo "@@@@@@@@@ C component.........."
cd $CASE_PATH/C
make -f makefile-sda clean && make -f makefile-sda
sda -criterion ../criterion.xml -file demo.0.0.preopt.bc
make -f makefile-sdi clean && make -f makefile-sdi

# 3. run the case
echo "@@@@@@@@@ runing the case.........."
cd $CASE_PATH
export CASE1="8"
./C/demo


