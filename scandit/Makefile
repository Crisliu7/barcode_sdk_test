all:
	g++ -O2 -std=c++0x scandit.cpp -lscanditsdk -lz -lpthread -lSDL2 -lSDL2_image `pkg-config opencv --cflags --libs` -o scandit

clean:
	rm -f scandit
