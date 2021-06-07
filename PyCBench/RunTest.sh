
export CASE_PATH=`pwd`

if [ "$1" == "clean" ]; then
    find ./ -name DIFG* | xargs rm -rf
    find ./ -name build | xargs rm -rf
    find ./ -name Temp | xargs rm -rf
    find ./ -name gmon.out | xargs rm -rf
    exit 0
fi

CASE_GROUP=("DynamicInvocation"  "FieldSensitivity"  "GeneralFlow" "GlobalFlow" "ObjectSensitivity")

# iter each case group
for group in ${CASE_GROUP[@]}
do
	echo $'\n'"@@@@@@ Execute group of benchmark $group @@@@@@"
	G_PATH=$CASE_PATH/$group
	Cases=`ls $G_PATH | grep "_case_"`
	
	Index=1
	for Case in $Cases
	do
		echo $'\t'"[$Index] - Execute the case $Case"
		cd $G_PATH/$Case
		./build.sh | grep "@@@@CASE-TEST"
	
		let Index++
	done
done

