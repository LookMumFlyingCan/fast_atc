#!/usr/bin/env fish

while inotifywait -e close_write main.cpp
  g++ -g -DLIBCXX_DEBUG -Wall -I . -O2 main.cpp -w -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system -lX11 -o atc
end
