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
  T(const ark& a){
    switch(a.kind()){
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
  
  printer P;
  P.whitespace(1);

  try{
      // First a plain-old conversion
      double d = arkTo<double>(a, "z");
      cout << "z = " << d << "\n";

      // If you prefer to do the xget yourself:
      d = arkTo<double>(*a.xget("z"));
      cout << "z = " << d << "\n";

      // A conversion with a default
      d = arkTo<double>(a, "ggg.rrr[3]", 19.);
      cout << "ggg.rr[3] (or default 19.): " << d << "\n";

      // Copy into an array (only the first three elements)
      int ai[3];
      int *end = arkToIter<int>(a, "array", &ai[0], 3);  
      if(end-ai != 3)
          throw std::runtime_error("Expected at least three elemements in \"array\"");

      cout << "ai: "; copy(&ai[0], &ai[3], ostream_iterator<int>(cout, " ")); cout << "\n";

      // and copy into a vector (all the elements)
      vector<int> vi;
      arkToIter<int>(a, "array", back_inserter(vi));
      // N.B.  array[3] = "0x27" and array[4] = 010 in testfile.  Check that
      // they are converted correctly into ints and printed in decimal.
      cout << "vi: "; copy(vi.begin(), vi.end(), ostream_iterator<int>(cout, " ")); cout << "\n";
      
      // If the target type T has a T(const ark&) constructor, we
      // use it rather than string_cast.
      T t;
      t = arkTo<T>(a, "array");
      cout << "array: " << t << "\n";

      t = arkTo<T>(a, "file");
      cout << "file: " << t << "\n";

      // Or we can read it directly into a string.
      string s = arkTo<string>(a, "file");
      cout << "file: " << s << "\n";

      vector<T> vt;
      arkToIter<T>(a, "x[1].tau", back_inserter(vt));
      cout << "vi: "; copy(vt.begin(), vt.end(), ostream_iterator<T>(cout, " ")); cout << "\n";
      

  }catch(Ark::exception &e){
      cout << "Caught: " << e.what() << "\n";
  }

  return 0;
}
