#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include "window/window.h"

#include <window/hardware.cpp>

std::pair<int,int> radarWindow::translateCoords(long double v, long double h) {
  //std::cerr << params.bar_offset + params.shift_factor*params.scale_width << " + "  << (long double)(v - params.vstart) << " / " << std::fabs(params.vend - params.vstart) << " * " << (window.getSize().x - 2*params.bar_offset - 2*params.shift_factor*params.scale_width) << '\n';
  return std::make_pair(
        params.bar_offset + params.shift_factor*params.scale_width + ((((long double)(v - params.vstart))/(std::fabs(params.vend - params.vstart))) * (window.getSize().x - 2*params.bar_offset - 2*params.shift_factor*params.scale_width)),
        params.bar_offset + params.shift_factor*params.scale_width +

        ((((long double)(h - params.hstart))/std::fabs(params.hend - params.hstart))

        * (window.getSize().y - 2*params.bar_offset - 2*params.shift_factor*params.scale_width))
      );
}

radarWindow::radarWindow(windowParams g, windowParams pln, std::mutex &access, std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store, std::mutex &sat_access, sat_status &status) : window(sf::VideoMode(g.wid, g.hei), g.title, sf::Style::Default, g.s), planeWindow(sf::VideoMode(pln.wid, pln.hei), pln.title, sf::Style::Default, pln.s), params(g), access(access), store(store), pparams(pln), sat_access(sat_access), status(status) {
  setTransparency(window.getSystemHandle(), g.alpha);
  window.setFramerateLimit(g.frames);
  planeWindow.setFramerateLimit(pln.frames);

  if(!ft.loadFromFile("assets/font.ttf"))
      std::cerr << "shieeet\n";
}

sf::Text radarWindow::create_temp(float temp, int last_bound, int y) {
  std::stringstream icc;
  icc << std::setfill('0') << std::fixed << std::setw(3) << std::setprecision(1) << temp << "C";
  return create_label(icc.str(), temp < 35 ? params.secondary : (temp > 60 ? params.selection : params.tert), params.bar_offset - 5 + last_bound, y, 11);
}

sf::Text radarWindow::create_volt(float temp, int last_bound, int y, int low, int mid) {
  std::stringstream icc;
  icc << std::setfill('0') << std::fixed << std::setw(1) << std::setprecision(2) << temp << "V\n---";
  return create_label(icc.str(), temp < low ? params.secondary : (temp > mid ? params.selection : params.tert), params.bar_offset - 5 + last_bound, y, 11);
}

sf::Text radarWindow::create_amp(float temp, int last_bound, int y, int low, int mid) {
  std::stringstream icc;
  icc << std::setfill('0') << std::fixed << std::setw(3) << std::setprecision(1) << temp << "A";
  return create_label(icc.str(), temp < low ? params.secondary : (temp > mid ? params.selection : params.tert), params.bar_offset - 5 + last_bound, y, 11);
}


radarWindow::~radarWindow() {
  this->planeWindow.close();
  this->window.close();
}
