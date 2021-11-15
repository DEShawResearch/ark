#ifndef ark_exception_hpp
#define ark_exception_hpp

#include <stdexcept>

/*! \file ark/exception.hpp
  Ark exception handling 
*/

namespace Ark {

  //! Ark::exception inherits from std::runtime_error.
  struct exception : public std::runtime_error {
    /*! Constructor.
      @param s the exception message.
    */
    explicit exception(const std::string& s);
  };

  //! Ark::InputError is an alternative which can be thrown by parsers
  //! for invalid inputs.
  struct InputError : public exception {

    /*! Constructor.
      @param s the exception message.
    */
    explicit InputError(const std::string& s) : exception(s) {}

    /*! Constructor to allow coercion of regular exceptions into InputErrors.
      @param e an Ark::exception
    */
    explicit InputError(const exception& e) : exception(e) {}
  };

}

#endif
