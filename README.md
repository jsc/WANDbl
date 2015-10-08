Acknowledgements
======
This code is derived from the following papers:
1. M. Petri, J. S. Culpepper, and A. Moffat. Exploring the Magic of WAND.
   Proceedings of the 18th Annual Australasian Document Computing Symposium
   (ADCS 2013) , pp 58-65, December 2013.
2. M. Petri, A, Moffat and J. S. Culpepper. Score-safe term dependency
   processing with hybrid indexes. Proceedings of the 37th Annual International
   Conference on Research and Development in Information Retrieval (SIGIR 2014)
   , pp 899-902, July 2014.
3. S. Gog and M. Petri. Compact indexes for flexible top-k retrieval. In Proc.
   CPM, pages 207–218, 2015. https://github.com/simongog/surf

Several people have contributed to this codebase over the years. They are:
Matthias Petri
Simon Gog
Joel Mackenzie
Yubin Kim
Shane Culpepper

Build Info
=========
* gcc 4.8.x 

```
git submodule init
git submodule update
cd build
cmake28 ..
make
cd ../external/indri-5.9
./configure
make
cd ../../src
make
```

Binary Info
======
There are two binaries.
1. bin/mk_wand_index -c GOV2_STOP wand_out
This will convert the Indri Index in GOV2_STOP generated using the file
ir-repo/index-GOV2_STOP.param. The WAND index will be in the wand_out
directory.
2. bin/wand_search -c wand_out -q ir-repo/gov2-2004.qry -k 1000 -o
   gov2-2004 
   Will run queries 701-750 on the GOV2 stopped collection, and generate 
   a timing and run file with the prefix gov2-2004.

Note that the input queries must be krovetz stemmed. There is no stemmer
built into the query engine. You can use the kstem_query program to
stem a text string. Its up to you to add the <query_number>; before
the query.