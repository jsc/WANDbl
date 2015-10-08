
# CONFIG REQUIRED: SET THIS VARIABLE TO THE PATH OF THE INDRI SOURCE CODE
INDRISRC=../external/indri-5.9
# CONFIG REQUIRED: SET THIS TO THE SDSL INSTALL PATH
SDSLPREFIX=../build/external/sdsl-lite/
DIVSUF=../build/external/sdsl-lite/external/libdivsufsort-2.0.1/
NEWTSRC=../include
FASTPFOR=../external/fastpfor/headers/

include $(INDRISRC)/MakeDefns
SHARED=
INCPATH=-I$(INDRISRC)/include $(patsubst %, -I$(INDRISRC)/contrib/%/include, $(DEPENDENCIES)) -I $(SDSLPREFIX)/include/ -I$(NEWTSRC) -I$(FASTPFOR) -I$(DIVSUF)/include -I..
LIBPATH=-L$(INDRISRC)/obj  $(patsubst %, -L$(INDRISRC)/contrib/%/obj, $(DEPENDENCIES)) -L $(SDSLPREFIX)/lib/ -L$(DIVSUF)/lib
LIBS=-lindri $(patsubst %, -l%, $(DEPENDENCIES)) -lsdsl -ldivsufsort ../build/libfastpfor_lib.a

all:
	$(CXX) $(CXXFLAGS) -O3 -msse4.2 -std=c++11 mk_wand_idx.cpp -o mk_wand_idx $(INCPATH) $(LIBPATH) $(LIBS) $(CPPLDFLAGS)
	$(CXX) $(CXXFLAGS) -O3 -msse4.2 -std=c++11 kstem_query.cpp -o kstem_query $(INCPATH) $(LIBPATH) $(LIBS) $(CPPLDFLAGS)

clean:
	rm -f mk_wand_idx kstem_query *~
