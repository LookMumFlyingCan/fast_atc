#include <iostream>
#include <optional>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "window/window.h"

void radarWindow::loop() {
  while(window.isOpen() && planeWindow.isOpen()){

    while(planeWindow.pollEvent(event)){
      handleEvent(planeWindow);
    }

    while(window.pollEvent(event)){
      handleEvent(window);
    }


    drawBg();
    window.draw(&bounding_box[0], bounding_box.size(), sf::Triangles);

    for(auto &t : this->labels)
      window.draw(t);

    this->guides.clear();

    size_t row = params.bar_offset;

    drawCategory("ICAO", pparams.bar_offset);
    drawCategory("CALL", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 1);
    drawCategory("IDENT", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 2);
    drawCategory("ALTI", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 3);
    drawCategory("SPD", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 4);
    drawCategory("AZIM", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 5);
    drawCategory("V/R", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 6);
    drawCategory("VSRC", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 7);
    drawCategory("TTPK", pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 8);

    row += guides[0].getCharacterSize();

    std::optional<std::chrono::time_point<std::chrono::system_clock>> latest_pkg;

	std::vector<std::vector<unsigned char>> to_del;

    access.lock();
    for(auto &plane : store){
      if(plane.second.position){
        //std::cerr << "lon: " << (*plane.second.position).lon << " lat: " << (*plane.second.position).lat << '\n';
        auto coords = translateCoords((*plane.second.position).lon, (*plane.second.position).lat);

        if(plane.second.velocity){
          sf::RectangleShape rect(sf::Vector2f(15, 2));
          rect.setOrigin(1.f, -0.5f);
          rect.rotate(-90.f + (*plane.second.velocity).heading);
          rect.setPosition(coords.first, coords.second);

          window.draw(rect);
        }

        sf::CircleShape circ;

        //std::cerr << coords.first << ' ' << coords.second << '\n';
        circ.setOrigin(2.f, 2.f);
        circ.setPosition(coords.first, coords.second);
        circ.setFillColor(plane.second.selected ? sf::Color::Red : sf::Color::Blue);
        circ.setRadius(4.f);

        window.draw(circ);
      }

      std::stringstream icc;
      icc << std::hex << (int)plane.first[0] << (int)plane.first[1] << (int)plane.first[2];

      auto data = plane.second;

      addLabel(icc.str(), data.selected ? pparams.selection : pparams.muted, pparams.bar_offset, row);
      addLabel(data.callsign ? *data.callsign : "INOP", pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 1, row);
      addLabel(data.ident ? (std::to_string((*data.ident).first) + " " + std::to_string((*data.ident).second)) : "INOP", pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 2, row);
      addLabel(data.altitude ? (std::to_string((*data.altitude).length) + "ft") : "INOP", data.altitude ? ((*data.altitude).gnss ? pparams.tert : pparams.quadr) : pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 3, row);
      icc = std::stringstream();
      icc << std::fixed << std::setprecision(2) << (data.velocity ? (*data.velocity).velocity : 0);
      addLabel(data.velocity ? (icc.str() + "kt") : "INOP", data.velocity ? ((*data.velocity).gnss ? pparams.tert : pparams.quadr) : pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 4, row);
      icc = std::stringstream();
      icc << std::fixed << std::setprecision(2) << (data.velocity ? (*data.velocity).heading : 0);
      addLabel(data.velocity ? icc.str() : "INOP", pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 5, row);
      addLabel(data.velocity ? (std::to_string((*data.velocity).vertical_rate) + "ft/min") : "INOP", pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 6, row);
      addLabel(data.velocity ? ((*data.velocity).info == inertial ? "INERT" : ((*data.velocity).info == airspeed_ias ? "IAS" : "TAS")) : "INOP", pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 7, row);
      addLabel(std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - data.last_pkg).count()), pparams.tert, pparams.bar_offset + ((planeWindow.getSize().x - 2*pparams.bar_offset) / l_size) * 8, row);

      row += guides[0].getCharacterSize();

      if(!latest_pkg)
        latest_pkg = data.last_pkg;
      else
        latest_pkg = std::max(*latest_pkg, data.last_pkg);

	  if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - data.last_pkg).count() > 50){
		  to_del.push_back(plane.first);
	  }
    }
    access.unlock();

	for(auto &x : to_del)
		store.erase(x);

    sat_access.lock();
    auto coords = translateCoords(status.pos.longitude, status.pos.latitude);
    sat_access.unlock();

    sf::CircleShape circ;

    circ.setOrigin(2.f, 2.f);
    circ.setPosition(coords.first, coords.second);
    circ.setFillColor(params.tert);
    circ.setRadius(4.f);

    window.draw(circ);

    for(auto &t : this->guides)
      planeWindow.draw(t);



    window.draw(create_label(latest_pkg ?
      std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - *latest_pkg).count()
      )
      :
      "PKGN"
    , latest_pkg ? params.secondary : params.tert, window.getSize().x - (3*params.bar_offset), 3*params.bar_offset));
    


    sf::Text north = create_label("N", params.secondary, (3*params.bar_offset), 3*params.bar_offset, 14);
    window.draw(north);

    std::stringstream icc;
    sat_access.lock();
    icc << std::dec << std::setfill('0') << std::setw(2) << (int)status.pos.timeHour << ':' << std::setw(2) << (int)status.pos.timeMin << ':' << std::setw(2) << (int)status.pos.timeSec << 'Z';
    sat_access.unlock();
    sf::Text gps_time = create_label(icc.str(), params.tert, 3*params.bar_offset, window.getSize().y - 3*params.bar_offset, 14);
    window.draw(gps_time);

    sat_access.lock();
    sf::Text pcbt = create_temp(status.tel.tempPCB, gps_time.getGlobalBounds().left + gps_time.getGlobalBounds().width, gps_time.getGlobalBounds().top);
    sf::Text pcbtt = create_label("PCB", params.tert, pcbt.getGlobalBounds().left, pcbt.getGlobalBounds().top - 15, 11);
    sf::Text sdrt = create_temp(status.tel.tempSDR, pcbt.getGlobalBounds().left + pcbt.getGlobalBounds().width, gps_time.getGlobalBounds().top);
    sf::Text sdrtt = create_label("SDR", params.tert, sdrt.getGlobalBounds().left, sdrt.getGlobalBounds().top - 15, 11);
    sf::Text env1t = create_temp(status.tel.tempENV1, sdrt.getGlobalBounds().left + sdrt.getGlobalBounds().width, gps_time.getGlobalBounds().top);
    sf::Text env1tt = create_label("ENV", params.tert, env1t.getGlobalBounds().left, env1t.getGlobalBounds().top - 15, 11);
    sf::Text env2t = create_temp(status.tel.tempENV2, env1t.getGlobalBounds().left + env1t.getGlobalBounds().width, gps_time.getGlobalBounds().top);
    sf::Text env2tt = create_label("ENV", params.tert, env2t.getGlobalBounds().left, env2t.getGlobalBounds().top - 15, 11);
    sf::Text rpit = create_temp(status.tel.tempRPI, env2t.getGlobalBounds().left + env2t.getGlobalBounds().width, gps_time.getGlobalBounds().top);
    sf::Text rpitt = create_label("RPI", params.tert, rpit.getGlobalBounds().left, rpit.getGlobalBounds().top - 15, 11);
    sat_access.unlock();
    window.draw(pcbt);
    window.draw(sdrt);
    window.draw(env1t);
    window.draw(env2t);
    window.draw(rpit);
    window.draw(pcbtt);
    window.draw(sdrtt);
    window.draw(env1tt);
    window.draw(env2tt);
    window.draw(rpitt);

    sat_access.lock();
    sf::Text vmain = create_volt(status.tel.psu_V5V, north.getGlobalBounds().left + north.getGlobalBounds().width + 20, north.getGlobalBounds().top - 15, 4.8, 5.1);
    sf::Text vlow = create_volt(status.tel.psu_V3V3, vmain.getGlobalBounds().left + vmain.getGlobalBounds().width, vmain.getGlobalBounds().top - 3, 3.2, 3.4);
    sf::Text vbatt = create_volt(status.tel.psu_Vbat, vlow.getGlobalBounds().left + vlow.getGlobalBounds().width, vmain.getGlobalBounds().top - 3, 7, 7.6);
    sf::Text amain = create_amp(status.tel.psu_A5V, north.getGlobalBounds().left + north.getGlobalBounds().width + 20, vmain.getGlobalBounds().top + 20, 0.3, 0.7);
    sf::Text alow = create_amp(status.tel.psu_A3V3, vmain.getGlobalBounds().left + vmain.getGlobalBounds().width, vlow.getGlobalBounds().top + 20, 0.1, 0.3);
    sf::Text abatt = create_amp(status.tel.psu_Abat, vlow.getGlobalBounds().left + vlow.getGlobalBounds().width, vbatt.getGlobalBounds().top + 20, 0.4, 0.9);
    sat_access.unlock();
    window.draw(vmain);
    window.draw(vlow);
    window.draw(vbatt);
    window.draw(amain);
    window.draw(alow);
    window.draw(abatt);

    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, sf::Vector2f(north.getGlobalBounds().left, north.getGlobalBounds().top));
    arrow.setPoint(1, sf::Vector2f(north.getGlobalBounds().left + north.getGlobalBounds().width, north.getGlobalBounds().top));
    arrow.setPoint(2, sf::Vector2f(north.getGlobalBounds().left + (north.getGlobalBounds().width)/2, north.getGlobalBounds().top - 10));
    arrow.setFillColor(params.secondary);

    window.draw(arrow);

    window.display();
    planeWindow.display();
  }
}
