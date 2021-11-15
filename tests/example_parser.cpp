#include <ark/ark.hpp>

#include <cstdlib>
#include <iostream>

using namespace Ark;
using namespace std;

int main(int argc,char **argv) {

  ark b;

  if (argv[1]) {
    try { parser().parse_file(b,argv[1]); }
    catch (Ark::exception &e) {
      fprintf(stderr,"Ark error\n%s",e.what());
    }
    catch (...) {
      fprintf(stderr,"parse of b failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile" << endl;
    exit(1);
  }

  //parser().parse_file(b, "%s/death.ark");

  printer P;
  P.no_delim(1).whitespace(1);
  cout << P(b) << endl;

  return 0;
}
