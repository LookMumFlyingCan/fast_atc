#include <iostream>
#include "window/window.h"

void radarWindow::produceBoundingBox(size_t width, size_t box_lenght, size_t box_offset) {
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
