# Use a base image with a C++ compiler, like gcc
FROM debian:bullseye-slim

# Set the working directory
WORKDIR /game

# Install the packages necessary to download and build SFML, tmxlite, and SQLite
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    git \
    libflac-dev \
    libogg-dev \
    libopenal-dev \
    libvorbis-dev \
    libfreetype6-dev \
    libx11-dev \
    libxrandr-dev \
    libudev-dev \
    libgl1-mesa-dev \
    sqlite3 \
    libsqlite3-dev \
    libsfml-dev \
    wget \
    unzip 

# Install SFML - replace '2.5.1' with the version you need
# RUN git clone --branch 2.5.1 --depth 1 https://github.com/SFML/SFML.git \
#     && mkdir SFML/build && cd SFML/build \
#     && cmake .. && make && make install 

# Install tmxlite
RUN git clone --depth 1 https://github.com/fallahn/tmxlite.git \
    && cd tmxlite && mkdir tmxlite/build && cd tmxlite/build \
    && cmake .. && make install 

ENV LD_LIBRARY_PATH /usr/local/lib/

# Copy the game source code into the Docker image
COPY . /game

# Compile the game
RUN g++ -o MyGame src/*.cpp -std=c++17 -L/usr/local/lib/ -I/usr/local/include/ -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -ltmxlite -lsqlite3

# Set the command to run the game
CMD ["./MyGame"]
