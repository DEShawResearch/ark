#include <ark/ark.hpp>

#include <cstdlib>
#include <iostream>

using namespace Ark;
using namespace std;

int main(int argc,char **argv) {

  ark a;
  ark b;

  if (argv[1]) {
    try { parser().parse_file(a,argv[1]); }
    catch (Ark::exception &e) {
      fprintf(stderr,"Ark error\n%s",e.what());
    }
    catch (...) {
      fprintf(stderr,"parse of b failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile mergefile" << endl;
    exit(1);
  }

  if (argv[2]) {
    try { parser().parse_file(b,argv[2]); }
    catch (Ark::exception &e) {
      fprintf(stderr,"Ark error\n%s",e.what());
    }
    catch (...) {
      fprintf(stderr,"parse of b failed\n");
      exit(1);
    }
  }
  else {
    cerr << "usage: " << argv[0] << " testfile mergefile" << endl;
    exit(1);
  }
  
  a.merge(b);

  printer P;
  P.no_delim(1).whitespace(1);
  cout << P(a) << endl;

  return 0;
}
