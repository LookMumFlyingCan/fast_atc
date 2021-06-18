#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <sstream>

#include <window/hardware.cpp>
#include <window/params.cpp>

constexpr int l_size = 8;

class radarWindow {
  private:
    sf::RenderWindow window;
    sf::RenderWindow planeWindow;
    windowParams params;
    sf::Event event;
    sf::Font ft;

    std::vector<sf::Vertex> bounding_box;
    std::vector<sf::Text> labels;
    std::vector<sf::Text> guides;

    std::mutex &access;
    std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store;

    void drawBg(){
      window.clear(params.bg);
      planeWindow.clear(params.bg);
    }

    void handleKeypres(sf::Keyboard::Key key){
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

    void handleEvent(sf::RenderWindow &wind){
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

    void produceBoundingBox(size_t width, size_t box_lenght, size_t box_offset) {
      for(int i = 1; i*box_lenght < (this->window.getSize().x - 2*box_offset); i++){
        if(!(i & 1))
          continue;

        addTriangle((i-1)*box_lenght + box_offset, box_offset);
        addTriangle((i-1)*box_lenght + box_offset, box_offset+width);
        addTriangle(i*box_lenght + box_offset, box_offset+width);

        addTriangle(i*box_lenght + box_offset, box_offset);
        addTriangle((i-1)*box_lenght + box_offset, box_offset);
        addTriangle(i*box_lenght + box_offset, box_offset+width);

        addTriangle((i-1)*box_lenght + box_offset, window.getSize().y - box_offset);
        addTriangle((i-1)*box_lenght + box_offset, window.getSize().y - box_offset - width);
        addTriangle(i*box_lenght + box_offset, window.getSize().y - box_offset - width);

        addTriangle(i*box_lenght + box_offset, window.getSize().y - box_offset);
        addTriangle((i-1)*box_lenght + box_offset, window.getSize().y - box_offset);
        addTriangle(i*box_lenght + box_offset, window.getSize().y - box_offset - width);
      }

      for(int i = 1; i*box_lenght < (this->window.getSize().y - 2*box_offset); i++){
        if(!(i & 1))
          continue;

        addTriangle(box_offset, (i-1)*box_lenght + box_offset);
        addTriangle(box_offset+width, (i-1)*box_lenght + box_offset);
        addTriangle(box_offset+width, i*box_lenght + box_offset);

        addTriangle(box_offset, i*box_lenght + box_offset);
        addTriangle(box_offset, (i-1)*box_lenght + box_offset);
        addTriangle(box_offset+width, i*box_lenght + box_offset);

        addTriangle(window.getSize().x - box_offset, (i-1)*box_lenght + box_offset);
        addTriangle(window.getSize().x - box_offset - width, (i-1)*box_lenght + box_offset);
        addTriangle(window.getSize().x - box_offset - width, i*box_lenght + box_offset);

        addTriangle(window.getSize().x - box_offset, i*box_lenght + box_offset);
        addTriangle(window.getSize().x - box_offset, (i-1)*box_lenght + box_offset);
        addTriangle(window.getSize().x - box_offset - width, i*box_lenght + box_offset);
      }

      this->labels.clear();

      for(int i = params.vstart; i <= params.vend; i++){
        int diff = i - params.vstart, vlen = (window.getSize().x - 2*box_offset - 2*params.shift_factor*params.scale_width)/(params.vend - params.vstart);

        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen - params.scale_width, 0, params.muted);
        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen + params.scale_width, 0, params.muted);
        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen - params.scale_width, box_offset + 5, params.muted);

        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen - params.scale_width, box_offset + 5, params.muted);
        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen + params.scale_width, 0, params.muted);
        addTriangle(box_offset + params.shift_factor*params.scale_width + diff*vlen + params.scale_width, box_offset + 5, params.muted);


        sf::Text res;
        res.setFont(ft);
        res.setString(std::to_wstring(i) + L"°");
        res.setCharacterSize(params.label_size);
        res.setFillColor(params.muted);
        res.setPosition( params.shift_factor*params.scale_width + box_offset + diff*vlen - params.text_offset, box_offset + 8);

        labels.push_back(res);
      }

      for(int i = params.hstart; i <= params.hend; i++){
        int diff = params.hend - i, hlen = (window.getSize().y - 2*box_offset - 2*params.shift_factor*params.scale_width)/(params.hend - params.hstart);

        addTriangle(0, box_offset + params.shift_factor*params.scale_width + diff*hlen - params.scale_width, params.muted);
        addTriangle(0, box_offset + params.shift_factor*params.scale_width + diff*hlen + params.scale_width, params.muted);
        addTriangle(box_offset + 5, box_offset + params.shift_factor*params.scale_width + diff*hlen - params.scale_width, params.muted);

        addTriangle(box_offset + 5, box_offset + params.shift_factor*params.scale_width + diff*hlen - params.scale_width, params.muted);
        addTriangle(0, box_offset + params.shift_factor*params.scale_width + diff*hlen + params.scale_width, params.muted);
        addTriangle(box_offset + 5, box_offset + params.shift_factor*params.scale_width + diff*hlen + params.scale_width, params.muted);


        sf::Text res;
        res.setFont(ft);
        res.setString(std::to_wstring(i) + L"°");
        res.setCharacterSize(params.label_size);
        res.setFillColor(params.muted);
        res.setPosition(box_offset + 8, params.shift_factor*params.scale_width + box_offset + diff*hlen - params.text_offset);

        labels.push_back(res);
      }
    }

    sf::Text create_label(std::string txt, sf::Color color, int x, int y){
      sf::Text ret;
      ret.setFont(ft);
      ret.setString(txt);
      ret.setPosition(x,y);
      ret.setFillColor(color);
      ret.setCharacterSize(params.label_size);

      return ret;
    }

    std::pair<int,int> translateCoords(long double v, long double h) {
      //std::cerr << params.bar_offset + params.shift_factor*params.scale_width << " + "  << (long double)(v - params.vstart) << " / " << std::fabs(params.vend - params.vstart) << " * " << (window.getSize().x - 2*params.bar_offset - 2*params.shift_factor*params.scale_width) << '\n';
      return std::make_pair(
            params.bar_offset + params.shift_factor*params.scale_width + ((((long double)(v - params.vstart))/(std::fabs(params.vend - params.vstart))) * (window.getSize().x - 2*params.bar_offset - 2*params.shift_factor*params.scale_width)),
            params.bar_offset + params.shift_factor*params.scale_width +

            ((((long double)(h - params.hstart))/std::fabs(params.hend - params.hstart))

            * (window.getSize().y - 2*params.bar_offset - 2*params.shift_factor*params.scale_width))
          );
    }

    void addLabel(std::string text, sf::Color c, int x, int y) {
      this->guides.push_back(create_label(text, c, x, y));
    }

  public:

    radarWindow(windowParams g, windowParams pln, std::mutex &access, std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store) : window(sf::VideoMode(g.wid, g.hei), g.title, sf::Style::Default, g.s), planeWindow(sf::VideoMode(pln.wid, pln.hei), pln.title, sf::Style::Default, pln.s), params(g), access(access), store(store) {
      setTransparency(window.getSystemHandle(), g.alpha);
      window.setFramerateLimit(g.frames);
      planeWindow.setFramerateLimit(pln.frames);

      if(!ft.loadFromFile("assets/font.ttf"))
          std::cerr << "shieeet\n";
    }

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

    void loop() {
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

        addLabel("ICAO", params.secondary, params.bar_offset, row);
        addLabel("CALL", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 1, row);
        addLabel("IDENT", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 2, row);
        addLabel("ALTI", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 3, row);
        addLabel("SPD", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 4, row);
        addLabel("AZIM", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 5, row);
        addLabel("V/R", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 6, row);
        addLabel("VSRC", params.secondary, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 7, row);

        row += guides[0].getCharacterSize();

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

          addLabel(icc.str(), data.selected ? params.selection : params.muted, params.bar_offset, row);
          addLabel(data.callsign ? *data.callsign : "INOP", params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 1, row);
          addLabel(data.ident ? (std::to_string((*data.ident).first) + " " + std::to_string((*data.ident).second)) : "INOP", params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 2, row);
          addLabel(data.altitude ? (std::to_string((*data.altitude).length) + "m") : "INOP", data.altitude ? ((*data.altitude).gnss ? params.tert : params.quadr) : params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 3, row);
          icc = std::stringstream();
          icc << std::fixed << std::setprecision(2) << (data.velocity ? (*data.velocity).velocity : 0);
          addLabel(data.velocity ? (icc.str() + "kt") : "INOP", data.velocity ? ((*data.velocity).gnss ? params.tert : params.quadr) : params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 4, row);
          icc = std::stringstream();
          icc << std::fixed << std::setprecision(2) << (data.velocity ? (*data.velocity).heading : 0);
          addLabel(data.velocity ? icc.str() : "INOP", params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 5, row);
          addLabel(data.velocity ? (std::to_string((*data.velocity).vertical_rate) + "ft/min") : "INOP", params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 6, row);
          addLabel(data.velocity ? ((*data.velocity).info == inertial ? "INERT" : ((*data.velocity).info == airspeed_ias ? "IAS" : "TAS")) : "INOP", params.tert, params.bar_offset + ((planeWindow.getSize().x - 2*params.bar_offset) / l_size) * 7, row);

          row += guides[0].getCharacterSize();
        }
        access.unlock();

        for(auto &t : this->guides)
          planeWindow.draw(t);


        window.display();
        planeWindow.display();
      }
    }

    ~radarWindow() {
      this->planeWindow.close();
      this->window.close();
    }
};
