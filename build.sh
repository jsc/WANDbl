#!/bin/bash
echo "Pull all external modules .."
git submodule init
git submodule update
echo "Configure and build dependencies .."
cd build
cmake28 ..
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
