

SdaAnalysis ()
{
	rm -rf /tmp/difg/LdaBin*
	
	sda -dir ./build -pre=1
	BC_FILES=`find ./build -name *.preopt.bc`
	for bc in $BC_FILES
	do
		echo "sda -file $bc -criterion ../../criterion.xml"
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
    	INSTALL_PATH=`find /usr/local/lib/python3.7/dist-packages/ -name $target`
        find $INSTALL_PATH -name "*.py" > "$target.ini"
        python -m pyinspect -M "$target.ini" pyList	
    fi
    
    return
}

Analyze ()
{
	Case=$1
	DelShareMem
	difaEngine &
	python -m pyinspect -C $ROOT/criterion.xml -t $Case &
}

Para=$1
echo "Recompile = $Para............"

target=pygame

DelShareMem
difaEngine &

# 1. build and translate python modules
cd ../../
export ROOT=`pwd`
CASE_PATH=$ROOT/Temp/$target
SCRIPTS=$ROOT/scripts/$target

if [ "$Para" == "recmpl" ]; then
	echo "Translate python sources............"
    python -m pyinspect -c -E $SCRIPTS/ExpList -d $target


    # 2. build and instrument C modules
    cp criterion.xml $CASE_PATH/
    cp $SCRIPTS/setup-*.py $CASE_PATH/
    cd $CASE_PATH
    rm -rf build
    python setup-lda.py build
    echo "Start SdaAnalysis............"
    SdaAnalysis

    # 3. build again and install the instrumented software
    rm -rf build
    echo "Start Instrumentation............"
    python setup-instm.py install

    # 4. generate file maping
    GenMap $SCRIPTS $CASE_PATH $target
else
	echo "Analyze a file $Para............"
    if [ -n $Para ]; then 
    	cd $CASE_PATH
    	Analyze $Para
    	sleep 2m
    	echo "Analyze finish............"
    	exit 0
    fi
fi

# 5. run the cases
cd $CASE_PATH
echo "Start run the case............"
#TestCase=(examples/aacircle.py examples/chimp.py examples/fonty.py examples/midi.py examples/sound.py\
#          examples/cursors.py examples/freetype_misc.py examples/moveit.py examples/resizing_new.py examples/sprite_texture.py\
#          examples/arraydemo.py  examples/glcube.py examples/music_drop_fade.py examples/scaletest.py examples/stars.py\
#          examples/audiocapture.py examples/dropevent.py examples/headless_no_windows_needed.py examples/overlay.py examples/scrap_clipboard.py examples/testsprite.py\
#          examples/blend_fill.py examples/eventlist.py  examples/pixelarray.py examples/scroll.py examples/textinput.py\
#          examples/blit_blends.py examples/fastevents.py examples/liquid.py examples/playmus.py examples/setmodescale.py examples/vgrade.py\
#          examples/camera.py examples/font_viewer.py examples/mask.py examples/prevent_display_stretching.py examples/sound_array_demos.py examples/video.py)
          
TestCase=(examples/aacircle.py examples/arraydemo.py examples/blend_fill.py examples/camera.py examples/fonty.py)
CaseNum=${#TestCase[*]}
Index=1
for Case in ${TestCase[@]}
do
	echo "[$Index/$CaseNum]======================= Execute the case $Case ======================="
	Analyze $Case
	
	sleep 60
	let Index++
	
	killall python     2> /dev/null
	killall difaEngine 2> /dev/null
	sleep 15
done







