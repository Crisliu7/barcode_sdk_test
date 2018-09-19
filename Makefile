DBRLIB_PATH=dynamsoft/lib
LDFLAGS=-L $(DBRLIB_PATH) -Wl,-rpath=$(DBRLIB_PATH) -Wl,-rpath=./

.PHONY: zbar scandit dynamsoft

all:
	g++ -O2 -std=c++0x zbar/zbar.cpp Tools.cpp -lzbar -lz -lpthread `pkg-config opencv --cflags --libs` -o zbar/zbar
	g++ -O2 -std=c++0x scandit/scandit.cpp Tools.cpp -lscanditsdk -lz -lpthread `pkg-config opencv --cflags --libs` -o scandit/scandit
	g++ -O2 -std=c++0x dynamsoft/dynamsoft.cpp Tools.cpp -lDynamsoftBarcodeReader $(LDFLAGS) -lz -lpthread `pkg-config opencv --cflags --libs` -o dynamsoft/dynamsoft

zbar:
	g++ -O2 -std=c++0x zbar/zbar.cpp Tools.cpp -lzbar -lz -lpthread `pkg-config opencv --cflags --libs` -o zbar/zbar

scandit:
	g++ -O2 -std=c++0x scandit/scandit.cpp Tools.cpp -lscanditsdk -lz -lpthread `pkg-config opencv --cflags --libs` -o scandit/scandit

dynamsoft:
	g++ -O2 -std=c++0x dynamsoft/dynamsoft.cpp Tools.cpp -lDynamsoftBarcodeReader $(LDFLAGS) -lz -lpthread `pkg-config opencv --cflags --libs` -o dynamsoft/dynamsoft


clean:
	rm -f zbar/zbar scandit/scandit dynamsoft/dynamsoft
