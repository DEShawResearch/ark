#ifndef __ArkTo_hpp__
#define __ArkTo_hpp__

#include "ark.hpp"
#include <string>
#include "string_cast.hpp"
#include <limits>
// N.B.  We used to have our own home-brew implementation of is_convertible.
// Now that we are using C++11, let's just use the standardized version.
// The homebrew can be resurrected from version-controled files prior
// to Oct 27, 2012  should the need arise.
#include <type_traits>

// C++11 type_traits offers us both is_convertible<T,U> and is_constructible<T, Args...>.
//   is_convertible is almost what we want, but it returns false when there is 
//   an *explicit* constructor:  explicit T::T(U).  It's what we had in 
//   our "homebrew" implementation.
//   During the standardization process, there was briefly an is_explicitly_constructible
//   proposed, but it was rejected due to some subtleties outlined in:
//      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3047.html
//   Nevertheless, is_constructible<T,U> (also discussed in n3047) should serve
//   our purposes perfectly.
namespace Ark{

/*! If std::is_convertible<T,ark> is true, i.e., if
  a T(const ark&) constructor exists, 
  then construct and return T(a).  Otherwise, require
  that a.kind() == Ark::Atom and return stringTo<T>(a.atom().str()).

  @param v ark value to convert
  @return converted value
*/
template <typename T>
typename std::enable_if<std::is_constructible<T, const Ark::ark&>::value, T>::type
arkTo(const Ark::ark&a){
    return T(a);
}

template <typename T>
typename std::enable_if<!std::is_constructible<T, const Ark::ark&>::value, T>::type
arkTo(const Ark::ark&a){
    if( a.kind() != Ark::Atom)
        throw InputError("arkTo:  stringTo conversion attempted on atom with kind()!=Ark::Atom " + string_cast::toString(a));
    try {
        return string_cast::stringTo<T>(a.atom().str());
    }catch( string_cast::bad_string_cast &bsc){
        std::stringstream sst;
        sst << "arkTo:  stringTo conversion failed for value "
            << string_cast::toString(a) 
            << "\n" << bsc.what();
        throw InputError(sst.str());
    }
}

/*! Use xget to find key in ark, a.  Return the result of arkTo<T>
  applied to the value.  In the event of an error, (lookup failure
  or conversion error) throws an Ark::exception.  For example:
 @code
     double near_interval = arkTo<double>(a, "respa.near_interval");
 @endcode

 @param a ark to look in.
 @param key extended key to look up.  See xget.
 @return converted value.
*/
template <typename T>
T arkTo(const Ark::ark& a, const std::string& key){ try {
    const Ark::ark *p = a.xget(key);
    if( p == 0 )
      throw InputError("key not in ark");
    return arkTo<T>(*p);
  }catch(exception& e){
      std::stringstream sst;
      sst << e.what() << "\n"
          << "Looking up key '" << key 
          << "' in ark: " << string_cast::toString(a);
      throw exception(sst.str());
  }
} // do not delete - doxygen doesn't grok function-scope try blocks

/*! Use xget to find key in ark, a.  If the key is not in the ark, or
  if the key's value is of kind Ark::None, then return dflt.
  Otherwise, return the result of
  applying arkTo<T> to the value.  If there is a conversion error, throw
  an Ark::exception.  For example:
  @code
     debug = arkTo<bool>(a, "debug", false);
  @endcode

  @param a ark to look in.
  @param key extended key to look up.  See xget.
  @param dflt default value, returned if the key is not present in the ark
  @return converted value
*/
template <typename T>
T arkTo(const Ark::ark& a,  const std::string& key, const T& dflt){ try {
    const Ark::ark *p = a.xget(key);
    if( p == 0 || p->kind() == Ark::None )
      return dflt;
    return arkTo<T>(*p);
  }catch(exception& e){
      std::stringstream sst;
      sst << e.what() << "\n"
          << "Looking up key '" << key 
          << "' in ark: " << string_cast::toString(a);
      throw exception(sst.str());
  }
} // do not delete - doxygen doesn't grok function-scope try blocks

/*! Throw an Ark::exception if a.kind() is not Ark::Vector.  Iterate over the
  vector copying the result of arkTo<T> 
  to the output iterator out.  No more than maxcopy elements
  will be copied to the output iterator.
  If there is a conversion error, throw
  an Ark::exception.

  @param a ark to look in.
  @param out an output iterator to which converted values will be assigned.
  @param maxcopy (optional) maximum number of elements to copy.
  @return an iterator that points to one past the last assigned output value
*/
template <typename T, typename ITER>
ITER arkToIter(const Ark::ark& a, ITER out, size_t maxcopy=std::numeric_limits<size_t>::max()) {
  if( a.kind() != Ark::Vector )
      throw InputError("arkToIter<T,ITER>(a, out):  a is not a vector:  a=" + string_cast::toString(a));
  for(Ark::vector_t::const_iterator pp = a.vector().begin(); pp!=a.vector().end() && maxcopy--; ++pp){
      *out++ = arkTo<T>(*pp);
  }
  return out;
}

/*! Use xget to find the key in ark, a.  Throw an exception if
  the result, p, is NULL.  Otherwise, return arkToIter(*p, out, maxcopy).
  For example:
  @code
     std::vector<double> temperatures;
     arkToIteropy<double>(a, "temperature", std::back_inserter(temperature));

     // With a fixed-size array, use maxcopy to avoid overflow:
     int procs[3];
     int *end = arkToIter<int>(a, "nprocs", &procs[0], 3);
     assert( end-procs == 3 );
  @endcode

  @param a ark to look in.
  @param key extended key to look up.  See xget.
  @param out an output iterator to which converted values will be assigned.
  @param maxcopy (optional) maximum number of elements to copy.
  @return an iterator that points to one past the last assigned output value
*/
template <typename T, typename ITER>
ITER arkToIter(const Ark::ark& a, const std::string& key, ITER out, size_t maxcopy=std::numeric_limits<size_t>::max()){
    const Ark::ark *p = a.xget(key);
    if( p == 0 )
        throw InputError("arkToIter<T,ITER>:  key (" + key + ") does not exist in ark: " + string_cast::toString(*p));
    return arkToIter<T>(*p, out, maxcopy);
}


} // namespace Ark
#endif
