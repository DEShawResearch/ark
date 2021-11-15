#ifndef __ark_kind_hpp
#define __ark_kind_hpp

#include <string>
#include <iostream>

/*! \file ark/kind.hpp */

namespace Ark {
  /*! Things an ark might be.
  */
  enum kind_t {
    None=0, //!< None a singular or uninitialized value.
    Atom,   //!< Atom an atom_t value.
    Vector, //!< Vector a vector_t value.
    Table //!< Table a table_t value.
  };
  /* It is useful to be able to get kind's out as strings. */
  std::string toString(const kind_t &k);

}

/*! output operator, calls toString.
  @param s output stream
  @param k kind type
*/
inline
std::ostream& operator<<(std::ostream& s,const Ark::kind_t& k) {
  return s << toString(k);
}


#endif
