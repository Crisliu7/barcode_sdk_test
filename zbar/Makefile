one:
	g++ -O2 -std=c++0x zbar_off.cpp -lzbar -lMagick++ -o zbar

onec:
	# gcc -O2 -std=c99 zbar.c -lscanditsdk -lz -lpthread -lSDL2 -lSDL2_image -o zbar
	c++ -O2 -o zbar zbar_off.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`


clean:
	rm -f zbar
