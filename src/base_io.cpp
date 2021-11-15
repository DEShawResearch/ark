#include "base.hpp"
#include "exception.hpp"
#include "tokens.hpp"
#include "printer.hpp"

#include <sstream>

/*! \file ark/base_io.cpp

The base I/O routines use the strict grammar.  Defined as follows:
\verbatim
---STRICT GRAMMAR---
  *,+,| are the usual meta-grammar operations allcaps words are
  non-terminals.  Everything else is as you see it.
ARK  -> NONE | STRING | [ARK*] | { KEYVAL* } | BOX
NONE -> ?
STRING is a " delimited string with simple '\' meta-charactering
KEYVAL -> KEY=ARK
KEY is a non-delimited string in the regexp [_a-xA-Z][_a-zA-Z0-9]*
BOX -> < POINTER >
POINTER is the pointer to the internal contents of the box.
\endverbatim

We have not done much with the box type in ark and this policy of
printing is of highly debatable value.  We also have no standard for
parsing the POINTER field for BOX and the policy for printing may
change because of that.
*/

namespace {
  using namespace Ark;
  //! Internal function to do strict ark parsing.
  //! @param t a token stream.
  //! @return the parsed ark.
  ark strict_parse(tokenizer &t) {
    ark a;
    switch(t.current().kind()) {
    case token::Symbol:
    case token::String:
      a.be(Atom).atom() = t.current().text();
      return a;
    case token::Syntax:
      {
        switch( t.current().syntax() ) {
        case '[':
          a.be(Vector);
          for(int i=0; t.next().syntax()!=']'; i++)
            a.vector().push_back(strict_parse(t));
          break;
        case '{':
          a.be(Table);
          for(int i=0; t.next().syntax()!='}'; i++) {
            if (t.current().kind() != token::Symbol)
              throw  InputError("expecting a key symbol");
            Ark::key_t key(t.current().text());
            if (a.table().find(key) != a.table().end()) {
              std::string err("duplicate key: ");
              throw InputError(err+key.str());
            }
            if (t.next().kind()!=token::Syntax || t.current().syntax()!='=')
              throw InputError("expecting a '='");
            t.next();
            a.table()[key] = strict_parse(t);
          }
          break;
        case '?':
          a.be(None); break;
        default:
          throw InputError("expecting '{' or '[' or '?'");
        }
        return a;
      }
      break;
    default:
      throw InputError("expecting '{' or '[' or '?' or string");
    }
  }
}

namespace Ark {

  ark parse(const std::string &s) {
    std::istringstream in(s);
    return parse(in);
  }

  ark parse(std::istream &in) {
    tokenizer t(in,tokenizer::syntax("{}<>[]=?","#"));
    try {
      ark a;
      t.next();
      a = strict_parse(t);
      if (t.next().kind() != token::End )
        throw InputError("extra stuff after the value");
      return a;
    }
    catch (Ark::exception &e) {
      std::ostringstream o;
      o << e.what() << "\n"
        <<"input error at line=" << t.lineno() << ",col=" <<t.colno()
        << ":"<< t.current().text() << std::endl;
      throw InputError(o.str());
    }
  }
}  


std::ostream &operator<<(std::ostream &o,const Ark::ark &a) {
  return o << Ark::printer()(a);
}

std::istream &operator>>(std::istream &i,Ark::ark &a) {
  a.clear();
  a = Ark::parse(i);
  return i;
}


