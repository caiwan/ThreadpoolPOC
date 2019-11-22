#!/bin/bash
BUILD_DIR=./build
[ -d "$BUILD_DIR" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ] && rm "$BUILD_DIR/CMakeCache.txt" || mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_INSTALL_PREFIX=deploy/ -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" -G "Unix Makefiles" ../
cmake --build ./ --target ExternalDependencies

cd ../
