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

#include <hasher/extension.cpp>
#include <backend/adsb.cpp>

#include <window/graphics.cpp>
#include <SFML/Graphics.hpp>

using namespace std;

std::mutex plane_access;
std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > planes;

sf::ContextSettings s { .antialiasingLevel = 5 };

void packets() {
  Socket sock(2008);

  sock.loop(plane_access, planes);
}

int main(int argc, char *argv[]){
  cerr << setprecision(20);

  std::thread(packets).detach();

  /*while(true) {
    plane_access.lock();
    for(auto &i : planes){
      std::cerr << "plane: " << std::hex << (int)i.first[0] << (int)i.first[1] << (int)i.first[2] << '\n';
      std::cerr << std::dec;
      std::cerr << " |-callsign: " << (i.second.callsign ? *i.second.callsign : "N/A") << '\n';
      std::cerr << " |-squawk: " << (i.second.squawk ? *i.second.squawk : -1) << '\n';
      std::cerr << " |-velocity: " << (i.second.velocity ? (*i.second.velocity).velocity : -1) << '\n';
      std::cerr << " |-heading: " << (i.second.velocity ? (*i.second.velocity).heading : -1) << '\n';
      std::cerr << " |-vertical: " << (i.second.velocity ? (*i.second.velocity).vertical_rate : -1) << '\n';
      std::cerr << " |-lon: " << (i.second.position ? (*i.second.position).lon : -1) << '\n';
      std::cerr << " |-lat: " << (i.second.position ? (*i.second.position).lat : -1) << '\n';
      std::cerr << " |-ident: " << (i.second.ident ? (*i.second.ident).first : -1) << " " << (i.second.ident ? (*i.second.ident).second : -1) << '\n';
    }
    plane_access.unlock();
    sleep(3);
  }*/


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
    .alpha = 235,
    .hstart = 51, .hend = 53,
    .vstart = 2, .vend = 4,
    .label_size = 11, .scale_width = 1,
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
    .alpha = 235,
    .hstart = 51, .hend = 53,
    .vstart = 2, .vend = 4,
    .label_size = 11, .scale_width = 1,
    .shift_factor = 25, .text_offset = 6,
    .title = L"Atc radar view"
  };

  //while(true) {}
  radarWindow radar(params, plnparams, plane_access, planes);

  radar.fillBuffers();

  radar.loop();

  return 0;
}
