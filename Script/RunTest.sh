
export CASE_PATH=`pwd`

Cases=`ls | grep "_case_"`
Index=1
for Case in $Cases
do
	echo "[$Index]======================= Execute the case $Case ======================="
	cd $CASE_PATH/$Case
	./build.sh | grep "@@@@CASE-TEST"
	
	let Index++
done

