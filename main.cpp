#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <iomanip>
#include <thread>
#include <mutex>
#include <map>
#include <thread>

#include <unistd.h>

typedef unsigned char byte;

#include "backend/plane.h"

#include <backend/socket.cpp>

#include "window/window.h"
#include "window/params.h"
#include <SFML/Graphics.hpp>

using namespace std;

std::mutex plane_access;
std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > planes;

std::mutex sat_access;
sat_status status;

sf::ContextSettings s { .antialiasingLevel = 5 };

void packets() {
  Socket sock(2008);

  sock.loop(plane_access, planes, sat_access, status);
}

int main(int argc, char *argv[]){
  cerr << setprecision(20);

  std::thread(packets).detach();

  windowParams plnparams {
    .bar_width = 2,
    .bar_offset = 20,
    .bar_lenght = 10,
    .hei = 400, .wid = 700,
    .bg = sf::Color(0,0,0),
    .muted = sf::Color(120,120,120),
    .secondary = sf::Color(170,210,190),
    .tert = sf::Color(170,250,140),
    .quadr = sf::Color(170,250,255),
    .selection = sf::Color::Red,
    .s = s,
    .alpha = 255,
    .hstart = 51, .hend = 53,
    .vstart = 2, .vend = 4,
    .label_size = 14, .scale_width = 1,
    .shift_factor = 25, .text_offset = 6,
    .title = L"Atc board view"
  };

  windowParams params {
    .bar_width = 2,
    .bar_offset = 20,
    .bar_lenght = 10,
    .hei = 400, .wid = 400,
    .bg = sf::Color(0,0,0),
    .muted = sf::Color(120,120,120),
    .secondary = sf::Color(170,210,190),
    .tert = sf::Color(170,250,140),
    .quadr = sf::Color(170,250,255),
    .selection = sf::Color::Red,
    .s = s,
    .alpha = 255,
    .hstart = 51, .hend = 53,
    .vstart = 2, .vend = 4,
    .label_size = 14, .scale_width = 1,
    .shift_factor = 25, .text_offset = 6,
    .title = L"Atc radar view"
  };

  radarWindow radar(params, plnparams, plane_access, planes, sat_access, status);

  radar.fillBuffers();

  radar.loop();

  return 0;
}
