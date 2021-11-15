#include <ark/parser.hpp>
#include <ark/printer.hpp>

#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace Ark;
using namespace std;

namespace Ark { // really, if you want to do your checking don't pollute base

  std::string xgetstr(const ark &a,const std::string &q) {
    const ark * b = a.xget(q);
    if (!b)
      throw exception("ark_xgetstr: key: " + q + " not found");
    const atom_t * c = b->get_atom();
    if (!c)
      throw exception("ark_xgetstr: key: " + q + " not an atom");
    return c->str();
  }

}

std::string annotation(const ark &a) {
#ifdef ANARKY_NOW
  std::ostringstream ss;
  ss << "<" << a.annotation().file << ":" << a.annotation().lineno << "/" << a.annotation().colno << ">";
  return ss.str();
#else
  return "";
#endif
}

int main(int argc,char **argv) {

  ark a;

  if (argv[1]) {
    try { parser().parse_file(a,argv[1]); }
    catch (...) {
      fprintf(stderr,"parse failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile" << endl;
    exit(1);
  }
  

  const ark *t;
  printer P;
  P.whitespace(1);

  cout << P(a) << endl;
  cout << "#####queries#####" << endl;

  if ( (t = a.xget("ggg")) )
    cout << "ggg" << " is " << P(*t) << annotation(*t) << endl;

  if ( (t = a.xget("ggg.rrr")->get(1)) )
    cout << "ggg.rrr[1]" << " is " << P(*t) << annotation(*t) << endl;

  if ( (t = a.xget("ggg.rrr")->get(2)) )
    cout << "ggg.rrr[2]" << " is " << P(*t) << annotation(*t) << endl;

  // Or xgetstr
  try {
      cout << "ggg.rrr[2]" << " is " << xgetstr(a,"ggg.rrr[2]") << endl;
      cout << "ggg.rrr[1]" << " is " << xgetstr(a,"ggg.rrr[1]") << endl; // throws with ./testfile
  }catch(Ark::exception &e){
      cout << "Caught: " << e.what() << "\n";
  }

  // misspelled
  if ( (t = a.xget("ggg.rr[1]")) )
    cout << "ggg.rr[1]" << " is " << P(*t) << annotation(*t) << endl;

  if ( (t = a.xget("x[1].tau")) )
    cout << "x[1].tau" << " is " << P(*t) << annotation(*t) << endl;

  return 0;
}
