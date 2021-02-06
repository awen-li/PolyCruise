
export CASE_PATH=`pwd`


# 2. translate the python code
rm -rf /tmp/difg/LdaBin*

echo "@@@@@@@@@ Python component.........."
CASE=`basename $CASE_PATH`
cd ..
python -m pyinspect -E $CASE/ExpList -c -d $CASE

# 3. run the case
echo "@@@@@@@@@ runing the case.........."
export CASE1="8"
cd Temp/$CASE/
python -m pyinspect -C criterion.xml -t Python/Demo.py

