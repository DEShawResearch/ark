#include "base.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "exception.hpp"
#include "argv.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdlib>

using namespace std;

static void usage(const std::string& program) {
    std::cerr << "usage: " << program << "[[--Option | outputKeyPath] ... ]\n"
              << "Build an ark from include files and explicit config keys.\n"
              << "--include and --cfg args are parsed first\n"
              << "The remaining arguments are processed in order.\n"
              << "OutputKeyPaths are looked up in the ark and are printed using the delim and whitespace flags in effect at\n"
              << "the point that the key appears on the command line\n"
              << "Option: \n"
              << "    --help              : Print this message\n"
              << "    --[no_]delim        : Strip outer {} or \"\" (default no_delim)\n"
              << "    --[no_]whitespace   : Enhance human readability (default whitespace)\n"
              << "    --[no_]open_tables   : print tables as key{...} rather than key={...}\n"
              << "    --width INT         : set linewrap threshold\n"
              << "    --include file      : Include this file (many)\n"
              << "    --cfg keypath=value : Insert this path.to.key=value (many)\n"
              << "    outputKeyPath       : Search for this keypath in the resulting ark.\n"
                 "                          Print the value to stdout, followed by a newline.\n"
                 "                          If not specified or if zero length, print the whole ark\n"
            << std::endl;
}


int main(int argc, char** argv)  try {
  Ark::ark ark;
  Ark::parser parser;
  Ark::printer printer;
  std::string outfile;
  bool printed_something = false;

  printer.no_delim(1).whitespace(1);

  Ark::argvparse(ark, argv, argv+argc);
  char **end = Ark::argvremove(argv, argv+argc);
  argc = end-argv;
  for(int i=1; i<argc; ++i){
    std::string text = argv[i];
  
    if (text == "--help") {
      usage(argv[0]);
      return 0;
    }
      
    else if (text == "--no_delim") {
      printer.no_delim(1);
    }

    else if (text == "--delim") {
      printer.no_delim(0);
    }

    else if (text == "--whitespace") {
      printer.whitespace(1);
    }

    else if (text == "--no_whitespace") {
      printer.whitespace(0);
    }

    else if (text == "--width") {
      if (++i<argc)
        printer.width(atoi(argv[++i]));
      else {
        usage(argv[0]);
        return 1;
      }
    }

    else if (text == "--open_tables") {
      printer.open_tables(1);
    }

    else if (text == "--no_open_tables") {
      printer.open_tables(0);
    }

    else {
      if( text.empty() ){
        // An empty string means print the whole thing.
        std::cout << printer(ark) << std::endl;
      }else{
        const Ark::ark *a = ark.xget(text);
        if( a )
          std::cout << printer(*a) << std::endl;
        else{
          std::cerr << "Key: " << text << " not found in ark\n";
          return 1;
        }
      }
      printed_something = true;
    }
  }

  // If we haven't printed anything yet, there weren't any getkeys on 
  // the command line.  Print the whole thing.
  if(!printed_something)
    std::cout << printer(ark) << std::endl;

  return 0;
}
 catch(std::exception &e){
   std::cerr << "Exception thrown: " << e.what() << "\n";
   return 1;
 }
