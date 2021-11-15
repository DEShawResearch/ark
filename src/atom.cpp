#include "atom.hpp"
#include <cstring>
#include <cstdlib>

void Ark::atom_storage_t::destroy() {
  u.bits = bitmasks::mask(u.bits,~bitmasks::all,0);
  free(u.ptr);
}
void Ark::atom_storage_t::init(const char *c) {
  u.ptr=strdup(c);
  u.bits = bitmasks::mask(u.bits,~bitmasks::all,bitmasks::atom);
}
const char *Ark::atom_storage_t::c_str() const {
  ptr_t p(u);
  p.bits = bitmasks::mask(p.bits,~bitmasks::all,0);
  return p.ptr;
}
