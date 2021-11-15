#include "key.hpp"
#include "exception.hpp"

void Ark::key_t::check_valid_key() const {
    auto k = str();
    if (!valid_key(k)) {
        throw exception(std::string("malformed key: ")+k);
    }
}

bool Ark::key_t::valid_key(std::string const& k) {
  bool good=true;
  good = good && (k.size()>0);
  good = good && (isalpha(k[0]) || k[0]=='_' || k[0]==':');
  for(unsigned i=1; i < k.size(); ++i) {
    good = good &&
      (isalpha(k[i]) || k[i]=='_' || k[i]==':' || isdigit(k[i]) || k[i]=='-');
  }
  return good;
}

