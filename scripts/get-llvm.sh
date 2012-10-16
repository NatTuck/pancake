#!/bin/bash

if [[ ! -d ~/Apps ]]
then
  mkdir ~/Apps
fi
cd ~/Apps

if [[ ! -d build ]]
then
  mkdir build
fi
cd build

svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_31/final llvm
(cd llvm/tools &&
    svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_31/final clang)
(cd llvm/projects &&
    svn co http://llvm.org/svn/llvm-project/compiler-rt/tags/RELEASE_31/final compiler-rt)

# Need to build and install llvm first here?

(cd llvm/projects &&
    git clone https://bitbucket.org/gnarf/axtor.git)
