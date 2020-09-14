#!/usr/bin/env bash
set -e
set -u
set -o pipefail


usage="$(basename "$0") [-h] [examples branch] [framework branch]  [unpacker branch] - program to install -pet framework

where:
    examples branch framework branch and unpacker branch are optional arguments 
    that allow to choose specific branch. The default value is master.
    -h  shows help.
    
    Example: ./get-me-a-jpet-framework.sh develop develop master
    It will launch the installation for the following setup:
    j-pet-framework-examples: develop
    j-pet-framework: develop
    unpacker:master
    "

if [ "$1" == "-h" ]; then
  echo "$usage"
  exit 0
fi

examples_branch="${1:-master}"
framework_branch="${2:-master}"
unpacker_branch="${3:-master}"


initial_location=`pwd`
trap "cd $initial_location" EXIT

let n_cpus=`getconf _NPROCESSORS_ONLN`/2
echo "The compilation will be parallelized using $n_cpus cores."

mkdir framework-bin
root_dir="$initial_location/framework-bin"
cd framework-bin/

mkdir unpacker
mkdir unpacker-build
mkdir core
mkdir core-build
mkdir examples

git clone --recurse-submodules https://github.com/JPETTomography/Unpacker2.git -b $unpacker_branch
git clone --recurse-submodules https://github.com/JPETTomography/j-pet-framework.git -b $framework_branch
git clone --recurse-submodules https://github.com/JPETTomography/j-pet-framework-examples.git -b $examples_branch 

cd $root_dir/unpacker-build

cmake ../Unpacker2 -DCMAKE_INSTALL_PREFIX=$root_dir/unpacker
make -j $n_cpus
make install

cd $root_dir/core-build

cmake ../j-pet-framework -DCMAKE_PREFIX_PATH=$root_dir/unpacker -DCMAKE_INSTALL_PREFIX=$root_dir/core
make -j $n_cpus
make install

cd $root_dir/examples

source $root_dir/core/bin/thisframework.sh
cmake ../j-pet-framework-examples -DCMAKE_INSTALL_PREFIX=`pwd`/../examples
make -j $n_cpus

cd $initial_location





