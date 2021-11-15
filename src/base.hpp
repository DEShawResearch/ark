#ifndef __ark_base_hpp
#define __ark_base_hpp

#include "exception.hpp"
#include "kind.hpp"
#include "key.hpp"
#include "atom.hpp"

#include <vector>
#include <map>


/*! \file ark/base.hpp */

/*!
  Ark is a namespace containing the datastructure ark and its support
  types and operators.  These include a printer and parser class.  The
  primary data type to work with, however is Ark::ark which is a tree
  of strings.
*/
namespace Ark {
  //! Ark represents a variadic type with care taken to keep the memory
  //! footprint small.
  class ark;

  typedef std::vector<ark> vector_t;
  //!< vector_t is an STL vector of arks.

  typedef std::map<key_t,ark> table_t;
  //!< table_t is an STL map of key_t-ark pairs.

  /*! Think of an ark as a union of Ark::atom_t, Ark::vector_t, and
    Ark::table_t.  As such it has the topology of a tree, with vectors
    and tables being the internal nodes and atoms, and None as
    leaf nodes.

    There are 4 "get" members which are supplied as a convenience for
    extracting values from an ark.  However, these functions are
    probably misplaced, definitely could be implemented outside this
    class, and their use should be considered deprecated and poor
    taste.
  */
  class ark {
    typedef union {
      bitmasks::bits_t        bits;

      atom_storage_t          atom;
      vector_t    *vector;
      table_t     *table;
    } var_ptr_t;
    var_ptr_t u;

    var_ptr_t unmasked() const {
      var_ptr_t p(u);
      p.bits = bitmasks::mask(p.bits,~bitmasks::all,0);
      return p;
    }
    void mask(bitmasks::bits_t m) {
      u.bits = bitmasks::mask(u.bits,~bitmasks::all,m);
    }

#ifdef ANARKY_NOW
  public:
    struct annotation_t {
      std::string file;
      size_t      lineno;
      size_t      colno;
      annotation_t() : file("?"), lineno(0), colno(0) {}
      annotation_t(const std::string &f,size_t l,size_t c):
        file(f), lineno(l), colno(c) {}
    };
  private:
    annotation_t ano;
  public:
    ark &annotate(const annotation_t &an) { ano = an; return *this; }
    const annotation_t &annotation() const { return ano; }
#endif

  public:
    //! destructor.  Not virtual, and don't even think about it
    //! if you want to keep the memory footprint small.
    ~ark();

    //! get the kind of this ark.
    //! @return the kind_t value appropriate for this ark.
    kind_t kind() const {
      switch( bitmasks::mask(u.bits,bitmasks::all,0) ) {
      default:
      case bitmasks::none:         return None;
      case bitmasks::atom:         return Atom;
      case bitmasks::vector:       return Vector;
      case bitmasks::table:        return Table;
      }
    }
    //! explicit constructor given a kind.
    //! @param k the kind.
    explicit ark(kind_t k);

    //! default constructs to None.
    ark();

    //! construct from an atom_t.
    //! @param a atom.
    ark(const atom_t &a);
    //! construct from a C++string.
    //! @param str c++string
    ark(const std::string& str);
    //! construct from a C-string.
    //! @param str cstring
    ark(const char* str);

    //! construct from a vector_t.
    //! @param v vector.
    ark(const vector_t &v);

    //! construct from an table_t.
    //! @param t table.
    ark(const table_t &t);

    //! Copy.
    //! @param a another ark.
    ark(const ark &a);

    //! swap: swings pointers but does not destroy anything.
    //! @param a another ark.
    void swap(ark &a);

    /*! Assignment.
      @param a another ark.
      @return reference to this ark.
    */
    ark &operator=(const ark &a) {
      ark tmp(a); swap(tmp);
      return *this;
    }

    /*! Merge.  Add the contents of the given ark to this ark,
      merging table entries and otherwise transmuting as needed
      by the second ark.  To be specific (in pseudocode):
\verbatim
      if (kind()==b.kind() && kind()==Table)
         foreach k in keys of b
            table()[k].merge(b.table()[k])
      else
         *this = b;
\endverbatim
      @param b another ark.
      @return reference to this ark (after modification).
    */
    ark &merge(const ark &b);

    /*! transmute to a different type.
      If this ark is already of kind k, then nothing.  Else, all
      data previously held by this ark is destroyed and this ark
      begins its new life as an ark of kind k.
      @param k the new kind.
      @return reference to this (new) ark.
    */
    ark &be(kind_t k);

    //! the obvious.  equivalent to be(None).
    void clear() { be(None); }

    // note: these accessors produced undefined behavior if applied to
    // something of the wrong kind.  i.e.  they are NON-TRANSMUTING.
    // Write code so that appropriate checking is some or else wrap
    // them if you want to use them.

    //! access as atom (undefined behavior if wrong kind).
    //! @return reference to this ark as an atom.
    const atom_t &atom() const { return ((atom_t*)&u.atom)[0]; }
    //! non-const version of atom().
    atom_t &atom()  { return ((atom_t*)&u.atom)[0]; }

    //! access as vector (undefined behavior if wrong kind).
    //! @return reference to this ark as a vector.
    const vector_t &vector() const {return unmasked().vector[0];}
    //! non-const version of vector().
    vector_t &vector()  {return unmasked().vector[0];} 

    //! access as table (undefined behavior if wrong kind).
    //! @return reference to this ark as a table.
    const table_t &table() const { return unmasked().table[0]; }
    //! non-const version of table().
    table_t &table()  { return unmasked().table[0]; }

    //! return a atom_t* or NULL.
    //! @return pointer to the atom if this ark is an atom.
    const atom_t *get_atom() const;
    /*! return vector element or NULL.
      @param i index
      @return pointer to the i-th element if this ark is a vector with such
      a element.
    */
    const ark *get(unsigned i) const;
    /*! return table element or NULL.
      @param k key
      @return pointer to the value of k if this ark is a table with such
      an element.
    */
    const ark *get(const key_t &k) const;

    /*! xget does get queries using an extended syntax (super-keys),
      which look like "key1.key2[index1][index2].key3".
      @param s superkey.
      @return ark element under this key if it exists else NULL.
    */
    const ark *xget(const std::string &s) const;
  };

  // **basic i/o**
  /*! parse the input stream and return the ark defined.
    This parsing is very strict (no includes, overrides, or superkeys);
    more advanced parsing modes are available via Ark::parser.
    @param in input stream.
    @return the parsed ark.
  */
  ark parse(std::istream &in);
  /*! parse from string.
    Just the istream version with a sstream adapter.
    @param s input string.
    @return the parsed ark.
  */
  ark parse(const std::string &s);
}

/*! basic ark I/O: output.  No pretty printing, fully delimited.
  more advanced printing modes are available via Ark::printer.
  @param o output stream.
  @param a an ark.
  @return reference to o.
*/
std::ostream &operator<<(std::ostream &o,const Ark::ark &a);

/*! basic ark I/O: input.  Strictly parses the input stream using
  Ark::parse().
  @param i input stream.
  @param a an ark.
  @return reference to i.
*/
std::istream &operator>>(std::istream &i,Ark::ark &a);

#endif
