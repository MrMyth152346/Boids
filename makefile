all:
	g++ -I SDL/include -L SDL/lib -o boid -g src/main.cc -lmingw32 -lSDL2main -lSDL2