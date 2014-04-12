CASABLANCA_INCLUDE_DIR=/Users/mx/Projects/cpp/casablanca/Release/include
CASABLANCA_LIB_DIR=/Users/mx/Projects/cpp/casablanca/build.release/Binaries
CXX_FLAGS =  -stdlib=libc++ -Wno-return-type-c-linkage -Wno-unneeded-internal-declaration -std=c++11 -fno-strict-aliasing -O3 -DNDEBUG -I$(CASABLANCA_INCLUDE_DIR) -I/usr/local/include   
LDIR= -L/usr/lib -L/usr/local/lib -L$(CASABLANCA_LIB_DIR)

redmine: redmine.cpp
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LDIR) -lcasablanca -lboost_system-mt -lboost_thread-mt -Wno-sign-compare -Wno-unused-parameter 