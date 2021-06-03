#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <iomanip>

#include <window/graphics.cpp>
#include <SFML/Graphics.hpp>

#include <backend/adsb.cpp>

using namespace std;

int main(int argc, char *argv[]){
  cerr << setprecision(20);
  sf::ContextSettings s { .antialiasingLevel = 5 };

  Socket feh(2008);
  feh.loop();

  /*windowParams params {
    .bar_width = 2,
    .bar_offset = 20,
    .bar_lenght = 10,
    .hei = 400, .wid = 400,
    .bg = sf::Color(0,0,0),
    .muted = sf::Color(120,120,120),
    .s = s,
    .alpha = 235,
    .hstart = 49, .hend = 51,
    .vstart = 19, .vend = 21,
    .label_size = 11, .scale_width = 1,
    .shift_factor = 25, .text_offset = 6,
    .title = L"Atc radar view"
  };

  radarWindow radar(params);

  radar.fillBuffers();

  radar.loop();*/

  return 0;
}
