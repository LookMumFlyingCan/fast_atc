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
