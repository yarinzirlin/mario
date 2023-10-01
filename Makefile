.PHONY: default
default: all;

DEBUG ?= 0

all:
	g++ -g -DDEBUG=$(DEBUG) src/*.cpp -I /opt/homebrew/Cellar/sfml/2.5.1_2/include -std=c++17 -o bin/portal -L/opt/homebrew/Cellar/sfml/2.5.1_2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -ltmxlite
