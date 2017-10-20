CMAKE=${CMAKE:="`which cmake`"}
VERBOSE=${VERBOSE:=0}
core_count=`lscpu -p | egrep -v '^#' | wc -l`
CORES=${CORES:=$core_count}
LLVM_PREFIX=${LLVM_PREFIX:="`llvm-config-3.9 --prefix`"}
INSTALL_PREFIX="`pwd`/install"

#Install dependencies

PACKAGES="python3 llvm-3.9 clang-3.9"
#llvm38-dev"

for p in $PACKAGES; do
    echo "Checking for $p"
    dpkg-query --show $p > /dev/null 2>&1 || sudo apt-get install -q -y --no-install-recommends $p || exit 1
done


mkdir -p build && cd build || exit 1
VERBOSE=$VERBOSE $CMAKE -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DLLVM_PREFIX=${LLVM_PREFIX} .. || exit 1
VERBOSE=$VERBOSE make -j $CORES || exit 1

mkdir -p $INSTALL_PREFIX || exit 1
VERBOSE=$VERBOSE make -j $CORES install || exit 1


