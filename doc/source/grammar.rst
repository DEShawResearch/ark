
Ark File Grammar
================

An ark file represents a nested collection of strings, lists, and tables (i.e. maps).

Strings are delimited by single or double quotes; or can appear without quotes if they
contain no spaces or grammar characters. 

Lists are delimited by square brackets, and their elements are separated by whitespace.

Tables are delimited by curly braces, except at the top level of a file, where the grammar
implicitly begins in table mode and the curly braces are omitted.  Table keys are not quoted,
and may not contain spaces or grammar symbols.

The special token `?` represents a `None`, i.e. a empty value. 

Here is an example of an ark file::

    foo {
        a = 42
        b = "a string"
        c = [elements of a ? list]
        d = [q { list: [x y z] } [sublist elements]]
        e = {
            y = 7
        }
    }
    bar = 3
    baz = [1 2 3]


At the top level, `foo`, `bar`, and `baz` are table keys, and we see
examples of strings, lists, nones, and nested strings and lists.  

Open vs. closed tables
----------------------

In the above example, the `foo` table is written in "open" form, with no equal sign following the key, 
while the `e` subtable does have an equals sign, and is therefore closed.  It makes no difference whether
a table is open or closed once it is parsed, but if an ark is merged with another ark, the difference can
be important.

There are several different mechanisms by which arks can be merged.

* The `arkcat` command treats its `--include` arguments as arks which are to be merged with whatever arks came before.

* The `Ark` class has a merge method which merges a parsed ark into another.

* The special `!include` directive described below merges its argument into the table context in which it was invoked.


Suppose we have the following three arks; 1.ark::

    a {
        b=1
    }
    c {
        d=1
    }

2.ark::

    a {
        b=2
    }
    c {
        e=2
    }

3.ark::

    a {
        b=2
    }
    c = {
        e=2
    }

The only difference between `2.ark` and `3.ark` is the presence of the `=` in the `c` table.  Merging `2.ark` into `1.ark` gives::

    a = {
        b = 2
    }
    c = {
        d = 1
        e = 2
    }

because `2.ark`'s `c` is open.  But merging `3.ark` into `1.ark` gives::

    a = {
        b = 2
    }
    c = {
        e = 2
    }

because `3.ark`'s `c` is closed.


Config overrides
----------------

The grammar described thus far is sufficient to represnt any ark, and
programs that generate arks will usually produce text in the above form.
For convenience, ark syntax recognizes some additional constructs with
an imperative flavor that make it easier to modify existing arks or to
refer to external files.

You can override a specific, deeply nested part of an ark using open
tables as shown above, but writing out all those curly braces is tedious
and error prone, and doesn't help you when the path to the part of the
ark you want to modify goes through a list.  An more powerful method is
to use dots to drill down into tables, and square brackets with indices
to drill down into lists.  For example an ark file containing::

    c.e=2

would set or replace the value for key `e` in table `c`.  Parent tables
are created where necessary.  Similarly::

    d[0]=1

would create the list `d` if necessary, and set the first element to
`1`.  If the list already exists, it's legal to index up to one past the
last existing element.  If you don't know how many elements are present,
the special index `+` appends to a list.  For example::

    a.d[+] = 1
    a.d[+] = 2
    a.d[+] = 3

is equivalent to::

    a { 
        d = [1 2 3]
    }

You can also delete a key from a table using the special `!erase`
directive.  The ark text::

    a {
        b = 2
        c = 3
    }

    a.b !erase

would leave you with::

    a {
        c = 3
    }

The `!erase` directive works on list elements as well::

    a = [ 1 2 ]
    a[0] !erase

parses to::

    a = [ 2 ]

    

Referencing files
-----------------

In the context of a table (or at top level), you can insert the contents of another ark using the `!include`
directive::

    a = {
        b { !include something.ark
    }

Here, the text of `something.ark` would be parsed as though it were literally substituted into the `b` table.

Another use case is wanting to refer to a file in the same directory as the ark, which will remain valid if
both files are moved to another directory.  The `!file` directive can help in this situation.  Suppose we have
the arks `foo.ark`::

    !include x/bar.ark

and `x/bar.ark`::

    a=!file bar.dms

The result of parsing `foo.ark` is::

    a = x/bar.dms

The `!file` directive is correctly processed in the context of the current directory of its host file, `x/bar.ark`.


Formal grammar
--------------

* ARK  -> NONE | STRING | [ARK*] | { KEYVAL* }

* NONE -> ?

* STRING is a ", ', or ` delimited string with simple '\' meta-charactering
       or a bare string (no intervening white-space or grammar symbols).

* KEYVAL  -> INCLUDE | SKEY=ARK | SKEY ARK
  the first form does assignment, the second form does an "enclosure",
  expecting ARK to be a table and then making the SKEYed element a
  table with additional keys taken from ARK.

* INCLUDE -> ! include STRING
  expands the file named by STRING, parsing the contents as KEYVAL*

* SKEY    -> IKEY | SKEY . IKEY
  keys which name tables can be subindexed with . in a familiar way

* IKEY    -> KEY | IKEY [ INDEX ]
  keys which name vectors are indexed in similarly to arrays.

* KEY is a non-delimited string in the regexp [_a-xA-Z][_a-zA-Z0-9]*

* INDEX   -> + | INT
  if + the index is taken to be the end+1 element of the array, i.e.
  the assignment is taken to be a 'push_back' operation.

* INT is a base 10 non-negative integer.

