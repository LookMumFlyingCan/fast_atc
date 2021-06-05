#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <iomanip>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <thread>

#include <unistd.h>

#include <hasher/extension.cpp>

#include <backend/adsb.cpp>

#include <window/graphics.cpp>
#include <SFML/Graphics.hpp>


using namespace std;

std::mutex plane_access;
std::unordered_map< std::vector<unsigned char>, plane, container_hash<std::vector<unsigned char>> > planes;

void packets() {
  Socket sock(2008);

  sock.loop(plane_access, planes);
}

int main(int argc, char *argv[]){
  cerr << setprecision(20);
  sf::ContextSettings s { .antialiasingLevel = 5 };

  std::thread net_handler(packets);

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

  windowParams params {
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

  radarWindow radar(params, plane_access, planes);

  radar.fillBuffers();

  radar.loop();

  return 0;
}
