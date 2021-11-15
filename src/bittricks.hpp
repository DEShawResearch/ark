#ifndef ark_bittricks_hpp
#define ark_bittricks_hpp

#include <cstdint>

/*! \file ark/bittricks.hpp

  Ark uses some pretty dirty bit-tricks to make the memory
  footprint for atoms quite small.
*/

namespace Ark {

  /*! Defines various 64-bit constants use for masking bits. */
  namespace bitmasks {
    // pointer alignment tricks
    typedef uint64_t bits_t; //!< bits_t is a 64 bit quantity abrev.
    //! We take the bottom 3 bits.
    static const bits_t all      =UINT64_C(7);
    //! a none pointer has all high bits off.
    static const bits_t none     =0;
    //! a atom pointer has high bit set.
    static const bits_t atom     =UINT64_C(1);
    //! a vector pointer has bit 1<<62 set.
    static const bits_t vector   =UINT64_C(2);
    //! a table pointer has bit  1<<61 set.
    static const bits_t table    =UINT64_C(4);
    /* We don't have boxes anymore, but if we wanted
       a new variant, there is room for one more.
    //! box has all three high bits set.
    static const bits_t box      =UINT64_C(6);
    */

    /*! mask the bits.
      @param b bits to mask.
      @param a the and bits.
      @param o the or bits.
      @return (b & a) | o.
    */
    inline bits_t mask(bits_t b,bits_t a,bits_t o) {
      return (b & a) | o;
    }
  }

  /*! A string storage device optimized to take up
    minimal space for short strings.
  */
  struct atom_storage_t {
    //! variadic pointer: either bits or a char *.
    union ptr_t {
      bitmasks::bits_t bits; //!< see the data as bits.
      char *ptr; //!< see the data as a pointer to char.
    };
    ptr_t u; //!< member bits.

    //! if the ptr field is active deallocate the large string.
    void destroy();

    //! Copy string from a C-string.  Does not deallocate.  For use
    //! in constructors and initializers.
    //! @param c C-string.
    void init(const char *c);

    //! return a C-style string.
    //! @return a C-string.
    const char *c_str() const;

    //! Assign from a C-string.  Does deallocate.
    //! @param c C-string.
    void assign(const char *c) {
      atom_storage_t tmp;
      tmp.init(c);
      destroy();
      u.bits = tmp.u.bits;
    }
  };

}

#endif
