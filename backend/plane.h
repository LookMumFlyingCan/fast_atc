#pragma once

#include "backend/properties.h"
#include "hasher/extension.h"

typedef unsigned char byte;

struct plane {
  bool selected;
  std::optional<std::pair<byte, byte>> ident;
  std::optional<coordinates> position;
  std::optional<distance> altitude;
  std::optional<movement> velocity;
  std::optional<int> squawk;
  std::optional<std::string> callsign;
  std::chrono::time_point<std::chrono::system_clock> last_pkg;
  std::optional<std::pair<std::vector<byte>, long long>> last_odd;
  std::optional<std::pair<std::vector<byte>, long long>> last_even;
};
