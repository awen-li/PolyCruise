#  !bash
# type './build.sh'  for release build
# type './build.sh debug'  for debug build

# check the environment variables
EnvVerify ()
{
if [ $LLVM_PATH == "" ]; then
 echo "please set ENV variable LLVM_PATH"
 exit
fi

if [ $CLANG_PATH == "" ]; then
 echo "please set ENV variable CLANG_PATH"
 exit
fi

echo "Environment is checking ok!"
}

EnvVerify
export LLVM_DIR=$LLVM_PATH/build
export PATH=$LLVM_DIR/bin:$PATH

BUILD_DIR="build"
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -D CMAKE_BUILD_TYPE:STRING=Debug ../

make -j4

