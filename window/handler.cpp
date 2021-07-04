#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

#include "window/window.h"

void radarWindow::handleKeypres(sf::Keyboard::Key key){
  switch(key){
    case sf::Keyboard::Key::Q:
      window.close();
      break;
    case sf::Keyboard::Key::W:
      params.hstart++;
      params.hend++;
      break;
    case sf::Keyboard::Key::S:
      params.hstart--;
      params.hend--;
      break;
    case sf::Keyboard::Key::A:
      params.vstart--;
      params.vend--;
      break;
    case sf::Keyboard::Key::D:
      params.vstart++;
      params.vend++;
      break;
    case sf::Keyboard::Key::Add:
      params.vend++;
      break;
    case sf::Keyboard::Key::Subtract:
        if(params.vend == params.vstart + 1)
          break;

        params.vend--;
        break;
        case sf::Keyboard::Key::Multiply:
          params.hend++;
          break;
        case sf::Keyboard::Key::Divide:
            if(params.hend == params.hstart + 1)
              break;

            params.hend--;
            break;
    case sf::Keyboard::Key::Down: {
        access.lock();

        if(store.size() == 0){
          access.unlock();
          break;
        }

        bool swck = false;
        bool fnd = true;
        for(auto i = store.begin(); i != store.end(); i++){
          if(swck){
            i->second.selected = true;
            swck = false;
            break;
          }


          if(i->second.selected) {
            i->second.selected = false;
            swck = true;
            fnd = false;
          }
        }

        if(swck)
          store.begin()->second.selected = true;

        if(fnd)
          store.begin()->second.selected = true;

        access.unlock();

        break;
      }
    case sf::Keyboard::Key::Up: {
          access.lock();

          if(store.size() == 0){
            access.unlock();
            break;
          }

          bool swck = false;
          bool fnd = true;
          auto i = store.end();
          do {
            i--;
            if(swck){
              i->second.selected = true;
              swck = false;
              break;
            }


            if(i->second.selected) {
              i->second.selected = false;
              swck = true;
              fnd = false;
            }
          } while (i != store.begin());

          if(swck)
            (--store.end())->second.selected = true;

          if(fnd)
            (--store.end())->second.selected = true;

          access.unlock();

          break;
        }
  }

  fillBuffers();
}

void radarWindow::handleEvent(sf::RenderWindow &wind){
  if(event.type == sf::Event::Closed)
    window.close();

  if(event.type == sf::Event::KeyPressed)
    handleKeypres(event.key.code);

  if(event.type == sf::Event::Resized) {
    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
    wind.setView(sf::View(visibleArea));

    fillBuffers();
  }
}
