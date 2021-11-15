#include <ark/ark.hpp>

using namespace Ark;

int main(int argc,char **argv) {

  ark a,b;

  a = "hey";

  a.be(Vector).vector().push_back("hello");
  a.be(Vector).vector().push_back("world");

  a.be(Table);
  a.table()["global_cell"].be(Table);
  a.table()["global_cell"].table()["partition"].be(Vector);
  a.table()["global_cell"].table()["partition"].vector().push_back("1");
  a.table()["global_cell"].table()["partition"].vector().push_back("3");
  a.table()["global_cell"].table()["partition"].vector().push_back("9");
  a.table()["global_cell"].table()["partition"].vector()[2] = "7";

  fdump(stdout,a);
  printf("\n");

  fdump(stdout, parse("{ x=[1 2 3] z=? y={z = hey w='string string'}}"));
  printf("\n");

  a.table()["sigma"] = "1.234";
  a.table()["topology"] = "january";

  b.be(Vector);
  b.vector().push_back("10");
  b.vector().push_back("41");
  b.vector().push_back("21");

  a.table()["global_cell"].be(Table).table()["partition"].swap(b);

  fdump(stdout,a);
  printf("\n");
  fdump(stdout,b);
  printf("\n");
  
  const ark *t;
  if ((t=a.get("global_cell"))) {
    fdump(stdout,*t);
    printf("\n");
    if ((t=t->get("partition"))) {
      fdump(stdout,*t);
      printf("\n");
    }
  }
  if ((t=a.get("global_cell")->get("partition"))) {
    fdump(stdout,*t);
    printf("\n");
  }
  const atom_t *c;
  if ((c=a.get("global_cell")->get("partition")->get(1)->get_atom()))
    printf("%s\n",c->c_str());
}
