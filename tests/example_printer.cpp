#include <ark/ark.hpp>

#include <cstdlib>
#include <iostream>

using namespace Ark;
using namespace std;

int main(int argc,char **argv) {

  ark a;

  if (argv[1]) {
    try { parser().parse_file(a,argv[1]); }
    catch (...) {
      fprintf(stderr,"parse of a failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile" << endl;
    exit(1);
  }
  try {
    printer P;

    cout << P(a) << endl;
    P(a).fprint(stdout); printf("\n");

    P.no_delim(1).whitespace(1);
    cout << P(a) << endl;
    P(a).fprint(stdout); printf("\n");

    P.no_delim(1).whitespace(0);
    cout << P(a) << endl;
    P(a).fprint(stdout); printf("\n");

    P.no_delim(0).whitespace(1);
    cout << P(a) << endl;
    P(a).fprint(stdout); printf("\n");
  }
  catch (const Ark::exception &e) {
    fprintf(stderr,"caught: %s\n",e.what());
  }
  return 0;
}
