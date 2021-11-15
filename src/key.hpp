#ifndef __ark_key_hpp
#define __ark_key_hpp

#include <string>

namespace Ark {

  /*! A "strong typedef" of a C++ string used as a key-value in an ark
    table.  The primary difference between key_t and std::string is
    that keys are immutable (much more limited functionality than a
    C++ string) and have a more limited range of values (originally,
    the usual alphanumeric-underscore convention for C symbols, but
    someone added '-' and recently ':') and this is checked in the
    constructor, with an exception thrown if violated.
  */
  class key_t {
    std::string _s;
    // key must be of the form [a-zA-Z0-9_][a-zA-Z0-9_:-]*
    void check_valid_key() const;

  public:
    static bool valid_key(std::string const& s);

    /*! Constructor.  Will throw.  Only exists to smooth over
      some hiccups with wisp. */
    key_t() { check_valid_key(); }

    /*! Constructor.  Checks validity of the string value.
      @param s C++-string.
    */
    key_t(const std::string &s) : _s(s) { check_valid_key(); }

    /*! Constructor.  Checks validity of the string value.
      @param s C-string.
    */
    key_t(const char *s) : _s(s) { check_valid_key(); }

    //! get a C++-string.
    //! @return a C++-string.
    const std::string &str() const { return _s; }

    //! get a C-string.
    //! @return a C-string.
    const char *c_str() const { return str().c_str(); }

    //! implicit conversion (to a const).
    operator const std::string&() const { return str(); }

    //! size of string.
    //! @return length of key as a string.
    unsigned size() const { return str().size(); }

    //! Comparison.
    //! @param k other key.
    bool operator< (const key_t &k) const {return str()< k.str();}
    //! Comparison.
    //! @param k other key.
    bool operator> (const key_t &k) const {return str()> k.str();}
    //! Comparison.
    //! @param k other key.
    bool operator==(const key_t &k) const {return str()==k.str();}
    //! Comparison.
    //! @param k other key.
    bool operator!=(const key_t &k) const {return str()!=k.str();}
    //! Comparison.
    //! @param k other key.
    bool operator<=(const key_t &k) const {return str()<=k.str();}
    //! Comparison.
    //! @param k other key.
    bool operator>=(const key_t &k) const {return str()>=k.str();}
  };

}

#endif
