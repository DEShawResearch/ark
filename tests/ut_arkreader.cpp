#include "../src/reader.cpp"    // errr...
#include <list>
#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace Ark;

typedef std::pair<std::string,std::string> QueryResult;
typedef std::list<QueryResult>             Battery;
typedef std::pair<std::string,Battery>     ArkBattery;
typedef std::list<ArkBattery>              Test;
int main() {

  Test test = {
    { "{x=1 y=2 a={b=10 c=20}}", {{"x","1"},
                                  {"y","2"},
                                  {"z",""},
                                  {"a.b","10"},
                                  {"a.c","20"}} },
    { "{x={y={z=123}} z=345}",{{"z","345"},
                               {"x z","345"},
                               {"x.z",""},
                               {"x.z x.y.z","123"},
                               {"x.y.z z","BADSEARCH"},
                               {"x.y x z","345"}} },
    { "{x={t=f v={} r=11 f=10}}",{{"x.t","f"},
                                  {"x.f","10"},
                                  {"x.v r","11"},
                                  {"x.w r","11"},
                                  {"x.w! r",""},
                                  {"x.v.w r","11"},
                                  {"x.v.r",""}} },
  };

  bool fail=false;
  for(ArkBattery const &t: test) {
    Ark::ark a=parse(t.first);
    Ark::reader r(a);
    for(QueryResult const &qr: t.second) {
      fprintf(stderr,"testing: %s/\"%s\"\n",t.first.c_str(),qr.first.c_str());
      std::string ans="";
      try {
        r.get(qr.first).set_opt(ans);
      }
      catch (reader::badsearch &) {
        ans = "BADSEARCH";
      }
      if (ans == qr.second) continue;
      fail=true;
      fprintf(stderr,"failed: (\"%s\" != \"%s\")\n",ans.c_str(),qr.second.c_str());
    }
  }

  if (fail) exit(1);
}
