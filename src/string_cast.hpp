/* DOCUMENTATION_BEGIN
  
   string_cast provides templated functions that convert arbitrary
   data types to (toString) and from (stringTo) strings.  

   To convert a string to a T use stringTo:
      template <typename T>
      T string_cast::stringTo<T>(const std::string&);

   If a string 'is_convertible' to T, then stringTo<T> calls the
   conversion function.  Otherwise it constructs an istringstream
   and uses an extraction (>>) operator.

   To convert a T to a string use toString:
      template <typename T>
      std::string string_cast::toString(const T&);

   If T 'is_convertible' to a string, then toString<T> calls the
   conversion function.  Otherwise, it constructs an ostringstream and
   uses an insertion (<<) operator.

   The meaning of 'is_convertible' is as in TR1 and the boost
   type_traits library.  Essentially, it means that either a
   non-explicit constructor or a member conversion function is
   available to perform the conversion.
  
   If a conversion error occurs, or if stringTo's argument is not
   completely consumed, a bad_string_cast (derived from
   std::runtime_error) is thrown.

   Since a string 'is_convertible' to a string, toString<string> and
   stringTo<string> both use the copy-constructor, which copies the
   entire string, including whitespace.
  
   The following considerationss apply when stream-based conversions
   are used.  They attempt to work around surprises and defects in the
   standard stream behavior.

   The standard behavior for the extraction operator is to skip
   leading whitespace (space, newline, tab, form-feed, vertical-tab).
   stringTo<T> skips leading whitespace except if T=char (see below)
   or if T=string (see above).

   stringTo<bool> recognizes "1", "0", "true" and "false".  It is case-sensitive.
   toString<bool> produces either "1" or "0".

   stringTo<char> does not skip whitespace.  The argument must have length()==1.

   stringTo clears the stream's basefield, which has the effect of
   doing integer conversions "as if" done by scanf "%i".  Thus, octal
   and hex strings are converted "correctly".  I.e., "0x10" and "010"
   are converted to 16 and 8 respectively.

   If T is a floating point type, then the stringTo conversion is performed by
   strtof, strtod, or strtold, as appropriate, which avoids deficiencies
   in C++ insertion operator handling of infinities, nans, etc.

   If T has a template <typename S> T(const S&) constructor, then
   stringTo<T> is likely to misbehave because it will try to
   construct the T with a T(const std::string&).  Workarounds:
   1) provide a non-templated T(const std::string&) or 2)
   or use the nifty boost/C++0x if_enabled metaprogramming hacks
   to prevent the template from matching.

   toString sets the ostringstream's precision to DECIMAL_DIG (see
   float.h) which should be enough to unambiguously represent any
   numeric value of the widest floating point type.  It may, however,
   be overkill.

   Note that stringTo, toString and bad_string_cast are in the
   string_cast namespace to avoid conflict with the very likely
   existence of user-defined functions called toString or stringTo
   (with similar if not identical behavior).

   There is some magic here (is_convertible), and quite a few details.
   It's pretty much exactly what you would have written yourself if
   you had taken the time to worry about templates, namespaces,
   exceptions, precision, basefield and special-casing of conversions of
   strings, floats, chars and bools.
  
   string_cast is a lot like boost::lexical cast, except that:
    - it uses a constructor or conversion member if one exists.
    - it deals with integer constants written in hex and octal.
    - it deals with Inf and Nan and anything else that strtod
      deals with that C++ istreams do not.
    - it deals with embedded whitespace when extracting 
      user-defined types.
    - it uses a sufficient precision even if you've got a user-defined
      type that does not specialize numeric_limits.
    - it's a lot smaller.
    - it doesn't require you to choose a particular version of boost.
    - it only converts to and from strings.  It doesn't attempt
      conversions between arbitrary types.

   Example:

    #include <string_cast/string_cast.hpp>
    using namespace std;
    using namespace string_cast;

    int i = stringTo<int>("314");
    int j = stringTo<int>("0x314");
    string withspaces = stringTo<string>("some string with spaces");
    //   assuming operator>>(std::istream&, MyType& ) exists
    MyType mt = stringTo<MyType>("3 14.15 xxx"); 
  
    string numberstring = toString(i);
    string mystring = toString(mt);
    string still_with_spaces = toString(withspaces);
  
    The header file has a self-contained implementation of
    is_convertible that relies on neither tr1 nor Boost.  However, the
    tr1 implementation may be used instead by defining the pp-symbol
    USE_TR1_TYPE_TRAITS at compile time.  Alternatively, the Boost
    version may be used by defining the pp-symbol
    USE_BOOST_TYPE_TRAITS at compile time.  Note that both of these
    may require other command-line flags to work properly, e.g.,
    -std=c++09 or -I/path/to/boost.

DOCUMENTATION_END */

// C++11 notes:
//  - is_convertible is officially standardized, but continues to have
//    the meaning "is_implicitly_convertible".  There is no
//    "is_explicitly_convertible", but there is is_constructible<TO,
//    FROM> which is very, very close.  See the working paper
//      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3047.html
//    for discussion of corner cases.
//
#ifndef __string_cast_dot_hpp__
#define __string_cast_dot_hpp__

#include <string>
#include <sstream>
#include <typeinfo>
#include <cfloat>               // For DECIMAL_DIG
#include <stdexcept>
#include <cstdlib>
#include <errno.h>

#if defined(USE_TR1_TYPE_TRAITS)
// As of tr1, type_traits are really, truly part of C++.  If your
// compiler supports the tr1 extensions, then adding
// -DUSE_TR1_TYPE_TRAITS to your command line will probably get you
// the most reliable implementation of is_convertible for your
// platform.  Note, you may need to add other command-line options
// to enable tr1 support.  E.g., with gcc4.4, you have to say -std=c++0x
// or -std=gnu++0x
#include <type_traits>
#elif defined(USE_BOOST_TYPE_TRAITS)
// As of July 2009, we don't use boost anywhere else in string_cast.  I'd
// rather not start a versionitis pandemic by introducing a specific
// version as a module prereq.  As an alternative, you are encouraged to add
// -DUSE_BOOST_TYPE_TRAITS to your command line if you are
// compiling/linking this file into a library or application that
// already uses a version of boost of your choice.  Type traits has
// been part of boost since 1.13, c. 2000.  So in 2009, if you're using
// boost, you're almost certainly using a version that supports
// is_convertible
#include <boost/type_traits/is_convertible.hpp>
#else
// If you don't have TR1 and you don't want to #include boost headers,
// then we do our best right here to implement is_convertible.  The
// boost version and tr1 versions (see above) are probably more
// portable.  If the version here doesn't work, please consider using
// TR1 or boost before trying to "fix" it.
#endif

// WARNING - is_convertible is not exactly what we want.
// If the constructor T(const ark&) is explicit, is_convertible<ark,T> is false.
// A fix is suggested at:
//  http://groups.google.com/group/comp.std.c++/browse_thread/thread/b567617bfccabcad#
// (If that url doesn't work, search the June 2009 comp.std.c++ archives for:
//  Subject:  a type_trait to tell if a particular constructor exists )
// Unfortunately, the suggested solution doesn't work with gcc/4.3 or earlier.
// Consider adopting the suggestion when/if we adopt gcc/4.4 or later.

namespace  string_cast{

class bad_string_cast : public std::runtime_error {
public:
    bad_string_cast( const std::string& message ) : std::runtime_error(message) {}
};

  namespace detail{

#if !defined(USE_TR1_TYPE_TRAITS) && !defined(USE_BOOST_TYPE_TRAITS)
    // This is basically the boost/1.38 implementation #ifdef'ed
    // for __GNUC__.  Comments in the boost source say:
    // "This version does not pass UDTs through ..."
    // YMMV.
    typedef char yes_type;
    struct no_type{ char padding[8]; };
    
    struct any_conversion
    {
      template <typename T> any_conversion(const volatile T&);
      template <typename T> any_conversion(T&);
    };

    template <typename T> struct checker
    {
      static no_type _m_check(any_conversion ...);
      static yes_type _m_check(T, int);
    };

    template <typename From, typename To>
    struct is_convertible
    {
      static From _m_from;
      static bool const value = sizeof( detail::checker<To>::_m_check(_m_from, 0) )
        == sizeof(yes_type);
    };
#endif // we're on our own for type_traits
  // Now that we've got an is_convertible predicate we can implement
  // stringTo and toString that choose a strategy depending on whether the
  // constructor is available.
  // First stringTo:
  template <typename T, bool is_convertible>
  struct _stringTo{};

  template <typename T>
  struct _stringTo<T, true>{
      static T convert(const std::string&a){ return T(a); }
  };

  template <typename T>
  struct _stringTo<T, false>{
      // If string isn't convertible to T, then go through an istringstream
      static T convert(const std::string&s){ 
          std::istringstream iss(s);
          T val;
          // by clearing basefield, integer conversions are 'as if' done by %i,
          // which does something sensible with 0xb, 11 and 013.
          iss.unsetf(std::ios::basefield);
          // Follow lexical_cast's logic and check that get() after the extractor returns eof
          if( !(iss >> val) || iss.get() != std::char_traits<std::istringstream::char_type>::eof() )
              throw bad_string_cast("string_cast::stringTo:  Failed to convert \"" + s + "\" to type " + typeid(T).name() );
          return val;
      }
  };

  // And the same trickery for toString
  template <typename T, bool is_convertible>
  struct _toString{};

  template <typename T>
  struct _toString<T, true>{
      static std::string convert(const T&v){ return std::string(v); }
  };

  template <class T>
  struct _toString<T, false>{
      // If T isn't convertible to string, then go through an ostream
      static std::string convert(const T& val){
          std::ostringstream oss;
          // One could jump through MANY hoops to try to evaluate exactly
          // how many digits are needed.  One day, it might even be
          // standardized as numeric_limits::max_digits10, e.g.,
          // http://www2.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1822.pdf.
          // But why bother?  It's much better to specify too much precision
          // than too little.  C99 gives us DECIMAL_DIG, which is enough
          // to preserve the value of any floating point type.  C99 didn't
          // bother to specialize for each type (arguably an oversight), so
          // we won't either.
#ifdef DECIMAL_DIG
          oss.precision(DECIMAL_DIG);
#else
          oss.precision(21); // enough for a 64-bit, radix-2 significand, e.g., x86 long double
#endif
          oss << val;
          if( !oss )
              throw bad_string_cast(std::string("string_cast::toString:  Failed to insert type ") + typeid(T).name() + " into ostringstream");
          return oss.str();
      }
  };

  } // namespace detail

// toString is the easy one.  The only interesting special case is the copy-constructor
// which is handled by the is_convertible logic.
template <typename T>
std::string toString(const T& v){
#if defined( USE_TR1_TYPE_TRAITS )
    return detail::_toString<T, std::is_convertible<T, std::string>::value >::convert(v);
#elif defined( USE_BOOST_TYPE_TRAITS )
    return detail::_toString<T, boost::is_convertible<T, std::string>::value >::convert(v);
#else
    return detail::_toString<T, detail::is_convertible<T, std::string>::value >::convert(v);
#endif
}

// stringTo has lots of special cases.
// The generic stringTo
template <typename T>
T stringTo(const std::string& s){
#if defined( USE_TR1_TYPE_TRAITS )
    return detail::_stringTo<T, std::is_convertible<std::string, T>::value >::convert(s);
#elif defined( USE_BOOST_TYPE_TRAITS )
    return detail::_stringTo<T, boost::is_convertible<std::string, T>::value >::convert(s);
#else
    return detail::_stringTo<T, detail::is_convertible<std::string, T>::value >::convert(s);
#endif
}

// specialize stringTo<bool> to deal with "true", "false", "1" or "0".
template <>
inline bool stringTo<bool>(const std::string& s){
    // The C++ standard says that whitespace is space, tab, newline, form-feed and vertical-tab.
    size_t pos = s.find_first_not_of(" \t\n\f\v");
    if(pos == std::string::npos)
        throw bad_string_cast("string_cast::stringTo<bool>: argument string is all whitespace");
    const std::string postwhite = s.substr(pos);
    if(postwhite == "true" || postwhite == "1")
        return true;
    else if(postwhite == "false" ||  postwhite == "0")
        return false;
    throw bad_string_cast("string_cast::stringTo<bool>:  Failed to convert \"" + s + "\" to bool.");
}

// specialize stringTo<char> so we can convert a single white-space char cleanly
template <>
inline char stringTo<char>(const std::string& s){
    if(s.length() != 1)
        throw bad_string_cast("string_cast::stringTo<char> argument \"" + s + "\" does not have length()==1");
    return s[0];
}

// specialize stringTo<floatingpoint> because C++ insertion can't handle
// Inf, Nan, etc.
template<>
inline float stringTo<float>(const std::string& s){
    const char *b = s.c_str();
    char *e;
    errno = 0;
#ifdef DESRES_OS_Windows
    float x = float(strtod(b, &e));
#else
    float x = strtof(b, &e);
#endif
    if(errno == EINVAL)
        throw bad_string_cast("string_cast::stringTo<float> argument \"" + s + "\" is invalid");
    if( e != b+s.length() )
        throw bad_string_cast("string_cast::stringTo<float> argument \"" + s + "\" has trailing content");
    return x;
}

template<>
inline double stringTo<double>(const std::string& s){
    const char *b = s.c_str();
    char *e;
    errno = 0;
    double x = strtod(b, &e);
    if(errno == EINVAL)
        throw bad_string_cast("string_cast::stringTo<double> argument \"" + s + "\" is invalid");
    if( e != b+s.length() )
        throw bad_string_cast("string_cast::stringTo<double> argument \"" + s + "\" has trailing content");
    return x;
}

template<>
inline long double stringTo<long double>(const std::string& s){
    const char *b = s.c_str();
    char *e;
    errno = 0;
#ifdef DESRES_OS_Windows
    long double x = strtod(b, &e);
#else
    long double x = strtold(b, &e);
#endif
    if(errno == EINVAL)
        throw bad_string_cast("string_cast::stringTo<long double> argument \"" + s + "\" is invalid");
    if( e != b+s.length() )
        throw bad_string_cast("string_cast::stringTo<long double> argument \"" + s + "\" has trailing content");
    return x;
}

}

#endif
