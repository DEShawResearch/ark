#include "kind.hpp"

std::string Ark::toString(const kind_t &k) {
  std::string ret;
  switch(k) {
  case None: ret="Ark::None"; break;
  case Atom: ret="Ark::Atom"; break;
  case Vector: ret="Ark::Vector"; break;
  case Table: ret="Ark::Table"; break;
  }
  return ret;
}

