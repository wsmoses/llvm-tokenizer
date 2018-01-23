CMAKE=${CMAKE:="`which cmake`"}
VERBOSE=${VERBOSE:=0}

LLVM_VER="5.0"
#Install dependencies

#wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -

PACKAGES="ninja-build unzip libz-dev cmake m4 llvm-${LLVM_VER}-runtime libllvm${LLVM_VER} llvm-${LLVM_VER}-dev llvm-${LLVM_VER} clang-${LLVM_VER} libclang-${LLVM_VER}-dev"
#llvm38-dev"

for p in $PACKAGES; do
    echo "Checking for $p"
    dpkg-query --show $p > /dev/null 2>&1 || sudo apt-get install -q -y --no-install-recommends $p || exit 1
done

PYTHON=${PYTHON:="python"}
${PYTHON} -m pip install . --upgrade --verbose 
