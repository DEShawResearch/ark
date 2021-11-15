#include "reader.hpp"
#include "tokens.hpp"

#include "string_cast.hpp"

#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <cstring>

//! \file util/Arkreader.hxx

namespace Ark {

  void details::throw_more_elements_than_expected(size_t s,size_t mx) {
      std::stringstream sst;
      sst << "More elements (" << s << ") in ark vector than expected ("
          << mx << ")";
      throw exception(sst.str());
  }
  void details::throw_fewer_elements_than_expected(size_t s,size_t mn) {
      std::stringstream sst;
      sst << "Fewer elements (" << s << ") in ark vector than expected ("
          << mn << ")";
      throw exception(sst.str());
  }

  namespace {
    std::string kindstr(const kind_t &k) {
      switch (k) {
      case Atom: return "atom";
      case Vector: return "vector";
      case Table: return "table";
      case None: return "none";
      default: return "unknown";
      }
    }
  }

  const ark & reader::top() const {
    if (lost())
        throw badsearch(std::string("no data found.\n" + report()));
    return *_current;
  }

  reader::reader(const ark &a) : reader() {
    if (a.kind()!=None) _current = &a;
  }

  void reader::descend(const size_t i) {
    char buf[64];
    sprintf(buf, "[%lu]", i);
    _history += buf;
    // leave bad searches alone
    if (lost()) return;
    const vector_t & v=vector();
    if (i >= v.size() || v[i].kind() == None)
      _current = NULL; // treat as a key error
    else
      _current = &v[i];
  }

  void reader::descend(const key_t &s) {
    _history += std::string(".") + s.str();
    // If currently good, then push current as a table.
    // Yes, we should generate an exception if not a table.
    // Whether we error or not, this table becomes a new scope.
    if (found()) _scope.push_front(&table());
    else return; // bad searches are left bad

    typedef std::list<const table_t *>::iterator iter_t;
    iter_t i = _scope.begin();
    table_t::const_iterator it = (*i)->find(s);
    if (! (it == (*i)->end() || it->second.kind() == None) ) {
      _current = &(it->second);
      return;
    }
    // key error, nothing found.
    _current = NULL;
  }

  void reader::bounce(const key_t &s) {
    // (note: bounce leaves invalid readers alone)
    _history += std::string(" ") + s.str();
    // If currently good, then push current as a table.
    // Yes, we should generate an exception if not a table.
    // Whether we error or not, this table becomes a new scope.
    if (found()) _scope.push_front(&table());

    typedef std::list<const table_t *>::iterator iter_t;
    for(iter_t i=_scope.begin(), e=_scope.end(); i != e ; ++i) {
      // look up s in scope i
      table_t::const_iterator it = (*i)->find(s);
      // find it!
      if (! (it == (*i)->end() || it->second.kind() == None) ) {
        // found it.  So reset the scope to here.
        _scope.erase(_scope.begin(),i);
        _current = &it->second;
        return; // found it.  done!
      }
    }
    // key error, nothing found.
    _current = NULL;
  }

  void reader::follow(const std::string &s) {
    tokenizer::syntax S("[].!","");
    std::istringstream In(s);
    tokenizer t(In,S);
    t.next();
    while( t.current().kind() != token::End ) {
      // bounce search if symbol without a dot
      if (t.current().kind()==token::Symbol) {
        // here we do a scoped search for the leading key
        bounce(t.current().text()); // opt = true
        t.next();
      }
      // descending searches . symbols
      else if (t.current().syntax()=='.'
               && t.next().kind() == token::Symbol) { 
        descend(t.current().text());
        t.next();
      }
      // descending searches . symbols
      else if (t.current().syntax()=='!') {
        if (lost()) _scope.clear(); // permanently lost
        t.next();
      }
      // vector index [ N ]
      else if (t.current().syntax()=='[') { 
        if (t.next().kind()!=token::Symbol) goto badquery;
      
        size_t offset = strtoul(t.current().text().c_str(),NULL,10);
      
        if (t.next().syntax()!=']') goto badquery;
        t.next();

        descend(offset);
      }
      else goto badquery;
    }
    return;
  badquery:
    throw exception(std::string("malformed ark query: ") + s);
  }

  reader reader::get(const std::string &s) const try {
    reader ret(*this);
    ret.follow(s);
    return ret;
  }
  catch (exception &e) {
    std::stringstream ss;
    ss << "Failed to get '" << s << "': " << e.what();
    throw badsearch(ss.str());
  } 

  std::string reader::report() const {
    std::ostringstream ss;
    ss << "Search history: " << _history << std::endl;
    return ss.str();
  }

#define EXPECT_KIND(want) do { \
  kind_t got = top().kind(); \
  if (want!=got) { \
    std::stringstream ss; \
    ss << "expected " << kindstr(want) << ", found " << kindstr(got) \
       << ".\n" << report(); \
    throw badsearch(ss.str()); \
  } \
} while(0)

  const table_t & reader::table() const {
    EXPECT_KIND(Table);
    return top().table();
  }
  const vector_t & reader::vector() const {
    EXPECT_KIND(Vector);
    return top().vector();
  }
  const atom_t & reader::atom() const {
    EXPECT_KIND(Atom);
    return top().atom();
  }
#undef EXPECT_KIND

  template<> Ark::reader::operator const char * () const {
    return atom().c_str();
  }
  template<> Ark::reader::operator bool () const {
    std::string s(str());
    if      (s=="true")  return true;
    else if (s=="false") return false;
    std::stringstream sst;
    sst << "unable to parse boolean '" << s.c_str() 
        << "' ('true' or 'false').\n" << report();
    throw exception(sst.str());
  }

#define defnOperator(T)                         \
  template<> Ark::reader::operator T () const { \
    return wrapStringCast< T >(#T,*this);       \
  }

  namespace { // wrap up string cast struff
    template <typename T> inline
    T wrapStringCast(const char *type,const Ark::reader &a) try {
      return string_cast::stringTo<T>(a.str());
    }
    catch( string_cast::bad_string_cast &bsc) {
        std::stringstream sst;
        sst << "bad string cast: unable to parse a " << type << " from " 
            << string_cast::toString(static_cast<const ark &>(a))
            << ".\n" << a.report();
        throw exception(sst.str());
    }
  }
  defnOperator(signed char)
  defnOperator(unsigned char)
  defnOperator(int)
  defnOperator(unsigned int)
  defnOperator(long)
  defnOperator(unsigned long)
  defnOperator(float)
  defnOperator(double)
  defnOperator(long double)

#undef defnOperator

}
