#include "base.hpp"
#include "tokens.hpp"

#include <sstream>
#include <cstdlib>

namespace Ark {

  exception::exception(const std::string& s):
  std::runtime_error(s) {}

  ark::ark(kind_t k) {
    u.bits = bitmasks::none;
    be(k);
  }
  ark::ark() {
    u.bits = bitmasks::none;
  }

  ark::ark(const atom_t &a) {
    u.atom.init(a.c_str());
  }
  ark::ark(const std::string& str) {
     u.atom.init(str.c_str());
  }
  ark::ark(const char* str) {
     u.atom.init(str);
  }
  ark::ark(const vector_t &v) {
    u.bits = bitmasks::none;
    be(Vector).vector() = v;
  }
  ark::ark(const table_t &t) {
    u.bits = bitmasks::none;
    be(Table).table() = t;
  }

  ark::~ark() { clear(); }

  ark &ark::be(kind_t t) {
    kind_t _kind=kind();
    if (t!=_kind) {
      switch(_kind) {
      case None: break;
      case Atom: u.atom.destroy(); break;
      case Vector: delete unmasked().vector; break;
      case Table:  delete unmasked().table; break;
      }
      switch(t) {
      case None: u.bits = bitmasks::none; break;
      case Atom: u.atom.init(""); break;
      case Vector: u.vector = new vector_t; mask(bitmasks::vector); break;
      case Table:  u.table  = new table_t; mask(bitmasks::table); break;
      }
    }
    return *this;
  }
#ifdef ANARKY_NOW
  ark::ark(const ark &a) : ano(a.ano) { // copy annotation
#else
  ark::ark(const ark &a) {
#endif
    u = a.u;
    switch(kind()) {
    case None: break;
    case Atom:
      u.atom.init(atom().c_str()); break;
    case Vector:
      u.vector = new vector_t(vector()); mask(bitmasks::vector); break;
    case Table :
      u.table = new table_t(table()); mask(bitmasks::table); break;
    }
  }
  
  void ark::swap(ark &a) {
    var_ptr_t tmpp = u;
    u = a.u;
    a.u = tmpp;
#ifdef ANARKY_NOW
    std::swap(ano,a.ano);
#endif
  }

  const atom_t *ark::get_atom() const {
    if (kind()==Atom) return &atom();
    return NULL;
  }

  const ark * ark::get(unsigned i) const {
    if (kind()==Vector) {
      const vector_t &v=vector();
      if (i < v.size()) return &v[i];
    }
    return NULL;
  }
  
  const ark * ark::get(const key_t &k) const {
    if (kind()==Table) {
      const table_t &t=table();
      table_t::const_iterator i=t.find(k);
      if (i!=t.end()) return &i->second;
    }
    return NULL;
  }
  
  const ark * ark::xget(const std::string &s) const {
    const ark * ret=this;
    try {
      tokenizer::syntax S("[].","");
      std::istringstream In(s);
      tokenizer t(In,S);
      t.next();
      while( ret && t.current().kind() != token::End ) {
        if (t.current().syntax()=='[') { 
          if (t.next().kind()!=token::Symbol) return NULL;

          unsigned offset = strtoul(t.current().text().c_str(),NULL,10);

          if (t.next().syntax()!=']') return NULL;
          ret = ret->get(offset);
        }
        else if (t.current().kind()==token::Symbol) {
          ret = ret->get(t.current().text());
        }
        else return NULL;

        t.next();

        if (t.current().kind() != token::End) {
          switch(t.current().syntax()) {
          case '.':
            if (t.next().kind() != token::Symbol) ret = NULL;
            // fallthrough
          case '[': break;
          default:  return NULL;
          }
        }
      }
      return ret;
    } catch(...) { /* anything goes wrong, you get NULL back */ }
    return NULL;
  }
}
