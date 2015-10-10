#!/bin/bash
path_to_executable=$(which cmake28)
if [ -x "$path_to_executable" ] ; then
  echo "Using cmake28 ..."
  CMAKE=cmake28
else 
  echo "No cmake28 found. Using cmake."
  echo "Assuming it is recent enough."
  echo "This might fail."
  CMAKE=cmake
fi

echo "Pull all external modules .."
git submodule init
git submodule update
echo "Configure and build dependencies .."
cd build
$CMAKE ..
make -j 5
echo "Configure and build Indri ..."
cd ../external/indri-5.9
./configure
make -j 5
echo "Compile the Indri extractor ..."
cd ../../src
make
cd ..
cp src/mk_wand_idx bin/mk_wand_idx
cp src/kstem_query bin/kstem_query
cp build/wand_search bin/wand_search
echo "Binaries are now in the bin directory"
