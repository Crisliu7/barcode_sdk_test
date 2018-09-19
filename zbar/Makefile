all:
	g++ -O2 -std=c++0x zbar.cpp ../Tools.cpp -lzbar -lz -lpthread `pkg-config opencv --cflags --libs` -o zbar

clean:
	rm -f zbar
