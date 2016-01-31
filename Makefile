all: gamepart1

gamepart1: gamepart1.cpp glad.c
	g++ -o gamepart1 gamepart1.cpp glad.c -lGL -lglfw -ldl

clean:
	rm gamepart1
