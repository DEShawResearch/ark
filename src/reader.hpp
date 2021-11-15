#ifndef __ark_reader_hpp
#define __ark_reader_hpp

#include "base.hpp"
#include "exception.hpp"

#include <vector>
#include <list>
#include <set>

/*! \file reader.hpp

Ark::reader adds stuff to namespace Ark which supports typesafe
and user-friendly traversal and extraction of data from an Ark::ark.

These functions take the form, primarily, of Ark::from() and Ark::from_opt()
(traversal functions) to be used in conjunction with Ark::to() (extraction
function).  For complicated traversals, to customize the behavior of the
Ark::to() functions, visitors are used and appropriate base classes for
two forms of visitation are defined.  These base-visitors are in no-op form,
so until you override their methods they do nothing.
  
The Ark::to() functions defined here makes use of the visitors and will
provide some example to developers.
*/

//! Arkreader adds stuff to Ark.
namespace Ark {
  
  /*! reader is just a better version of ark's original reader_ptr which
    allows for a variety of exception-checked inter-conversions.
    reader also quietly turns Ark::None into a NULL for all intents
    and purposes.  Many of the get and set functions use reader, but
    the conversions are all implicit.
  */
  class reader; // Forward declarations

  // helper routines
  namespace details {
    void rethrow_failed_get(std::exception &e,const char *s);
    void throw_more_elements_than_expected(size_t s,size_t mx);
    void throw_fewer_elements_than_expected(size_t s,size_t mn);
    struct invoke_reader_constructor {
      const reader & r;
      invoke_reader_constructor(const Ark::reader &_r) : r(_r) {}
      operator const reader & () { return r; }
    };
  }
}

/*!
  According to overloading rules and subsequent definitions, this
  means that the extraction will attempt to use:
  \arg member function T::operator^=() (if defined).
  \arg the global operator^=().
  \arg or r = T(a) which will first look for s constructor and then
       will look for a specialization of reader::operator T ().

  Thus, users can specialize behavior for their favorite type T
  in a number of ways (in order of recommendation):
  \arg write a constructor taking only an ark
  \arg explicitly specialize reader::operator T () (have to do this for enums).
  \arg define an member function T::operator^= (for classes which
       don't have a good operator=(), for example).
  \arg define an specialization of the global operator^=(T &,reader)
       (a non-invasive and non-copying approach).
*/
template <typename T>
inline void operator^=(T &,const Ark::reader &);

namespace Ark {

  class reader {
    const ark *            _current; /* current ark.  Could be NULL, which
                                        indicates a bad search result.  If
                                        not NULL it will not be None. */
    std::list<const table_t *> _scope; /* a list of non-NULL table pointers
                                          which function as scopes for bounced
                                          searches.  */
    std::string            _history; /* record of all searching done on this
                                        reader or its copies. */

    /* Gives *_current.  The deref is checked for NULL, so it fails
     with an exception if _current is NULL.
    */
    const ark & top() const;

    void descend(const size_t i);
    void descend(const key_t &k);
    void bounce (const key_t &k);
    void follow (const std::string &s);

    /* Internally useful, but they need to be kept private (see operator
       conversions below. */
    const table_t & table() const;
    const vector_t & vector() const;
    const atom_t & atom() const;

  public:
    // default constructor is ok.  Just creates a reader without scopes
    // and with a bad search.
    reader() : _current(NULL),_scope(),_history(),_logger(NULL) {}

    //! explicit constructor from an ark.  Not sure we need.
    //! @param a pointer
    //! Equivalent to the default constructor if the given ark is None.
    explicit reader(const ark &a);
    
    //! do we have any scopes?
    bool found() const { return _current; }
    bool lost()  const { return !found(); }

    //! kind access
    bool isVector() const { return top().kind() == Vector; }
    //! kind access
    bool isTable() const { return top().kind() == Table; }
    //! kind access
    bool isAtom() const { return top().kind() == Atom; }

    //! Explicit value access.
    std::string str() const {
      std::string s = atom().str(); 
      if (_logger) _logger->log(history(),s);
      return s;
    }

    //! Returns 'lookup history: ' prefixed before history()
    //! @return a canonical string to use for reporting the key history.
    std::string report() const;

    // return query history
    const std::string &history() const { return _history; }

    //! Table access and reader lookup
    //! @param s a key-string of the form (.?KEY|[INT])*
    //! @return the result of a scoped version of ark::xget().  The result
    //! may be lost() if the search has a key lookup error along the way.
    reader get(const std::string &s) const;

    //! strictly descend off the current vector (error if not a vector)
    //! via the index (which must be in range).
    //! @param i the index to look up
    //! @return the reader under that index.
    reader getVec(const size_t i) const {
      reader ret(*this);
      ret.descend(i);
      return ret;
    }

    //! returns the size of the current vector (error if not a vector)
    size_t sizeVec() const { return vector().size(); }

    //! Wrapper around the common operation of checking if an ark
    //! is valid (it could be invalid from a failed get_opt) and then
    //! setting a value if it is valid.
    //! @param t reference of value to set
    template <typename T>
    void set(      T &t) const { t ^= *this; }

    //! Having this variant allows magic proxy setters to be used
    //! @param t const reference of a proxy setter ("set_as_...")
    template <typename T>
    void set(const T &t) const { t ^= *this; }

    //! This variant does a goodness check before the assignment, doesn't
    //! set if lost().  Return value indicates whether the set was done.
    template <typename T>
    bool set_opt(      T &t) const {
      bool ret = found();
      if (ret) set(t);
      else if (_logger) _logger->log_opt(history());
      return ret;
    }

    //! This variant does a goodness check before the assignment, doesn't
    //! set if lost().  Return value indicates whether the set was done.
    template <typename T>
    bool set_opt(const T &t) const {
      bool ret = found();
      if (ret) set(t);
      else if (_logger) _logger->log_opt(history());
      return ret;
    }

    //! Hopefully these won't get abused
    operator const ark & () const { return top(); }
    operator const table_t & () const { return table(); }
    operator const vector_t & () const { return vector(); }
    operator const atom_t & () const { return atom(); }

    //! A hook class to allow partial specialization.
    //! This doesn't need to be used if one is just making
    //! a conversion of a specific type.  In that case,
    //! just specialize reader::operator T () (see below).
    template <typename T>
    struct operator_T {
      static T value(const reader &r) {
        return T(details::invoke_reader_constructor(r));
      }
    };

    //! Let an ark turn into anything
    //! Users are encouraged to overload the crap out of this.
    template <typename T>
    operator T () const { return operator_T<T>::value(*this); }

    // exception thrown by get
    struct badsearch : public exception {
      explicit badsearch(const std::string& s) : exception(s) {}
      explicit badsearch(const exception& e) : exception(e) {}
    };

    // class for logging callback
    struct logger {
      virtual ~logger() {}
      virtual void log(const std::string &hist,const std::string &val) = 0;
      virtual void log_opt(const std::string &hist) = 0;
    };
    // yes, public.  People may need to set and reset this thing.
    logger    *            _logger;  
  };

}

//! template operator^=() function.  Used to provide one of many means
//! of extracting data into a user-defined type.  The unspecialized
//! form assigns to t from a.  @param t reference to the destination
//! variable
//! @param t something we are weakly asigning to.
//! @param a ark reference.
template <typename T>
inline void operator^=(T &t,const Ark::reader &a) {
  t = a.operator T();
}

namespace Ark {
  //! the const & operator isn't enough.
  template <> inline reader::operator ark () const { return top(); }
  //! the const & operator isn't enough.
  template <> inline reader::operator table_t () const { return table(); }
  //! the const & operator isn't enough.
  template <> inline reader::operator vector_t () const { return vector(); }
  //! the const & operator isn't enough.
  template <> inline reader::operator atom_t () const { return atom(); }

  // Operator specializations
  template <> inline
  reader::operator std::string () const { return str(); }

  //! specialization for C-strings.
  //! @return a pointer to the internal ark::c_str().
  template <> reader::operator const char * () const;

  //! return true for 'true' and false for 'false'
  template <> reader::operator bool () const;

#define declOperator(T) \
  template <> reader::operator T () const;

  declOperator(signed char)
  declOperator(unsigned char)
  declOperator(int)
  declOperator(unsigned int)
  declOperator(long)
  declOperator(unsigned long)
  declOperator(float)
  declOperator(double)
  declOperator(long double)

#undef declOperator

  // Some helpers for containers.
  template <typename T>
  struct set_as_vector_t {
    T &t;
    set_as_vector_t(T &c) : t(c) {}
    void operator^=(const Ark::reader &a) const {
      size_t s = a.sizeVec();
      t.resize(s);
      for(size_t i=0; i < s; ++i) t[i] ^= a.getVec(i);
    }
  };
  // overload helper
  template <typename T> inline
  set_as_vector_t<T> set_as_vector(T &c) { return set_as_vector_t<T>(c); }

  template <typename T>
  struct set_as_list_t {
    T &t;
    set_as_list_t(T &c) : t(c) {}
    void operator^=(const Ark::reader &a) const {
      size_t s = a.sizeVec();
      t.clear();
      for(size_t i=0; i < s; ++i) t.push_back(a.getVec(i));
    }
  };
  // overload helper
  template <typename T> inline
  set_as_list_t<T> set_as_list(T &c) { return set_as_list_t<T>(c); }

  template <typename T>
  struct set_as_set_t {
    T &t;
    set_as_set_t(T &c) : t(c) {}
    void operator^=(const Ark::reader &a) const {
      size_t s = a.sizeVec();
      t.clear();
      typedef typename T::value_type value_t; // to avoid ambiguity--v
      for(size_t i=0; i < s; ++i) t.insert(a.getVec(i).operator value_t ());
    }
  };
  // overload helper
  template <typename T> inline
  set_as_set_t<T> set_as_set(T &c) { return set_as_set_t<T>(c); }

  // helper for C-style arrays
  template <typename T,typename Int>
  struct set_as_array_t {
    T *t;
    Int mx,*len;
    set_as_array_t(T *c,Int m,Int *l) : t(c),mx(m),len(l) {}
    void operator^=(const Ark::reader &a) const {
      Int s = a.sizeVec();
      if (mx < s) details::throw_more_elements_than_expected(s,mx);
      if (s < mx)
        if (!len) details::throw_fewer_elements_than_expected(s,mx);
      if (len) *len = s;
      for(Int i=0; i < s; ++i) t[i] ^= a.getVec(i);
    }
  };
  // overload helper
  template <typename T,typename Int> inline
  //! @param c the array pointer to set
  //! @param m the maximum allowable number of elements
  //! @param l optional pointer.  If not given, m is the exact
  //!        number of elements to read, otherwise, the number
  //!        actually read is recorded in l.
  set_as_array_t<T,Int> set_as_array(T *c,Int m,Int *l=NULL) {
    return set_as_array_t<T,Int>(c,m,l);
  }

  // insert into operator_T structure for magic auto conversion
  // of basic STL types.
  template <typename T>
  struct reader::operator_T< std::vector<T> > {
    static std::vector<T> value(const reader &a) {
      std::vector<T> ret;
      set_as_vector_t< std::vector<T> >(ret) ^= a;
      return ret;
    }
  };

  template <typename T>
  struct reader::operator_T< std::list<T> > {
    static std::list<T> value(const reader &a) {
      std::list<T> ret;
      set_as_list_t< std::list<T> >(ret) ^= a;
      return ret;
    }
  };

  template <typename T>
  struct reader::operator_T< std::set<T> > {
    static std::set<T> value(const reader &a) {
      std::set<T> ret;
      set_as_set_t< std::set<T> >(ret) ^= a;
      return ret;
    }
  };
}

#endif
