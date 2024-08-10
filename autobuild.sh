#!/bin/bash  
set -x  # 开启调试模式，显示执行的命令及其参数  
  
BUILD_DIR="$(pwd)/build"  
  
# 删除 build 目录下的所有内容，但保留 build 目录本身  
rm -rf "$BUILD_DIR"/*  
  
# 进入 build 目录并执行 cmake 和 make  
cd "$BUILD_DIR" && \  
    cmake .. && \  
    make
