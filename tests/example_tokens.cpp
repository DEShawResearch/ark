#include <ark/tokens.hpp>
#include <ark/exception.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace Ark;
using namespace std;

int main(int argc,char **argv) {
  if (argv[1]) {
    try {
      tokenizer::syntax S("()[]<>{}.?!=","#");
      ifstream F(argv[1]);
      for(tokenizer T(F,S); T.next().kind() !=token::End; ) {
        std::cout << T.current().text()
                  << " at line " << T.lineno()
                  << " and col " << T.colno() 
                  << " of file " << argv[1] << std::endl;
      }
    }
    catch (const Ark::exception &e) {
      fprintf(stderr,"Caught: '%s'\n",e.what());
      exit(1);
    }
  }
  else {
    fprintf(stderr,"usage: %s testfile\n",argv[0]);
    exit(1);
  }
}
