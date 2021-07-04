#pragma once

#include <SFML/Graphics.hpp>

#include "window/params.h"
#include <mutex>

#include "backend/plane.h"
#include "backend/telemetry.h"

#include "hasher/extension.h"


constexpr int l_size = 9;

class radarWindow {
  private:
    sf::RenderWindow window;
    sf::RenderWindow planeWindow;
    windowParams params;
    windowParams pparams;
    sf::Event event;
    sf::Font ft;

    std::vector<sf::Vertex> bounding_box;
    std::vector<sf::Text> labels;
    std::vector<sf::Text> guides;

    std::mutex &access;
    std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store;

    std::mutex &sat_access;
    sat_status &status;

    void drawBg(){
      window.clear(params.bg);
      planeWindow.clear(params.bg);
    }

    void handleKeypres(sf::Keyboard::Key key);
    void handleEvent(sf::RenderWindow &wind);
    void produceBoundingBox(size_t width, size_t box_lenght, size_t box_offset);

    sf::Text create_label(std::string txt, sf::Color color, int x, int y){
      sf::Text ret;
      ret.setFont(ft);
      ret.setString(txt);
      ret.setPosition(x,y);
      ret.setFillColor(color);
      ret.setCharacterSize(params.label_size);

      return ret;
    }

    sf::Text create_label(std::string txt, sf::Color color, int x, int y, size_t fsize){
      sf::Text ret;
      ret.setFont(ft);
      ret.setString(txt);
      ret.setPosition(x,y);
      ret.setFillColor(color);
      ret.setCharacterSize(fsize);

      return ret;
    }

    std::pair<int,int> translateCoords(long double v, long double h);

    void addLabel(std::string text, sf::Color c, int x, int y) {
      this->guides.push_back(create_label(text, c, x, y));
    }

    void drawCategory(std::string text, int x){
      addLabel(text, pparams.secondary, x, pparams.bar_offset);
    }

    sf::Text create_temp(float, int, int);
    sf::Text create_volt(float, int, int, int, int);
    sf::Text create_amp(float, int, int, int, int);

  public:
    radarWindow(windowParams g, windowParams pln, std::mutex &access, std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store, std::mutex &sat_access, sat_status &status);

    void addTriangle(size_t x, size_t y, sf::Color c = sf::Color::Blue){
      bounding_box.push_back( sf::Vertex(
          sf::Vector2f(x, y),
          c,
          sf::Vector2f(0,0)
        ));
    }

    void fillBuffers(){
      bounding_box.clear();
      produceBoundingBox(params.bar_width, params.bar_lenght, params.bar_offset);
    }

    void loop();

    ~radarWindow();
};
