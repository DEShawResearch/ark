from __future__ import print_function

from ark import Ark
import os
import pytest
from six.moves import cPickle as pickle

@pytest.fixture
def desmond():
    path = os.path.join(os.path.dirname(__file__), 'files/desmond.ark')
    return Ark.load(path).set('boot.file', 'test.dms').set('app', 'mdsim')

def test_load(desmond):
    assert desmond['desmond_submit.locker.enable'] == False
    assert desmond['boot.file'] == 'test.dms'
    assert desmond['app'] == 'mdsim'

def test_save(desmond, tmpdir):
    tmp = tmpdir.join('out.ark').strpath
    desmond.save(tmp)
    new = Ark.load(tmp)
    assert desmond == new

def test_pickle(desmond):
    s = pickle.dumps(desmond)
    new = pickle.loads(s)
    assert new == desmond

    s = pickle.dumps(desmond, pickle.HIGHEST_PROTOCOL)
    new = pickle.loads(s)
    assert new == desmond

def test_contains(desmond):
    assert 'mdsim' in desmond
    assert 'boot.file' in desmond
    assert 'not_a_key' not in desmond
    assert not 'not_a_key' in desmond
    assert 34 not in desmond

def test_get():
    cfg = Ark({'test': 'value',
               'nest': {'test': 2},
               'nest2': {'nesta': {'nesta1': 3}}})
    assert cfg.get('test') == 'value'
    assert cfg['nest.test'] == 2
    assert cfg.get('missing_key') == None
    with pytest.raises(KeyError):
        cfg.__getitem__('missing_key')
    assert cfg.get('missing_with_def', 42) == 42
    assert cfg['nest2.nesta'] == {'nesta1' : 3}
    assert cfg.get('nest2.nesta') == {'nesta1' : 3}


def test_get_list():
    cfg = Ark({'test': 'value',
               'nest': {'test': [[{'value': 1}, {'value': 2}]]}})
    assert cfg['nest.test[0][1].value'] == 2

def test_set():
    cfg = Ark()
    cfg.set('test', 'value')
    cfg.set('nest.test', 2)
    cfg['nest.test2'] = 3
    assert cfg.get('test') == 'value'
    assert cfg.get('nest.test') == 2
    assert cfg.get('nest.test2')== 3

def test_del():
    cfg = Ark()
    cfg['a.b'] = 42
    cfg['a.c'] = 5
    cfg['a.d'] = [1,2,3]
    assert 'a.b' in cfg
    del cfg['a.b']
    assert 'a.b' not in cfg
    assert cfg.pop('a.c') == 5
    assert 'a.c' not in cfg

    assert cfg['a.d[1]'] == 2
    del cfg['a.d[1]']
    assert cfg['a.d'] == [1,3]
    assert 'a.d[2]' not in cfg
    assert cfg.pop('a.d[0]') == 1
    assert cfg['a.d'] == [3]

def test_boolean():
    assert Ark.parse('a=True b=true c=False d=false') == dict(a=True, b=True, c=False, d=False)

def test_merge():
    cfg = Ark({'test': 'value',
               'nest': {'test': 2, 'test1': 3}})
    # assign a new open-table nest, which should preserve
    # values which exist but are not updated
    new_nest = {'other_value': 4,
                'test1': 5}
    cfg.set('nest', new_nest, merge=True)
    expected = Ark({'test': 'value',
                    'nest': {'test': 2,
                             'test1': 5,
                             'other_value': 4}})
    assert cfg == expected

def test_merge2():
    d1 = dict(c=dict(a=5, b=6, x=dict(y=5)))
    d2 = dict(a=7, x=dict(z=3))
    cfg1 = Ark(d1).set('c', d2, merge=False)
    cfg2 = Ark(d1).set('c', d2, merge=True)
    assert cfg1 == dict(c=d2)
    assert cfg2 == dict(c=dict(a=7, b=6, x=dict(y=5,z=3)))

def test_merge():
    cfg = Ark(a=5, b=dict(y=3))
    cfg.merge(dict(c=42, b=dict(z=4)))
    assert cfg == dict(a=5, c=42, b=dict(y=3, z=4))

def test_numpy():
    import numpy
    z = numpy.zeros(4)
    ark = Ark().set("foo", z)
    assert isinstance(ark['foo'], type(z))
    assert Ark.parse(ark.format()) == { 'foo' : [0.0, 0.0, 0.0, 0.0] }
    ark = Ark().set("foo", numpy.zeros((3,2), dtype='i'))
    assert Ark.parse(ark.format()) == { 'foo' : [[0,0],[0,0],[0,0]] }

def test_parse():
    assert Ark.parse('foo=[1 2]') == dict(foo=[1,2])
    assert Ark.parse('[1 2 {a=" 2 3 "} ]', keyvals=False) == [1,2,dict(a=" 2 3 ")]
    assert Ark.parse("'hit me'", keyvals=False) == 'hit me'

    with pytest.raises(RuntimeError):
        Ark.parse(' no good')

def test_include(desmond):
    path = os.path.join(os.path.dirname(__file__), 'files/includeme')
    new = desmond.include(path)
    assert new is desmond
    assert desmond['gotincluded'] == [1,2,3]
    assert desmond['desmond_submit.locker.enable'] is False

def test_nested(desmond):
    assert desmond['desmond_submit']['locker.enable'] is False

    d = Ark(dict(a=[42, dict(b=dict(c='hello'))]))
    assert d['a[1].b.c'] == 'hello'
    assert d['a[1]']['b.c'] == 'hello'

    assert Ark().set('a.b.c', 1)=={'a':{'b':{'c':1}}}
    assert Ark().set('a[0][0]', 1)=={'a':[[1]]}
    assert Ark().set('a[0].b[0]', 1)=={'a':[{'b':[1]}]}

def test_nested2():
    ark=Ark()
    ark['y[0]']=5
    ark['y[1]']=6
    ark['y[2]']=7
    ark['y[3][0]']=8
    ark['z[0].x']=9
    assert ark=={'y':[5,6,7,[8]],'z':[{'x':9}]}
    ark['y.f'] = 42
    assert ark['y.f'] == 42
    ark['y[0]'] = 'foo'
    assert ark['y[0]'] == 'foo'
    ark['y[0]'] = 'bar'
    assert ark['y[0]'] == 'bar'

def test_nested_overlap():
    ark=Ark.parse('a[0].b.c=[1 2 3]')
    ark['a[0].c'] = 4
    assert ark == {'a': [{'c': 4, 'b': {'c': [1, 2, 3]}}]}

    ark=Ark.parse("a.b=hello")
    ark["a.c.interval"]=2.0
    assert ark == {'a': {'b': 'hello', 'c': {'interval': 2.0}}}


def test_file(tmpdir):
    ark = Ark().set('boot.file', Ark.File('input.dms'))
    want = 'boot = {\n    file = !file input.dms\n}\n'
    got = ark.format()
    assert want==got

    want = 'boot.file = !file input.dms\n'
    got = ark.format(flatten=True)
    assert want==got


def test_bad_keys():
    ark=Ark()
    for key in """
    [0]
    a[0].0[0]
    """.splitlines():
        with pytest.raises(ValueError):
            ark[key] = 0

