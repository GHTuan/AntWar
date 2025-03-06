all:
	g++ -I src/include -L src/lib -o main main.cpp Object.cpp Physic2D.cpp Game.cpp Sound.cpp Vector2.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

run:	
	./main