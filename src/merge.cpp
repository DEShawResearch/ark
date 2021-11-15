#include "base.hpp"

namespace Ark {

  ark &ark::merge(const ark &b) {
    if (kind()==b.kind() && kind()==Table) {
      table_t::const_iterator i=b.table().begin(),e=b.table().end();
      for ( ; i!= e; ++i) table()[i->first].merge(i->second);
    }
    else *this = b;
    return *this;
  }

}
