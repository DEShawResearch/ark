#include "printer.hpp"
#include "parser.hpp" // for syntax definitions

#include <sstream>
#include <climits>

namespace Ark {

  static bool requires_quotes(atom_t const& a) {
    if (a.c_str()[0] == '\0') return true;
    tokenizer::syntax const& syn = parser::val_syn;
    for (const char* c = a.c_str(); *c; ++c) {
      if (syn.is_reserved(*c) || isspace(*c) || *c=='\\') return true;
    }
    return false;
  }

  void fdump(FILE *f,const ark &a) {
    switch(a.kind()) {
    case None:
      fprintf(f,"?");
      return;
    case Atom:
      fprintf(f,"\"%s\"",a.atom().c_str());
      return;
    case Vector: {
      fprintf(f,"[");
      vector_t::const_iterator i;
      for(i=a.vector().begin(); i != a.vector().end(); ++i)
	fdump(f,*i);
      fprintf(f,"]");
      return;
    }
    case Table: {
      fprintf(f,"{");
      table_t::const_iterator i;
      for(i=a.table().begin(); i != a.table().end(); ++i) {
	fprintf(f,"%s=",i->first.c_str());
	fdump(f,i->second);
      }
      fprintf(f,"}");
      return;
    }
    }
  }

  namespace {
    void space(std::ostream &o,unsigned sp) {
      for(unsigned j=0; j < sp; ++j) o<<" ";
    }
  }

  void printer::printer_ref::output_flatten(const std::string &key,
                                               const ark &a,
                                               std::ostream &o,
                                               unsigned ind) const {
    const bool whitespace = flags.whitespace;
    const unsigned tab = flags.indent;
    if (a.kind()==None || a.kind()==Atom ||
        (a.kind()==Vector && a.vector().empty()) ||
        (a.kind()==Table && a.table().empty()) ) {

      if (whitespace) space(o,tab*ind);

      o<<key;
      if (whitespace) o<<" = ";
      else            o<<"=";

      switch(a.kind()) {
      case None: (o<<"?"); break;
      case Atom: {
        std::ostringstream oss;
        const char* s = a.atom().c_str();
        if (from_python && (unsigned char)(*s) == 0xff) {
            oss << "!file ";
            ++s;
        }
        const bool with_quotes = !whitespace || requires_quotes(a.atom());
        if (with_quotes) oss<<"\"";
        for(; *s ; oss<<*s, ++s)
          if (*s=='\\' || *s=='"') oss<<"\\";
        if (with_quotes) oss<<"\"";
        o<<oss.str();
      } break;
      case Vector: (o<<"[]"); break;
      case Table: (o<<"{}"); break;
      }

      if (whitespace) o<<"\n";
    }
    else if (a.kind()==Vector) {
      for(size_t i=0; i < a.vector().size(); ++i) {
        std::stringstream sst;
        sst << key << "[" << i << "]";
        output_flatten(sst.str(), a.vector()[i], o, ind);
      }
    }
    else if (a.kind()==Table) {
      for(table_t::const_iterator p=a.table().begin();
          p != a.table().end(); ++p)
        output_flatten(key + "." + p->first.str(),p->second,o,ind);
    }
    else {
      // THROW?
    }
  }

  void printer::printer_ref::output(const ark &a,
                                    std::ostream &o,
                                    unsigned &ind,unsigned &col,
                                    bool top) const {
    const bool delim = !(flags.no_delim && top);
    const bool whitespace = flags.whitespace;
    const bool open_tables = flags.open_tables;
    const bool flatten = flags.flatten;
    const unsigned width = flags.width?flags.width:INT_MAX;
    const unsigned tab = flags.indent;

    switch(a.kind()) {
    case None:
      (o<<"?");
      break;
    case Atom:
      if (delim) {
        std::ostringstream oss;
        const char* s = a.atom().c_str();
        if (from_python && (unsigned char)(*s) == 0xff) {
            oss << "!file ";
            ++s;
        }
        const bool with_quotes = !whitespace || requires_quotes(a.atom());
        if (with_quotes) oss<<"\"";
        for(; *s ; oss<<*s, ++s)
          if (*s=='\\' || *s=='"') oss<<"\\";
        if (with_quotes) oss<<"\"";
        o<<oss.str(), col+=oss.str().size();
      }
      else { // no-delimited atoms don't escape
        const char* s = a.atom().c_str();
        if (from_python && (unsigned char)(*s) == 0xff) {
            o << "!file ";
            ++s;
        }
        o << s;
        col += a.atom().str().size() + 5;
      }
      break;
    case Vector: {
      if (delim) o<<"[", ++col;
      unsigned rem=col;
      for(vector_t::const_iterator i=a.vector().begin();
          i != a.vector().end(); ++i) {

        if (whitespace &&
            i!=a.vector().begin()) {
          if (col > width) o<<"\n", col=rem, space(o,col);
          else o<<" ", ++col;
        }

        output(*i,o,ind,col,false);
      }
      if (delim) o<<"]", ++col;
    } break;
    case Table: {
      if (delim) {
        ++ind;
        o<<"{",++col;
        if (whitespace) o<<"\n";
      }
      for(table_t::const_iterator p=a.table().begin();
          p != a.table().end(); ++p) {
        if (flatten) {
          output_flatten(p->first.str(),p->second,o,ind);
        }
        else {
          if (whitespace) col = tab*ind, space(o,col);

          o<<p->first.str(), col+=p->first.str().size();
          if( open_tables && p->second.kind() == Table ) {
            if (whitespace) o<<" ", ++col;
            else /* pass */ ;
          }
          else {
            if (whitespace) o<<" = ", col+=3;
            else            o<<"=", ++col;
          }
          output(p->second,o,ind,col,false);
          
          if (whitespace) o<<"\n";
        }
      }
      if (delim) {
        --ind;
        if (whitespace) col = tab*ind, space(o,col);
        o<<"}",++col;
      }
    } break;
    }
  }

  namespace { // simple FILE* to ostream adapter
    struct FILEbuf : std::basic_streambuf< char, std::char_traits< char > > {
      FILE *_f;
      FILEbuf(FILE *f) : _f(f) {}

      // For EOF detection above
      typedef std::char_traits< char > Traits;

      // This is called every time characters are put to stream.
      int overflow(int c = Traits::eof()) {
        // Handle output
        if (c != Traits::eof()) fputc(c,_f);
        // I'm not sure about this return value!
        return 0;
      }
      
      // This function is called when stream is flushed,
      // for example when std::endl is put to stream.
      int sync(void) { return fflush(_f); }
    };

    struct FILEstream : std::basic_ostream< char, std::char_traits< char > > {
      FILEbuf buf;
      FILEstream(FILE *f) :
        std::basic_ostream< char, std::char_traits< char > >(&buf),buf(f) {}
    };
  }

  void printer::printer_ref::fprint(FILE *f) const {
    FILEstream fs(f);
    output(fs);
  }

}

std::ostream &operator<<(std::ostream &o,const Ark::printer::printer_ref &x) {
  x.output(o); return o;
}
