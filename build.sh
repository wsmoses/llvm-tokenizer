CMAKE=${CMAKE:="`which cmake`"}
VERBOSE=${VERBOSE:=0}
core_count=`lscpu -p | egrep -v '^#' | wc -l`
CORES=${CORES:=$core_count}
LLVM_PREFIX=${LLVM_PREFIX:="`llvm-config-3.9 --prefix`"}
INSTALL_PREFIX="`pwd`/install"

#Install dependencies

PACKAGES="python3 llvm-3.9 clang-3.9 libclang-3.9-dev"
#llvm38-dev"

for p in $PACKAGES; do
    echo "Checking for $p"
    dpkg-query --show $p > /dev/null 2>&1 || sudo apt-get install -q -y --no-install-recommends $p || exit 1
done

pip3 install . --upgrade
