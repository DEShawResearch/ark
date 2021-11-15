#ifndef ark_parser_hpp
#define ark_parser_hpp

#include "base.hpp"
#include "tokens.hpp" // for my private members

/*! \file ark/parser.hpp

  The parser grammar is a superset of the strict grammar (again
  we don't have any standard for parsing boxes).  This superset was
  chosen to allow for shorthands which users would need to invoke from the
  command line, includes to allow online insertion of files, and
  overrides to allow some primitive inheritance/reuse.
  <code>
\verbatim
---PARSER GRAMMAR---
ARK  -> NONE | STRING | [ARK*] | { KEYVAL* }
NONE -> ?
STRING is a ", ', or ` delimited string with simple '\' meta-charactering
       or a bare string (no intervening white-space or grammar symbols).
KEYVAL  -> INCLUDE | SKEY=ARK | SKEY ARK
  the first form does assignment, the second form does an "enclosure",
  expecting ARK to be a table and then making the SKEYed element a
  table with additional keys taken from ARK.
INCLUDE -> ! include STRING
  expands the file named by STRING, parsing the contents as KEYVAL*
SKEY    -> IKEY | SKEY . IKEY
  keys which name tables can be subindexed with . in a familiar way
IKEY    -> KEY | IKEY [ INDEX ]
  keys which name vectors are indexed in similarly to arrays.
KEY is a non-delimited string in the regexp [_a-xA-Z][_a-zA-Z0-9]*
INDEX   -> + | INT
  if + the index is taken to be the end+1 element of the array, i.e.
  the assignment is taken to be a 'push_back' operation.
INT is a base 10 non-negative integer.
\endverbatim
  </code>
*/

namespace Ark {

/*! parser defines a helper type used for parsing arks.  In theory,
  having a helper type for the parser allows some user-level control
  over the parsing.  In practice, the parser type isn't that
  malleable, and just serves as a place to hold auxilliary parsing
  state which possibly could be done another way.

  Example:
<code>
\verbatim
  Ark::ark b;
  parser().parse_file(b,argv[1]);
\endverbatim
</code>

  This style has some similarities with ark::printer, but since member
  functions that modify the parsed syntax are strongly discouraged,
  it's not clear how far this similarity can or should be pushed.  An
  alternative parser is, for all intents and purposes, an alternative
  ark language, so unless one can justify "balkanizing" the ark
  language, alternative parsers will be rejected.

  A parser takes an ark to update.  It does not clear its input, but
  merely overrides it.  If this is not what the user wants, the user
  should call the ark::clear() member before using the parser.
*/
  class parser {
    unsigned include_depth;    //!< current include file depth.
    const char *current_file;  //!< name of current include file.

    ark parse_value(tokenizer &t) const;

    void descend(ark &a,tokenizer &t) const;
    void parse_keyvalue(ark &a,tokenizer &t) const;
    void include_file(ark &a,const std::string &f) const;

  public:
    static tokenizer::syntax key_syn; //!< punctuation for tokens in a key context.
    static tokenizer::syntax val_syn; //!< punctuation for tokens in a value context.
    static tokenizer::syntax no_syn;  //!< punctuation for tokens any other context.

    /*! Constructor.
      Sets the syntax and initializes the include file meta-information.
    */
    parser() : include_depth(0),current_file(NULL) {};
    /*! parse into an ark from a C++ stream.
      Starts from the production rule for ARK in the grammar.
      @param a input ark.
      @param in input stream.
      @return reference to a.
    */
    ark &parse(ark &a,std::istream &in) const;
    /*! parse() via stringstream.
      @param a input ark.
      @param s input string.
      @return reference to a.
    */
    ark &parse(ark &a,const std::string &s) const;

    /*! parse into a sequence of key-value pairs from a C++ stream.
      Useful for reading config files.
      Starts from the production rule for KEYVAL* in the grammar.
      @param a input ark.
      @param in input stream.
      @return reference to a.
    */
    ark &parse_keyvals(ark &a,std::istream &in) const;
    /*! parse_keyvals() via stringstream.
      @param a input ark.
      @param s input string.
      @return reference to a.
    */
    ark &parse_keyvals(ark &a,const std::string &s) const;

    /*! parse the contents of a file as a sequence of key-value pairs.
      Useful for reading config files.
      Starts from the production rule for KEYVAL* in the grammar.
      Checks include depth and updated the include file meta-information.
      @param a input ark.
      @param s file to read.
      @return reference to a.
    */
    ark &parse_file(ark &a,const std::string &s) const {
      include_file(a,s);
      return a;
    }
  };
}



#endif
