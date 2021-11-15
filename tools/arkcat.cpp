#include "base.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "exception.hpp"
#include "argv.hpp"
#include <cstdlib>
#include <functional>

static void usage(const std::string& program, int exstatus) {
  std::cerr << "usage: " << program 
            << " [--help]"
            << " [--[no_]delim]"
            << " [--[no_]whitespace]"
            << " [--[no_]open_tables"
            << " [--include file]*"
            << " [--cfg line]*"
            << " [--flatten]"
            << std::endl
            << std::endl
            << "    --help              : print this message\n"
            << "    --delim             : add outer {} or [] to top-level list or table\n"
            << "    --whitespace        : enhance human readability\n"
            << "    --width INT         : set linewrap threshold\n"
            << "    --flatten           : ouput in 'dotted' notation rather than 'tabular' notation\n"
            << "    --open_tables       : print tables as key{...} rather than key={...}\n"
            << "    --include file      : Include this file as a table\n"
            << "    --cfg line          : parse given line as a table (see below)\n"
            << "    --cfg -             : (special case) parse stdin as a table\n"
    
            << std::endl
            << "Build an ark from include files and explicit config keys"
            << std::endl;

  exit(exstatus);
}

int main(int argc, char** argv) {
  Ark::ark ark;
  Ark::parser parser;
  Ark::printer printer;

  // set printer defaults
  printer.no_delim(true);
  bool whitespace=true;
  printer.whitespace(whitespace);

  // Demonstrate using argvremove_copy to defer the call to arkparse
  // until after the non-ark processing.
#if 1
  std::vector<const char *> nonarkargs;
  Ark::argvremove_copy(argv, argv+argc, std::back_inserter(nonarkargs));
  for(std::vector<const char *>::iterator p=nonarkargs.begin()+1; p!=nonarkargs.end(); ++p){
    std::string text = *p;
#else
  // or, if you don't like fancy STL:
  const char ** nonarkv = new const char*[argc+1]; // don't forget to delete[] it...
  int nonarkc = Ark::argvremove_copy(argv, argv+argc+1, nonarkv) - nonarkv - 1;
  for(int i=1; i<nonarkc; ++i) {
    std::string text = nonarkv[i];
#endif

    if (text == "--help") {
      usage(argv[0], 0);
    }
      
    else if (text == "--delim") {
      printer.no_delim(false);
    }

    else if (text == "--no_delim") {
      printer.no_delim(true);
    }

    else if (text == "--whitespace") {
      whitespace = true;
      printer.whitespace(whitespace);
    }

    else if (text == "--no_whitespace") {
      whitespace = false;
      printer.whitespace(whitespace);
    }

    else if (text == "--open_tables") {
      printer.open_tables(true);
    }

    else if (text == "--no_open_tables") {
      printer.open_tables(false);
    }

    else if (text == "--width") {
      if (++p!=nonarkargs.end())
        printer.width(atoi(*p));
      else
          usage(argv[0], 1);
    }

    else if (text == "--flatten") {
      printer.flatten(true);
    }

    else
        usage(argv[0], 1);
  }


  Ark::argvparse(ark, argv, argv+argc);

  // Print
  std::cout << printer(ark);
  // newline if no whitespace
  if (!whitespace) std::cout << std::endl;

  return 0;
}
