.PHONY: default
default: all;

DEBUG ?= 0

install:
	sudo apt-get update && sudo apt-get install -y cmake g++ git libflac-dev libogg-dev libopenal-dev libvorbis-dev libfreetype6-dev libx11-dev libxrandr-dev libudev-dev libgl1-mesa-dev sqlite3 libsqlite3-dev libsfml-dev wget unzip
	if [ ! -d "tmxlite" ] ; then git clone https://github.com/fallahn/tmxlite.git tmxlite; else cd "tmxlite" && git pull https://github.com/fallahn/tmxlite.git; fi
	cd tmxlite && mkdir -p tmxlite/build && cd tmxlite/build \
	&& cmake .. && sudo make install && cd ../../../../../..



all:
	g++ -g -DDEBUG=$(DEBUG) src/*.cpp -I /opt/homebrew/Cellar/sfml/2.6.1/include -std=c++17 -o mario -L/opt/homebrew/Cellar/sfml/2.6.1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -ltmxlite -lsqlite3
