CC=gcc
CCFLAGS=-c

DBRLIB_PATH=lib
LDFLAGS=-L $(DBRLIB_PATH) -Wl,-rpath=$(DBRLIB_PATH) -Wl,-rpath=./
DBRLIB=-lDynamsoftBarcodeReader

STDLIB=-lstdc++

TARGET=ReadBarcode
OBJECT=ReadBarcode.o
SOURCE=dynamsoft.cpp

# build rule for target.
$(TARGET): $(OBJECT)
	$(CC) -o $(TARGET) $(OBJECT) $(STDLIB) $(DBRLIB) $(LDFLAGS)

# target to build an object file
$(OBJECT): $(SOURCE)
	$(CC) $(CCFLAGS) $(SOURCE)


all:
	g++ -O2 -std=c++0x dynamsoft.cpp ../Tools.cpp -lDynamsoftBarcodeReader $(LDFLAGS) -lz -lpthread `pkg-config opencv --cflags --libs` -o dynamsoft
# the clean target
.PHONY : clean
clean: 
	rm -f dynamsoft

