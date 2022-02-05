
export CASE_PATH=`pwd`

if [ "$1" == "clean" ]; then
    find ./ -name DIFG* | xargs rm -rf
    find ./ -name build | xargs rm -rf
    find ./ -name Temp | xargs rm -rf
    find ./ -name gmon.out | xargs rm -rf
    exit 0
fi

CASE_GROUP=("DynamicInvocation"  "FieldSensitivity"  "GeneralFlow" "GlobalFlow" "ObjectSensitivity")

CaseNum=0
FailNum=0
unset DIS_GLBTAINT
unset DUMPGRAPH
# iter each case group
for group in ${CASE_GROUP[@]}
do
	echo $'\n'"@@@@@@ Execute group of benchmark $group @@@@@@"
	G_PATH=$CASE_PATH/$group
	Cases=`ls $G_PATH`
	
	for Case in $Cases
	do
		echo $'\t'"[$CaseNum] - Execute the case $Case"
		cd $G_PATH/$Case
		FailFlag=`./build.sh | grep "@@@@CASE-TEST FAIL"`
		if [ -n "$FailFlag" ]; then
			echo $FailFlag
			let FailNum++
	    fi
	
		let CaseNum++
	done
	
	rm -rf $G_PATH/Temp
done

echo
echo
let Correct=CaseNum-FailNum
echo "Finish test, (Correct/Total) = ($Correct, $CaseNum)"

