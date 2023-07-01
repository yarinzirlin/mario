.PHONY: default
default: all;


all:
	g++ ./*.cpp -I /opt/homebrew/Cellar/sfml/2.5.1_2/include -std=c++17 -o bin/portal -L/opt/homebrew/Cellar/sfml/2.5.1_2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
