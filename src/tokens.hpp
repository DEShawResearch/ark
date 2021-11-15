#ifndef ark_tokens_hpp
#define ark_tokens_hpp

#include <string>
#include <iostream>

/*! \file ark/tokens.hpp

One of the auxiliary classes used by Ark is a simple character-based
tokenizer for parsing inputs and queries.
*/

namespace Ark {
  /*! A token is a variadic type representing a chunk of input, delimited
   by "syntax characters" or a single "syntax character". */
  class token {
    friend class tokenizer;
  public:
    //! The things a token might be.
    enum kind_t {
      None=0, //!< Used to initialize.
      End,    //!< End of input stream.
      Syntax, //!< A syntax character.
      Symbol, //!< A maximal non-whitespace, non-syntax substring of input.
      String  //!< A quote (special syntax characters) delimited substring.
    };
  private:
    kind_t k; //!< The kind of this token.
    std::string buf; //!< A temporary buffer for input.

  public:
    //! default initialized the kind to None.
    token() : k(None), buf("") {}
    /*!
      returns the kind.
      @return the kind of this token.
    */
    kind_t      kind() const {return k;}

    /*! Return the syntax character of this token or char(0) if this is
      a non-syntax token.
      @return syntax character.
    */
    char syntax() const {return (k==Syntax) ? (buf[0]) : ('\0');}
    /*! Return the text of this token as a C++-string.
      @return the text of this token.
    */
    std::string text() const;
  };


  /*! A tokenizer produces a stream of tokens from a C++ input stream.
    The interface is a basic iteration scheme supporting a call like
    <code>
\verbatim
    for(tokenizer t(cin,syn); t.next().current() != End; ) {
      switch(t.current().kind()) {....}
    }
\endverbatim
    </code>
  */
  class tokenizer {
  public:
    //! Syntax information is held in an internal class.
    class syntax {
      // syntax information
      std::string comment; //!< The start of comment character.  Comments
                           //!< currently start from a comment character and
                           //!< go until a newline.  Might change this some
                           //!< day.
      std::string syntax_chars; //!< syntax characters (non-quote).
      std::string quote; //!< allowed quote characters for string.
      std::string reserved; //!< syntax+quote+comment concatenation.
    public:
      /*! Constructor.
        @param syn syntax characters.
        @param com comment characters.

        Currently quote characters are all of "'`, hard-coded.
      */
      syntax(const std::string &syn,const std::string &com):
        comment(com), syntax_chars(syn), quote("\"'`"),
        reserved(comment+syntax_chars+quote)
      {}
      //! test for comment character.
      //! @param c
      //! @return whether a comment.
      bool is_comment(char c) const {
        return std::string::npos != comment.find(c);
      }
      //! test for syntax character.
      //! @param c
      //! @return whether a syntax.
      bool is_syntax(char c) const {
        return std::string::npos != syntax_chars.find(c);
      }
      //! test for quote character.
      //! @param c
      //! @return whether a quote.
      bool is_quote(char c) const {
        return std::string::npos != quote.find(c);
      }
      //! test for reserved character.
      //! @param c
      //! @return whether a reserved.
      bool is_reserved(char c) const {
        return std::string::npos != reserved.find(c);
      }
    };

  private:
    syntax S;
    // tokenizing state
    std::istream &in;
    unsigned      line,col;
    token         t;

    void nextch(void); // yes this BS does help.  C++ iostreams blow.
    static const size_t BUFsize=256;
    char *C,*E,BUF[BUFsize];

  public:
    /*! Construct a tokenizer from an input stream and a syntax.
      @param i input stream.
      @param syn syntax.

      Initializes some internal meta-information about the stream to
      track line numbers and such.

      The initial value of current() is None.
    */
    tokenizer(std::istream &i,syntax syn)
      : S(syn),in(i),line(1),col(0),t() {
      C=BUF;
      in.get(C,BUFsize,0);
      E=C+in.gcount();
    }
    //! The current line number in the stream.
    //! @return the line number.
    unsigned lineno() const {return line;}
    //! The current column number in the stream.
    //! @return the column number.
    unsigned colno()  const {return col; }

    //! The current token in the stream.
    //! @return a token reference.
    const token &current() const {return t;}

    //! The next token in the stream according to a given syntax.
    //! @param s given syntax.
    //! @return token reference.
    const token &next(const syntax &s);
    //! The next token in the stream according the syntax from the
    //! constructor.
    //! @return token reference.
    inline const token &next() { return next(S); }
  };
}

#endif
