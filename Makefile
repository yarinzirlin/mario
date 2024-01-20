.PHONY: default
default: all;

DEBUG ?= 0

all:
	g++ -g -DDEBUG=$(DEBUG) src/*.cpp -I /opt/homebrew/Cellar/sfml/2.6.1/include -std=c++17 -o mario -L/opt/homebrew/Cellar/sfml/2.6.1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -ltmxlite -lsqlite3
