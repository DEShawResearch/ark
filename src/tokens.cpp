#include "exception.hpp"
#include "tokens.hpp"
#include <sstream>
#include <ctype.h>

namespace Ark {
  std::string token::text() const {
    switch(k) {
    case None:    return "<undefined>"; 
    case End:     return "<end of file>"; 
    case Syntax:
    case Symbol:  return buf;
    case String:  return buf;//.substr(1,buf.size()-2);
    }
    // can't reach here
    return "no way I reached this line here";
  }

  void tokenizer::nextch(void) {
    if (*C=='\n') ++line, col=0; else ++col;
    if (++C==E) {
      C=BUF;
      in.get(C,BUFsize,0);
      E=C+in.gcount();
    }
  }

  const token &tokenizer::next(const syntax &S) {
    // mop up whitespace and comments
    // ok so current char is non-whitespace within bounds
    while( C!=E ) {
      while( C!=E && isspace(*C) ) nextch();
      if (S.is_comment(*C))
        while( C!=E && *C!='\n' ) nextch();
      else break;
    }
    if (C==E) {
      t.k   = token::End;
      nextch();
    }
    else if (S.is_syntax(*C)) {
      t.k   = token::Syntax;
      t.buf = *C;
      nextch();
    }
    else if (S.is_quote(*C)) {
      t.k   = token::String;
      //t.buf = *C;
      t.buf.clear();
      char q = *C;
      bool esc=0;
      for( nextch(); C!=E && (*C!=q || esc) ; nextch() )
        if ( *C !='\\' || esc) {
          t.buf += *C; esc=0;
        }
        else esc=1;
      if (*C != q) // check terminal quote
        throw InputError("invalid string token");
      //t.buf += *C;
      nextch();
    }
    else {
      t.k   = token::Symbol;
      t.buf = *C;
      for(nextch(); C!=E && !isspace(*C) && !S.is_reserved(*C);nextch())
        t.buf += *C;
    }
    return t;
  }
}
