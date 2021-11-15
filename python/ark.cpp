#include <pybind11/pybind11.h>
#include <sstream>
#include <set>
#include <ark/ark.hpp>

using namespace pybind11;
using namespace Ark;

namespace {
    struct _ {
        _() {
            module mod;
            // collections.Iterable is deprecated in 3.7 in favor of
            // collections.abc, but the latter is unavailable in 2.7.
            try {
                mod = module::import("collections.abc");
            } catch (error_already_set&) {
                mod = module::import("collections");
            }
            iterable = mod.attr("Iterable");
        };
        handle iterable;
    } _init;
    typedef std::set<PyObject*> ObjSet;
    void make_object(ark& a, handle obj, ObjSet circular=ObjSet()) {
        if (circular.count(obj.ptr())) {
            throw std::runtime_error("Circular reference detected");
        }
        circular.insert(obj.ptr());
        handle File = module::import("ark").attr("File");
        if (isinstance<str>(obj)) {
            a.be(Atom).atom() = std::string(str(obj));
        } else if (isinstance<float_>(obj)) {
            /* I'm using repr(obj) here because Python's repr generates
             * a compact, but non-lossy, string representation of floats.
             * printf("%.17g"), would also be non-lossy but generates 
             * something like "0.10000000000000001" for 0.1, while Python's
             * repr returns "0.1". */
            a.be(Atom).atom() = std::string(repr(obj));
        } else if (isinstance<dict>(obj)) {
            table_t& t = a.be(Table).table();
            for (auto pair : reinterpret_borrow<dict>(obj)) {
                ark& v = t[std::string(str(pair.first))];
                make_object(v, pair.second, circular);
            }
        } else if (isinstance(obj, _init.iterable)) {
            vector_t& v = a.be(Vector).vector();
            for (auto elem : obj) {
                v.resize(v.size()+1);
                make_object(v.back(), elem, circular);
            }
        } else if (isinstance(obj, File)) {
            // stick a funny character at the front which will be
            // handled by the ark printer
            std::string s;
            s.push_back(0xff);
            s.append(std::string(str(obj.attr("text"))));
            a.be(Atom).atom() = s;
        } else if (obj.ptr()==Py_None) {
            a.be(None);
        } else if (obj.ptr()==Py_True) {
            a.be(Atom).atom() = "true";
        } else if (obj.ptr()==Py_False) {
            a.be(Atom).atom() = "false";
        } else {
            a.be(Atom).atom() = std::string(str(obj));
        }
    }

    object to_object(ark const& a, bool convert_strings) {
        switch (a.kind()) {
            case Atom:
                if (convert_strings) {
                    const char* s = a.atom().c_str();
                    /* empty string? */
                    if (*s=='\0') return str(s);
                    /* true/false? */
                    if (!strcasecmp(s, "true")) return bool_(true);
                    if (!strcasecmp(s, "false")) return bool_(false);
                    /* integer? */
                    char* end = NULL;
                    long i = strtol(s, &end, 0);
                    if (*end=='\0') return int_(i);
                    /* float? */
                    end = NULL;
                    double d = strtod(s, &end);
                    if (*end=='\0') return float_(d);
                    /* plain old string */
                    return str(s);
                }
                return str(a.atom().c_str());
            case Vector:
                {
                    list L;
                    for (ark const& e : a.vector()) {
                        L.append(to_object(e,convert_strings));
                    }
                    return L;
                }
                break;
            case Table:
                {
                    dict D;
                    for (table_t::value_type const& t : a.table()) {
                        D[str(t.first.c_str())] = to_object(t.second,convert_strings);
                    }
                    return D;
                }
                break;
            case None: ;
        };
        return none();
    }
}

PYBIND11_MODULE(_ark, m) {

    enum_<kind_t>(m, "kind")
        .value("None", None) 
        .value("Atom", Atom)
        .value("Vector", Vector)
        .value("Table", Table)
        .export_values();

    class_<ark>(m, "ark")
        .def(init<>())
        ;

    class_<parser>(m, "parser")
        .def(init<>())
        .def("parse_string",
             (ark&(parser::*)(ark&, std::string const&) const) &parser::parse)
        .def("parse_keyvals",
             (ark&(parser::*)(ark&, std::string const&) const) &parser::parse_keyvals)
        .def("parse_file", &parser::parse_file)
        ;

    class_<printer::printer_ref>(m, "printer_ref")
        .def("output", [](const printer::printer_ref& p) {
            std::stringstream ss;
            p.output(ss);
            return ss.str();
            })
        ;

    class_<printer>(m, "printer")
        .def(init<>())
        .def("width",   &printer::width)
        .def("indent",  &printer::indent)
        .def("whitespace",&printer::whitespace)
        .def("no_delim",&printer::no_delim)
        .def("open_tables",&printer::open_tables)
        .def("flatten", &printer::flatten)
        .def("__call__", &printer::operator())
        ;

    m.def("from_object", [](object obj) {
            ark a; make_object(a, obj); return a;
            });
    m.def("to_object", to_object);

    m.def("valid_key", Ark::key_t::valid_key);
}

