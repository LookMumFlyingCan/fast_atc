#include <algorithm>

template <typename Container>
struct container_hash {
    std::size_t operator()(const Container &c) const {
      size_t hash = c.size();
      for(auto &i : c){
        hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      }

      return hash;
    }
};

template <typename Container>
struct container_comp {
    bool operator()(const Container &a, const Container &b) const {
      for(int i = 0; i < std::min(a.size(), b.size()); i++){
        if(a[i] == b[i])
          continue;

        return a[i] < b[i];
      }

      return false;
    }
};
