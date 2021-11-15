#ifndef __ark__argv_dot_hpp__
#define __ark__argv_dot_hpp__

#include "ark.hpp"
#include <string>
#include <iostream>

namespace Ark{

//! @cond
extern const std::string _dashinclude;
extern const std::string _dashcfg;    
//! @endcond

/*! Parse argv command line arguments in the range [b, e).
    Command line options of the form --include filename and --cfg keyval
    are recognized.  All other command line options are silently ignored.
    For example:
  @code
      argvparse(myark, argv, argv+argc)
  @endcode

  @param a ark to parse into
  @param b iterator pointing to beginning of command line arguments, e.g., argv
  @param e iterator pointing to end of command line arguments, e.g., argv+argc

  @bug The argv utilities may be confused by non-ark command line arguments that
  look like ark-specific arguments, --include or --cfg.
 */
template <class IITER>
void
argvparse(ark& a, IITER b, IITER e){
  const std::string di("--include");
  const std::string dc("--cfg");
  const std::string ds("-");
  Ark::parser P;
  for(; b!=e; ++b){
    if( !*b ){              // handle argv[argc] = NULL
      continue;
    }else if( *b == di ){
      IITER ppb = ++b;
      if(ppb == e)
        throw InputError("dangling --include arg at end of argument sequence");
      P.parse_file(a, *ppb);
    }else if( *b == dc ){
      IITER ppb = ++b;
      if(ppb == e)
        throw InputError("dangling --cfg arg at end of argument sequence");
      if (*ppb == ds) P.parse_keyvals(a, std::cin);
      else            P.parse_keyvals(a, *ppb);
    }else {
      std::string s(*b);
      if( s.substr(0,di.size()+1) == di+"=" ){
        std::string arg = s.substr(di.size()+1);
        P.parse_file(a, arg);
      }
      else if( s.substr(0,dc.size()+1) == dc+"=" ){
        std::string arg = s.substr(dc.size()+1) ;
        if (arg == ds) P.parse_keyvals(a, std::cin);
        else           P.parse_keyvals(a, arg);

      }
    }
  }
}

/*! Copy all arguments from the range [b, e) to out, skipping those
  of the form --include filename and --cfg keyval.  The name
  and behavior are analogous to stl::remove_copy.

  For example:
  
  @code
      std::vector<const char *> nonark;
      argvremove_copy(argv, argv+argc, std::back_inserter(nonark));
      //< process nonark command-line options >
      argvparse(myark, argv, argv+argc);
  @endcode

  Or if you prefer to avoid fancy STLisms and to copy the final NULL:

  @code
      const char **nonarkv = new[argc+1];  // don't forget to delete[]
      int nonarkc = argvremove_copy(argv, argv+argc+1, nonarkv) - nonarkv - 1;
      //< process nonarkc/nonarkv command-line options >
      argvparse(myark, argv, argv+argc);
  @endcode

  It is permissible for out and b to point to the same location.
  It is an error if out points inside the range [b+1, e).

  Note that if e=argv+argc+1 the final NULL pointer at argv[argc] will
  be copied to the output.  If, instead, e=argv+argc, then the
  final NULL will not be copied.

  @param b iterator pointing to beginning of command line arguments, e.g., argv
  @param e iterator pointing to end of command line arguments, e.g., argv+argc
  @param out output iterator to which non-ark arguments will be copied.
  @return an output iterator pointing to the end of the final range.

  @bug The argv utilities may be confused by non-ark command line arguments that
  look like ark-specific arguments, --include or --cfg.
 */
template <class IITER, class OITER>
OITER
argvremove_copy(IITER b, IITER e, OITER out){
  const std::string di("--include");
  const std::string dc("--cfg");
  for(; b!=e; ++b){
    if( !*b ){ // handle argv[argc] = NULL
      *out++ = *b;
      continue;
    } else if( *b == di || *b == dc ){
      if( ++b == e )
        throw InputError("dangling --include or --cfg arg at end of argument range");
    } else if (std::string(*b).substr(0,di.size()+1)==di+"=" ||
               std::string(*b).substr(0,dc.size()+1)==dc+"=") {
      // pass
    } else {
      *out++ = *b;
    }
  }
  return out;
}
  
/*! Destructively remove all arguments of the form --include filename and --cfg
  keyval from the command line arguments in the range [b, e).  An iterator
  pointing to the new end of the range is returned.

  @code
      argvparse(myark, argv, argv+argc);
      argc = argvremove(argv, argv+argc+1) - argv - 1;
      // or alternatively, if you don't need argv[argc] = NULL
      argc = argvremove(argv, argv+argc) - argv;
  @endcode

  @param b iterator pointing to beginning of command line arguments, e.g., argv
  @param e iterator pointing to end of command line arguments, e.g., argv+argc
  @return an iterator pointing to the new end of the copied range.

  @bug The argv utilities may be confused by non-ark command line arguments that
  look like ark-specific arguments, --include or --cfg.
 */
template <class ITER>
ITER
argvremove(ITER b, ITER e){
    return argvremove_copy(b, e, b);
}

}
#endif
