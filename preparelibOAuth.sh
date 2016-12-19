#!/bin/sh
git submodule update --init --recursive
xcodebuild -project libOAuth/liboauth.xcodeproj -scheme OAuth -configuration Release
if [ ! -e ./lib ]; then
  mkdir lib
fi
buildPath=libOAuth/DerivedData/Build/Products/Release/
cp ${buildPath}z/*.a  lib/.
cp ${buildPath}OpenSSL/lib/*.a lib/.
cp ${buildPath}liboauth/*.a lib/.

if [ ! -e ./include ]; then 
  mkdir include
fi
cp -r ${buildPath}z/usr/local/include/* include/.
cp -r ${buildPath}OpenSSL/include/* include/.
cp -r ${buildPath}liboauth/usr/local/include/* include/.
