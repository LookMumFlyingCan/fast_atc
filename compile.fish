#!/usr/bin/env fish

while inotifywait -e close_write main.cpp
  g++ -g -DLIBCXX_DEBUG -Wall -I . -O2 main.cpp -w  -lX11 -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system -lpthread -o atc
end
