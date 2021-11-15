#include <ark/parser.hpp>
#include <ark/printer.hpp>
#include <ark/arkTo.hpp>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <iterator>

using namespace Ark;
using namespace std;

// To demonstrate the behavior of arkTo we need a type that
// is_convertible from an Ark.  For T, the conversion amounts to
// recording the ark's kind in a string.
struct T{
  string s;
  T(){ s = "default_constructed"; };
  T(const Ark::reader& a){
    switch(static_cast<const ark &>(a).kind()){
    case Atom:
      s = "Atom"; break;
    case Vector:
      s = "Vector"; break;
    case Table:
      s = "Table"; break;
    case None:
      s = "None"; break;
    }
  }
};

ostream& operator<<(ostream& s, const T& t){
  return s << t.s;
}

int main(int argc,char **argv) {

  ark A;

  if (argv[1]) {
    try { parser().parse_file(A,argv[1]); }
    catch (...) {
      fprintf(stderr,"parse failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile" << endl;
    exit(1);
  }

  reader a(A);
  printer P;
  P.whitespace(1);

  try{
      // First a plain-old conversion
      double d=0;
      a.get("z").set(d);
      cout << "z = " << d << "\n";

#if 0 // compile tests
      std::string S  = a;
      S = a.operator std::string();
      double db = a;
      db = a;
#endif

      bool bl = a.get("True");
      cout << "True is " << bl << std::endl;

      // A conversion with a default
      d=19.;
      a.get("ggg.rrr[3]").set_opt(d);
      cout << "ggg.rr[3] (or default 19.): " << d << "\n";

      // Copy into an array (only the first three elements)
      int ai[5];
      a.get("array").set(Ark::set_as_array(ai,5));

      //std::string amb=a;
      //ark aa(ark(a));

      cout << "ai: "; copy(&ai[0], &ai[5], ostream_iterator<int>(cout, " ")); cout << "\n";

      // and copy into a vector (all the elements)
      vector<int> vi;
      a.get("array").set(vi);
      // N.B.  array[3] = "0x27" and array[4] = 010 in testfile.  Check that
      // they are converted correctly into ints and printed in decimal.
      cout << "vi: "; copy(vi.begin(), vi.end(), ostream_iterator<int>(cout, " ")); cout << "\n";
      
      // If the target type T has a T(const ark&) constructor, we
      // use it rather than string_cast.
      T t;
      a.get("array").set(t);
      cout << "array: " << t << "\n";

      a.get("file").set(t);
      cout << "file: " << t << "\n";

      // Or we can read it directly into a string.
      string s = "";
      a.get("file").set(s);
      cout << "file: " << s << "\n";

      vector<T> vt;
      reader b = a.get("x[1]");
      b.get("ert").set(s);
      cout << "x[1] , ert: " << s << "\n";
      
      vt.clear();
      a.get("x[1].tau").set(vt);
      cout << "vt: "; copy(vt.begin(), vt.end(), ostream_iterator<T>(cout, " ")); cout << "\n";

  }catch(Ark::exception &e){
      cout << "Caught: " << e.what() << "\n";
  }

  return 0;
}
