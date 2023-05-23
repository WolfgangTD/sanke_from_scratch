all:
	clang++ -Isrc/Include -Lsrc/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer