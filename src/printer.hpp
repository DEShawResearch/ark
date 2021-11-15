#ifndef ark_printer_hpp
#define ark_printer_hpp

#include "base.hpp"

#include <cstdio>
#include <ostream>

/*! \file ark/printer.hpp

Although the ark class has an ostream output method, a variety of
modifications to the output 'style' might be warranted, such as the
addition of white-space for pretty printing or the removal of outer
delimiters for 'config file' generation.  Maybe you'd like to write to
a C FILE*.  To support these modified forms, we define a printer class
which interacts in nice ways with C++ iostreams and C FILE*.

Example:
<code>
\verbatim
    Ark::ark a;
    Ark::printer P;
    P.no_delim(1).whitespace(1);
    cout << P(a) << endl;
    cout << P.no_delim(0)(a) << endl;
\endverbatim
</code>
*/

namespace Ark {
  /*! Consider this a 'debug only' sort of output which isn't intended
    to look very good.
    @param f C-style stream.
    @param a output ark.
  */
  void fdump(FILE *f,const ark &a);

  //! The collection of output modification flags currently recognized.
  struct print_flags {
    bool  no_delim;   //!< omit delimiters of outermost ark element
    bool  whitespace; //!< insert whitespace for pretty printing.
    bool  open_tables;//!< print tables as key {...} rather than key = {...}
    bool  flatten;    //!< display as extended-key = value pairs.
    unsigned width;   //!< start wrapping lines once this width is exceeded
    unsigned indent;  //!< number of spaces per indent
    //! default initializes everything to false.
    print_flags() : no_delim(0),whitespace(0),open_tables(0),flatten(0),
                    width(0),indent(4) {}
  };

  //! A printer is thought of as "something that knows how to print arks".
  class printer {
    print_flags flags; //!< current set of printing flags.

  public:
    //! A printer generates a print_ref when given an ark.
    //! printer_refs get sent to ostreams or take FILE*s.
    class printer_ref {
      print_flags flags; //! flags of the printer.
      const ark &a; //! the ark to print.
      const bool from_python;

    private:
      void output(const ark &a,std::ostream &o,
                  unsigned &ind,unsigned &col,bool top) const;
      void output_flatten(const std::string &k,const ark &a,std::ostream &o,
                          unsigned ind) const;

    public:
      //! Construct from flags and ark.
      //! @param f flags to print with.
      //! @param A ark to print.
      printer_ref(const print_flags &f,const ark &A, bool from_python=false) : flags(f), a(A), from_python(from_python) {}
      //! public function to be called by operator<<().
      //! @param o output.
      void output(std::ostream &o) const { 
        unsigned i=0,c=0;
        output(a,o,i,c,true);
      }
      //! public function to be called on C-style streams.
      //! @param f C-style stream.
      void fprint(FILE *f) const;
    };

    //! trivial default constructor.
    printer() {}

    /*! chain-able setter for width.
      @param b new value for width flag.
      @return reference to this printer.
    */
    printer &width(unsigned b) { flags.width = b; return *this; }

    /*! chain-able setter for indent.
      @param b new value for indent flag.
      @return reference to this printer.
    */
    printer &indent(unsigned b) { flags.indent = b; return *this; }

    /*! chain-able setter for whitespace.
      @param b new value for whitespace flag.
      @return reference to this printer.
    */
    printer &whitespace(bool b) { flags.whitespace = b; return *this; }

    /*! chain-able setter for no_delim.
      @param b new value for no_delim flag.
      @return reference to this printer.
    */
    printer &no_delim(bool b) { flags.no_delim = b; return *this; }

    /*! chain-able setter for open_tables
      @param b new value for open_tables flag.
      @return reference to this printer.
    */
    printer &open_tables(bool b) { flags.open_tables = b; return *this; }

    /*! chain-able setter for flatten
      @param b new value for flatten flag.
      @return reference to this printer.
    */
    printer &flatten(bool b) { flags.flatten = b; return *this; }

    /*! Apply printer to an ark and give a printer reference.
      @param a the ark to print.
      @return a printer_ref built from a and flags.
    */
    printer_ref operator()(const ark &a, bool from_python=false) const {
      return printer_ref(flags,a,from_python);
    }
  };
}

/*! C++-stream output (calls printer_ref::output()).
  @param o output stream.
  @param x printer reference.
  @return o
*/
std::ostream &operator<<(std::ostream &o,const Ark::printer::printer_ref &x);

#endif
