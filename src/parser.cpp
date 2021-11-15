#include "exception.hpp"
#include "parser.hpp"

#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <climits>

#include <sys/stat.h>

namespace Ark {

  // static member initialization
  tokenizer::syntax parser::key_syn("{}<>[]!.=","#");
  tokenizer::syntax parser::val_syn("{}<>[]!?","#");
  tokenizer::syntax parser::no_syn("{}<>[]","#");

  inline
  std::string throwInputError(exception         &orig,
                              const std::string &msg,
                              const char        *file,
                              tokenizer         &t) {
    std::ostringstream o;
    o << orig.what() << "\n"
      << msg << " at " << (file?file:"???")
      << ":" << t.lineno() << ":" << t.colno();
    throw InputError(o.str());
  }

  //public:
  ark &parser::parse(ark &a,std::istream &in) const {
    tokenizer t(in,no_syn);
    try {
      t.next(val_syn);
      a = parse_value(t);
      if (t.next().kind() != token::End )
        throw InputError("extra stuff after the value");
    }
    catch (exception &e) {
      throwInputError(e,"parse problem",current_file,t);
    }
    return a;
  }
  ark &parser::parse(ark &a,const std::string &s) const {
    std::istringstream In(s);
    return parse(a,In);
  }
  //public:
  ark &parser::parse_keyvals(ark &a,std::istream &in) const {
    tokenizer t(in,no_syn);
    try {
#ifdef ANARKY_NOW
      ark::annotation_t anno(current_file,t.lineno(),t.colno());
      if (a.kind()!=Table) a.annotate(anno);
#endif
      a.be(Table);
      while( t.next(key_syn).kind()!=token::End ) parse_keyvalue(a,t);
    }
    catch (exception &e) {
      throwInputError(e,"parse problem",current_file,t);
    }
    return a;
  }
  ark &parser::parse_keyvals(ark &a,const std::string &s) const {
    std::istringstream In(s);
    return parse_keyvals(a,In);
  }
  //private:
  void parser::parse_keyvalue(ark &a,tokenizer &t) const {
    // check for !include and other key-like specials
    if (t.current().kind()==token::Syntax
        && t.current().syntax()=='!') {
      if (t.next().kind() != token::Symbol)
        throw InputError("expecting a special symbol");

      if (t.current().text()=="include") {
        t.next();
        if (t.current().kind()==token::Symbol
            || t.current().kind()==token::String) {
          try {
            include_file(a,t.current().text());
          }
          catch (exception &e) {
            throwInputError(e,"include problem",current_file,t);
          }
        }
        else
          throw InputError("!include expected a string or quoted string");
      }
      else
        throw InputError("unknown special token");
      return;
    } // nothing special

    // OK, I expect to see a key now
    if (t.current().kind()!=token::Symbol)
      throw InputError("expecting a key symbol");

#ifdef ANARKY_NOW
    ark::annotation_t anno(current_file,t.lineno(),t.colno());
    if (a.kind()!=Table) a.annotate(anno);
#endif

    table_t &d=a.be(Table).table(); // I must be a table
    key_t k=t.current().text();     // this must be my key
    t.next(key_syn);

    // possible special syntax for !erase
    if (t.current().kind()==token::Syntax
        && t.current().syntax()=='!') {
      if (t.next().kind() != token::Symbol)
        throw InputError("expecting a special symbol");

      if (t.current().text()=="erase") {
        d.erase(k);
      }
      else
        throw InputError("unknown special token");
      return;
    } // nothing special

    descend(d[k],t);
  }
  // private:
  void parser::descend(ark &a,tokenizer &t) const {
#ifdef ANARKY_NOW
    ark::annotation_t anno(current_file,t.lineno(),t.colno());
#endif
    if (t.current().syntax()=='[') {
      unsigned offset;
#ifdef ANARKY_NOW      
      if (a.kind()!=Vector) a.annotate(anno);
#endif
      // we have a vector access
      vector_t &v=a.be(Vector).vector();

      // read a number or "+"
      if (t.next(key_syn).kind()!=token::Symbol)
        throw InputError("expecting a number");
      if (t.current().text()=="+") {
        offset = v.size();
      }
      else {
        const char * S=t.current().text().c_str();
        char       * E=NULL;
        offset = strtoul(S,&E,10);
        if (S==E || *E) throw InputError("unable to parse strange number");
      }
      // read ']'
      if (t.next(key_syn).syntax()!=']') throw InputError("expecting ']'");

      if (offset==v.size()) v.push_back(ark(None));
      else if (offset > v.size())
        throw InputError("non-contiguous vector set not allowed");
      t.next(key_syn); // get next and keep descending

      // possible special syntax for !erase
      if (t.current().kind()==token::Syntax
          && t.current().syntax()=='!') {
        if (t.next().kind() != token::Symbol)
          throw InputError("expecting a special symbol");

        if (t.current().text()=="erase") {
          v.erase(v.begin()+offset);
        }
        else
          throw InputError("unknown special token");
        return;
      } // nothing special

      descend(v[offset],t);
    }
    // t now is on a . is more key or an = if value is next
    // now follow any array references after it
    else if (t.current().syntax()=='.') {
      t.next(key_syn); // back into a keyvals context
      parse_keyvalue(a,t);
    }
    else if (t.current().syntax()=='{') {
#ifdef ANARKY_NOW
      if (a.kind()!=Table) a.annotate(anno);
#endif
      // namespace
      a.be(Table);
      while( t.next(key_syn).syntax()!='}') parse_keyvalue(a,t);
    }
    else if (t.current().syntax()=='=') {
      // assignment
      t.next(val_syn);
      a = parse_value(t);
      //a.annotate(anno);
    }
    else throw InputError("expecting '.' or '=' or '{'");
  }
  namespace {
    std::string pathify(const std::string &s,const char *path) {
      if (!path || (s.size() && s[0]=='/')) return s;
      const char *ind = strrchr(path,'/');
      if (!ind) return s;
      return std::string(path,ind+1)+s;
    }
  }
  //private:
  ark parser::parse_value(tokenizer &t) const {
    ark a;
#ifdef ANARKY_NOW
    ark::annotation_t anno(current_file,t.lineno(),t.colno());
    a.annotate(anno);
#endif
    // check for special form
    if (t.current().syntax()=='!') {
      if (t.next().kind() != token::Symbol)
        throw InputError("expecting a special symbol");

      if (t.current().text()=="file") {
        t.next();
        if (t.current().kind()==token::Symbol
            || t.current().kind()==token::String) {
          a.be(Atom); // use current_file to get relative path
          a.atom() = pathify(t.current().text(),current_file);
        }
        else
          throw InputError("!file expected a string or quoted string");
      }
      else
        throw InputError("unknown special token");
    }
    else { // ok, not special
      switch(t.current().kind()) {
      case token::Symbol:
      case token::String:
        a.be(Atom);
        a.atom() = t.current().text();
        break;
      case token::Syntax:
        switch( t.current().syntax() ) {
        case '[':
          a.be(Vector);
          for(int i=0; t.next(val_syn).syntax()!=']'; i++)
            a.vector().push_back(parse_value(t));
          break;
        case '{':
          a.be(Table);
          while( t.next(key_syn).syntax()!='}') parse_keyvalue(a,t);
          break;
        case '?':
          a.be(None);
          break;
        default:
          throw InputError("expecting '{' or '[' or '?'");
        }
        break;
      default:
        throw InputError("expecting '{' or '[' or '?' or string");
      }
    }
    return a;
  }
  //private:
  void parser::include_file(ark &a,const std::string &f) const {
    if (f.empty())        throw InputError("include filename is empty");
    if (include_depth>20) throw InputError("include depth exceeded");

    // form path
    std::string path = pathify(f,current_file);

    parser tmp;
    tmp.current_file = path.c_str();
    tmp.include_depth = include_depth+1;

    try {
      std::ifstream F;
      // need this because ifstream will open a dir without complaining
      // and act as if it is an empty file.
      struct stat statbuf;
      if(stat(tmp.current_file, &statbuf) == -1) {
        std::ostringstream o;
        o << "unable to stat file: " << tmp.current_file;
        o << " (" << strerror(errno) << ")";
        throw InputError(o.str());
      }
      if (!S_ISREG(statbuf.st_mode)){
        std::ostringstream o;
        o << "not a regular file: " << tmp.current_file;
        throw InputError(o.str());
      }
      F.open(tmp.current_file);
      // ok, done with that mess
      if (F) tmp.parse_keyvals(a,F);
      else {
        std::ostringstream o;
        o << "unable to read file: " << tmp.current_file;
        throw InputError(o.str());
      }
    }
    catch (exception &e) {
      std::ostringstream o;
      o << "unable to parse file: " << tmp.current_file
        << "\n" << e.what();
      throw InputError(o.str());
    }
  }
}
