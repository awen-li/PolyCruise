
export CASE_PATH=`cd ../../japronto && pwd`

# 1. install C module of the case
cp ../criterion.xml $CASE_PATH/
cp ./setup-*.py $CASE_PATH/
cd $CASE_PATH
rm -rf build
python setup-lda.py build
sda -file build/lib.linux-x86_64-3.7/japronto/request/crequest.cpython-37m-x86_64-linux-gnu.so.0.0.preopt.bc -criterion ../criterion.xml
rm -rf build
python setup-instm.py build
cd $CASE_PATH


