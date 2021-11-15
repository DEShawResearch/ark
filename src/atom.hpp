#ifndef __ark_atom_hpp
#define __ark_atom_hpp

#include "bittricks.hpp"
#include <string>

/*! \file ark/atom.hpp */

namespace Ark {
  /*! An atom_t is a very lightweight string storage unit.
    It mostly has construction and conversion operations which allow
    it to convert between C and C++ strings, while maintaining an
    aggressively small memory footprint.
  */
  class atom_t : private atom_storage_t {
  public:
    //! deallocates the atom_storage_t parent.
    ~atom_t() {destroy();}

    //! defaults to empty string.
    atom_t() {init("");}

    //! copies into the atom_storage_t.
    //! @param a an atom.
    atom_t(const atom_t &a) {init(a.c_str());}

    //! copies into the atom_storage_t.
    //! @param a a C++ string.
    atom_t(const std::string a) {init(a.c_str());}

    //! copies into the atom_storage_t.
    //! @param a a C string.
    atom_t(const char *a) {init(a);}

    //! get a C-string.
    //! @return a pointer to a fixed C string (don't decallocate).
    const char *c_str() const {return atom_storage_t::c_str();}

    //! get a C++-string.
    //! @return a C++-string version of the atom.
    std::string str() const {return std::string(c_str()); }

    //! assignment.
    //! @param a an atom. Yes, a = a should work.
    //! @return reference to this atom_t.
    atom_t &operator=(const atom_t &a) { assign(a.c_str()); return *this; }

    //! allow implicit coercion to a C++-string.
    operator std::string() const { return str(); }
  };
}
#endif
