struct plane {
  bool selected;
  std::optional<std::pair<byte, byte>> ident;
  std::optional<coordinates> position;
  std::optional<distance> altitude;
  std::optional<movement> velocity;
  std::optional<int> squawk;
  std::optional<std::string> callsign;
  std::optional<std::pair<std::vector<byte>, long long>> last_odd;
  std::optional<std::pair<std::vector<byte>, long long>> last_even;
};
