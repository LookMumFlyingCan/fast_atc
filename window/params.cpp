struct windowParams {
  size_t bar_width, bar_offset, bar_lenght, hei, wid;
  sf::Color bg;
  sf::Color muted;
  sf::ContextSettings s;
  unsigned char alpha;
  int hstart, hend, vstart, vend;
  size_t label_size, scale_width, shift_factor, text_offset;
  std::wstring title;
};