CC = g++
CFLAGS := -g -Wall -Weffc++ -std=c++11
OBJECT_FILES := bin/Main.o bin/Manager.o bin/Rule.o bin/OutputManager.o bin/XMLParser.o
INCLUDE_LIBRARIES = -I/usr/include -I$(POCO_LIBRARY)/Foundation/include -I$(POCO_LIBRARY)/Util/include -I$(POCO_LIBRARY)/Net/include -I$(POCO_LIBRARY)/XML/include
SHARED_LIBRARIES = -L/usr/lib -L$(POCO_LIBRARY)/lib/Linux/x86_64 -fpermissive
NETFILTER_LIBS = -lnetfilter_queue
POCO_LIBS = -lPocoNet -lPocoUtil -lPocoFoundation -lPocoXML -lPocoJSON

# All Targets
all: bin/sim

bin/sim: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) -o $@ $(SHARED_LIBRARIES) $(POCO_LIBS) $(NETFILTER_LIBS)

# Depends on the source and header files
bin/OutputManager.o: src/OutputManager.cpp #include/OutputManager.h
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE_LIBRARIES)

bin/Rule.o: src/Rule.cpp include/OutputManager.h #include/Rule.h
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE_LIBRARIES)

bin/Manager.o: src/Manager.cpp include/Rule.h #include/Manager.h
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE_LIBRARIES)

bin/XMLParser.o: src/XMLParser.cpp src/Manager.cpp src/Rule.cpp
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE_LIBRARIES)

bin/Main.o: src/Main.cpp src/XMLParser.cpp #scr/Manager.cpp
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE_LIBRARIES)

#Clean the build directory
clean:
	rm -rf bin/*
