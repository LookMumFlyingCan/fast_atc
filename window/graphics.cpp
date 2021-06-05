#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

#include <window/hardware.cpp>
#include <window/params.cpp>

class radarWindow {
  private:
    sf::RenderWindow window;
    windowParams params;
    sf::Event event;
    sf::Font ft;

    std::vector<sf::Vertex> bounding_box;
    std::vector<sf::Text> labels;

    std::mutex &access;
    std::unordered_map<std::vector<byte>, plane, container_hash<std::vector<unsigned byte>> > &store;

    void drawBg(){
      window.clear(params.bg);
    }

    void handleKeypres(sf::Keyboard::Key key){
      switch(key){
        case sf::Keyboard::Key::Q:
          window.close();
          break;
        case sf::Keyboard::Key::Up:
          params.hstart++;
          params.hend++;
          break;
        case sf::Keyboard::Key::Down:
          params.hstart--;
          params.hend--;
          break;
        case sf::Keyboard::Key::Left:
          params.vstart--;
          params.vend--;
          break;
        case sf::Keyboard::Key::Right:
          params.vstart++;
          params.vend++;
          break;
      }

      fillBuffers();
    }

    void handleEvent(){
      if(event.type == sf::Event::Closed)
        window.close();

      if(event.type == sf::Event::KeyPressed)
        handleKeypres(event.key.code);

      if(event.type == sf::Event::Resized) {
        sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
        window.setView(sf::View(visibleArea));

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

    std::pair<int,int> translateCoords(long double v, long double h) {
      return std::make_pair(
            params.bar_offset + params.shift_factor*params.scale_width + (v - params.vstart)/(abs(params.vend - params.vstart)) * (window.getSize().x - 2*params.bar_offset - 2*params.shift_factor*params.scale_width),
            params.bar_offset + params.shift_factor*params.scale_width + (((h - params.hstart)/(abs(params.hend - params.hstart)) * (window.getSize().y - 2*params.bar_offset - 2*params.shift_factor*params.scale_width)))
          );
    }

  public:

    radarWindow(windowParams g, std::mutex &access, std::unordered_map<std::vector<unsigned char>, plane, container_hash<std::vector<unsigned char>> > &store) : window(sf::VideoMode(g.wid, g.hei), g.title, sf::Style::Default, g.s), params(g), access(access), store(store) {
      setTransparency(window.getSystemHandle(), g.alpha);

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
      while(window.isOpen()){
        while(window.pollEvent(event)){
          handleEvent();
        }

        drawBg();
        window.draw(&bounding_box[0], bounding_box.size(), sf::Triangles);

        for(auto &t : this->labels)
          window.draw(t);

        access.lock();
        for(auto &plane : store){
          if(plane.second.position){
            sf::CircleShape circ;

            auto coords = translateCoords((*plane.second.position).lon, (*plane.second.position).lat);

            std::cerr << coords.first << ' ' << coords.second << '\n';
            circ.setPosition(coords.first, coords.second);
            circ.setFillColor(sf::Color::Red);
            circ.setRadius(4);

            window.draw(circ);
            if(plane.second.velocity){
              sf::RectangleShape rect(sf::Vector2f(10, 2));
              std::cerr <<(*plane.second.velocity).heading << '\n';
              rect.rotate((*plane.second.velocity).heading);
              rect.setPosition(coords.first+8, coords.second+4);

              window.draw(rect);
            }
          }
        }
        access.unlock();

        window.display();
      }
    }

    ~radarWindow() {
      this->window.close();
    }
};
