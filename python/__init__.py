'''
Functions for converting ark text to and from Python data structures.

following correspondence is made between Python types and ark types:

    =======    ======    ===========================
    Python     Ark       Example                    
    =======    ======    ===========================
    bool       atom      True    <---> "true"        
    bool       atom      False   <---> "false"       
    string     atom      "abc"   <---> "abc"         
    integer    atom      32      <---> "32"          
    float      atom      0.1     <---> "0.1"         
    list       vector    [1,2,3] <---> ["1" "2" "3"] 
    dict       table     {'a':5} <---> {a="5"}       
    =======    ======    ===========================

Setting the convert_strings option in fromString() and fromArgv() to
True enables conversion from ark atoms into int, float,or bool Python
types when possible.  In this mode, conversions from ark to Python
will first attempt to convert to integer, failing that to float, and
only then fall back to string.  By default, convert_strings is False,
though it may prove convenient.

When converting from Python to ark, Python floats are converted to
strings using Python's repr() function, which preserves information
while constructing a concise representation of the bits.

'''
from . import _ark
import argparse
import collections
import os

from collections.abc import Mapping

def toString(obj, keyvals=True, whitespace=True,
                  open_tables=False, flatten=False):
    ''' 
    Serialize obj to an Ark formatted ``str``.

    If keyvals is False, the top level object need not be a table,
    and if it is, the output will be bracketed by braces.  By
    default, keyvals is True, so obj is expected to be a table, and
    no outer braces are included, making it suitable for writing
    to a file.

    If whitespace is False, a compact but less human-readable representation 
    is returned.  By default, whitespace is included.

    If open_tables is True, all tables will be written in open form instead
    of closed form.

    If flatten is True, ark text is produced in flattened form.
    '''
    p = _ark.printer()

    p.whitespace(whitespace)
    p.no_delim(keyvals)
    p.open_tables(open_tables)
    p.flatten(flatten)

    a = _ark.from_object(obj)
    return p(a, True).output()

def fromString(s, keyvals=True, convert_strings=False):
    '''
    Deserialize ``s`` (a ``str`` instance containing Ark syntax) to a 
    Python object.  

    By default, the ark text is expected to be in keyvals form, i.e. a
    table with no outer braces.  To parse in 'default' mode, set the
    keyword argument keyvals to False.

    If convert_strings is True, try to convert atoms to int, float,
    or bool before giving up and settling for string.
    '''
    a = _ark.ark()
    p = _ark.parser()
    if keyvals:
        p.parse_keyvals(a, s)
    else:
        p.parse_string(a, s)
    return _ark.to_object(a, convert_strings)

def fromArgv(argv, convert_strings=False):
    '''
    Handle --include xxx and --cfg options from a list of strings.
    
    This function recognizes two options:
        --include <file>        -- read keyvals from file
        --cfg <text>            -- read literal keyvals from text

    It returns the ark produced by parsing the provided ark text,
    and the elements of argv that were ignored.

    If convert_strings is True, try to convert atoms to int, float,
    or bool before giving up and settling for string.
    '''
    a = _ark.ark()
    i,n = 0, len(argv)
    args = []
    while i<n:
        if argv[i] == '--include':
            i += 1
            if i==n:
                raise ValueError('Missing argument to --include')
            _ark.parser().parse_file(a, argv[i])
        elif argv[i] == '--cfg':
            i += 1
            if i==n:
                raise ValueError('Missing argument to --cfg')
            _ark.parser().parse_keyvals(a, argv[i])
        else:
            args.append(argv[i])
        i += 1
    return _ark.to_object(a, convert_strings), args

def load(*paths, **kwds):
    ''' Read ark(s) from file(s).

    Keyword arguments as in fromArgv().
    '''
    argv = list()
    for path in paths:
        argv.extend(('--include', path))
    cfg, _  = fromArgv(argv, **kwds)
    return cfg

def save(obj, path, **kwds):
    ''' Write ark to file.

    Args:
        obj (dict): ark object as nested dict, list, string.
        path (str): file path

    Keyword arguments as in toString().
    '''
    with open(path, 'w') as fp:
        fp.write(toString(obj, **kwds))

class ArkAction(argparse.Action):
    @staticmethod
    def configure(parser, prefix=None, defval=None, abspath=False):
        ''' add --$prefix-include and --$prefix-cfg arguments to parser,
        with dest=$prefix_arkarg, optionally resolve abs paths
        '''
        inc = '--%s-include' % prefix if prefix else '--include'
        cfg = '--%s-cfg' % prefix if prefix else '--cfg'
        dst = '%s_arkarg'% prefix if prefix else 'arkarg'
        if defval is None: defval=[]
        inctype = os.path.abspath if abspath else None
        parser.add_argument(inc, action=ArkAction, dest=dst, default=defval, type=inctype)
        parser.add_argument(cfg, action=ArkAction, dest=dst)

    def __init__(self, option_strings, dest, nargs=None, **kwds):
        argparse.Action.__init__(self, option_strings, dest, **kwds)

    def __call__(self, parser, namespace, value, option_string):
        prev = getattr(namespace, self.dest)
        if option_string.endswith('-include'):
            prev.append('--include')
        elif option_string.endswith('-cfg'):
            prev.append('--cfg')
        else:
            raise argparse.ArgumentError(option_string,
                    'Unrecognized ark action')
        prev.append(value)


class File(object):
    ''' object which prepends '!file' to its argument on serialization
    '''
    def __init__(self, text):
        self._text = text

    def __str__(self):
        return '!file "%s"' % self._text

    def __repr__(self):
        return 'File("%s")' % self._text

    @property
    def text(self):
        ''' wrapped text '''
        return self._text


class Ark(dict):
    ''' dict-like class implementing 'xget' style accessors

    Arks are a subclass of dict, and can be used like dicts.
    The primary difference is that values might be copied when they are assigned.
    
    At its most basic, this class allows you to index into dicts using
    a shorthand '.' syntax. For an Ark a::
    
            a["foo.bar[0].baz"]
    
        is the same as::
    
            a["foo"]["bar"][0]["baz"]
    
    Arks can be loaded and saved with Ark.load() and save().

    Based on the original bark.BArk.
    '''

    def __init__(self, *args, **kwds):
        super(Ark, self).__init__()
        d = dict(*args, **kwds)
        for k,v in d.items():
            self[k] = v

    @classmethod
    def load(cls, path, convert_strings=True):
        ''' Load an ark from path or file

        Args:
            path (str): file path
            convert_strings (bool): convert strings to their apparent types

        Returns:
            Ark
        '''
        args = ['--include', path]
        cfg, _ = fromArgv(args, convert_strings=convert_strings)
        return cls(cfg)

    def include(self, path, convert_strings=True):
        ''' Read an ark file into this ark

        Args:
            path (str): file path
            convert_strings (bool): convert strings to their apparent types

        Returns:
            Ark
        '''
        s = self.format()
        args = ['--cfg', s, '--include', path]
        cfg, _ = fromArgv(args, convert_strings=convert_strings)
        self.clear()
        self.update(cfg)
        return self

    @classmethod
    def parse(cls, text, convert_strings=True, keyvals=True):
        ''' Create an ark from its textual represntation

        Args:
            text (str): ark text
            convert_strings (bool): convert strings to their apparent types
            keyvals (bool): assume keys at top level

        Returns:
            Ark
        '''
        obj = fromString(text, convert_strings=convert_strings, keyvals=keyvals)
        return _cast_to_ark(obj)


    def format(self, **kwds):
        ''' string representation of self from toString '''
        return toString(self, **kwds)


    
    def save(self, path, **kwds):
        ''' Save ark to file

        Args:
            path (str): file path

        Additional keyword arguments passed to toString.
        '''
        cfg = toString(self, **kwds)
        with open(path, 'w') as fp:
            fp.write(cfg)

    @staticmethod
    def _split_key(key):
        return str(key).split('.')

    @staticmethod
    def _parse_list_indices(k):
        idx = []
        if k.endswith(']'):
            k, idx = k.split('[', 1)
            idx = [int(i) for i in idx[:-1].split('][')]
        return k, idx

    @staticmethod
    def _get_from_list(ark, idx):
        for j in idx:
            if not isinstance(ark, list):
                raise KeyError('[%d]' % j)
            ark = ark[j]
        return ark

    def __getitem__(self, key):
        ark = self
        try:
            for k in self._split_key(key):
                k, idx = self._parse_list_indices(k)
                ark = dict.__getitem__(ark, k)
                ark = self._get_from_list(ark, idx)
        except KeyError as e:
            raise KeyError("No key %s; failed lookup on %s" % (key, e))
        except IndexError as e:
            raise KeyError("No key %s; failed lookup on %s" % (key, e))
        except TypeError:
            raise KeyError("No key %s; expected dict but got value for key %s" % (key, k))
        return ark

    def __delitem__(self, key):
        ark = self
        keys = self._split_key(key)
        try:
            for k in keys[:-1]:
                k, idx = self._parse_list_indices(k)
                ark = dict.__getitem__(ark, k)
                ark = self._get_from_list(ark, idx)
        except KeyError as e:
            raise KeyError("No key %s; failed lookup on %s" % (key, e))
        except TypeError:
            raise KeyError("No key %s; expected dict but got value for key %s" % (key, k))
        last_key = keys[-1]

        # at this point we may have a key like 'd[1]', in which case we
        # have to drill down one more time.
        k, idx = self._parse_list_indices(last_key)
        if idx:
            ark = ark[k]
            for id in idx[:-1]:
                ark = ark[id]
            del ark[idx[-1]]
        else:
            dict.__delitem__(ark, last_key)

    def pop(self, key, *args):
        ''' D.pop(k,[d])-> v
        Remove specified key and return the corresponding value.
        If key is not found, d is returned if given, otherwise KeyError
        is raised.

        Args:
            key (str): ark key

        '''
        if len(args) > 1:
            raise TypeError("pop expected at most 2 arguments; got %d" % (
                len(args)+1))
        try:
            value = self.__getitem__(key)
            self.__delitem__(key)
            return value
        except KeyError:
            if args:
                return args[0]
            raise

    def __contains__(self, key):
        try:
            self.__getitem__(key)
        except KeyError:
            return False
        return True

    def __setitem__(self, key, value):
        self.set(key, value, merge=False)

    def get(self, key, d=None):
        try:
            return self.__getitem__(key)
        except KeyError:
            return d

    def merge(self, keyvals):
        ''' merge keyvals into self

        Args:
            keyvals (dict): keyval source

        Returns:
            Ark
        '''
        return _merge_ark(self, keyvals)

    def set(self, key, value, merge=True):
        '''
        Assign @value to @key.
        
        If merge=True, then assigned Table will be meshed with existing
        table. Atoms in @value will overwrite existing atoms in @self,
        but atoms in @self that do not exist in @value will remain.
        '''
        if merge and isinstance(value, dict):
            oldval = self.get(key, dict())
            _merge_ark(oldval, value)
            value = oldval

        # split key into tokens: strings for dict keys, integers for list indices
        tokens = []
        for elem in key.strip().split('.'):
            if elem.endswith(']'):
                k, rest = elem.split('[', 1)
                tokens.append(k)
                tokens += [int(x) for x in rest[:-1].split('][')]
            else:
                k = elem
                tokens.append(elem)
            if not _ark.valid_key(k):
                raise ValueError("invalid key '%s'" % key)
        tokens.append(_cast_to_ark(value))

        # drill down into the ark, using the current token as either the
        # dict key or list index, and the token after that to decide what
        # sort of value to autovivify.
        ark = self
        token1 = tokens[0]

        for i in range(1, len(tokens)):
            token2 = tokens[i]
            final = False
            if i==len(tokens)-1:
                val = token2
                final = True
            elif isinstance(token2, str):
                val = dict()
            elif isinstance(token2, int):
                val = list()
            if isinstance(ark, dict):
                if final:
                    dict.__setitem__(ark, token1, val)
                elif dict.__contains__(ark, token1):
                    oldval = dict.__getitem__(ark, token1)
                    if not isinstance(oldval, type(val)):
                        dict.__setitem__(ark, token1, val)
                else:
                    dict.__setitem__(ark, token1, val)
                ark = dict.__getitem__(ark, token1)
            else:
                if len(ark)==token1:
                    ark.append(val)
                elif final:
                    ark[token1] = val
                ark = list.__getitem__(ark, token1)
            token1 = token2

        return self


def _cast_to_ark(obj):
    if isinstance(obj, (Ark, str)):
        return obj
    if isinstance(obj, Mapping):
        return Ark((k, _cast_to_ark(v)) for k,v in obj.items())
    if isinstance(obj, list):
        return [_cast_to_ark(v) for v in obj]
    return obj
    
def _merge_ark(ark, values):
    for k,v in values.items():
        if isinstance(v, dict) and k in ark:
            _merge_ark(ark[k], v)
        else:
            ark[k] = v


# convenenient to have File live in the Ark namespace so that
# you can just do `from ark import Ark`
Ark.File = File

