from __future__ import print_function

import os
import sys
import tempfile
import pytest 
import ark

testfile=os.path.join(os.path.dirname(__file__), 'files', 'testfile')
testfile=os.path.normpath(os.path.relpath(testfile))

def assertEqual(x,y): assert x==y

def testEmptyString():
    d=dict(key="")
    assert ark.fromString(ark.toString(d))==d

def testEraseListElement():
    cfg,args = ark.fromArgv(("--cfg", "a=[1 2]", "--cfg", "a[0]!erase"))
    assert cfg==dict(a=['2'])
    cfg,args = ark.fromArgv(("--cfg", "a{b=3 c=[1 2]}", "--cfg", "a.c[1]!erase"))
    assert cfg==dict(a=dict(b='3', c=['1']))
    with pytest.raises(RuntimeError):
        ark.fromArgv(("--cfg", "a=[1 2]", "--cfg", "a[3]!erase"))
    # access to one past the current end is a no-op, as this erases
    # the None ark just autovivivied.
    cfg,args = ark.fromArgv(("--cfg", "a=[1 2]", "--cfg", "a[+]!erase"))
    assert cfg==dict(a=['1','2'])
    cfg,args = ark.fromArgv(("--cfg", "a=[1 2]", "--cfg", "a[2]!erase"))
    assert cfg==dict(a=['1','2'])


def testNonWhitespace():
    d=dict(a=12345, b='xyz')
    s = ark.toString(d, whitespace=False)
    print(d, s)
    assert ark.fromString(s, convert_strings=True)==d

def testTestfile():
    testfile_as_python = {'A': {'y': '2', 'x': '1'}, 'ert': '17', 'B': {'y': '2'}, 'y': [{'myarray': ['3', '4', '5']}, 'yuiop', 'qwerty', 'added', 'added again', '\n', '\\'], 'f': ['1'], 'z': '5', 'jgak': '123', 'ggg': {'gotincluded': ['1', '2', '3'], 'testing_file_form3': 'tests/files/', 'rrr': ['5', {'jj': '183'}, 'xxx'], 'testing_file_form2': '/parser.cpp', 'testing_file_form1': 'tests/files/ark/parser.cpp'}, 't': {'multi': {'ijkl': 'abcd', 'gotincluded': ['1', '2', '3'], 'record': 'here', 'testing_file_form1': 'tests/files/ark/parser.cpp', 'testing_file_form2': '/parser.cpp', 'testing_file_form3': 'tests/files/'}, 'e': '4'}, 'file': "blah blah `file` here's \\`'s", 'nothing': ['.1', None, '0.4'], 'x': [{'tau': ['1', '2']}, {'tau': ['23', {'hey': 'there'}, '56']}], 'array': ['1', '2', '3', '0x27', '010'], 'foo': ['3'], 'True': 'true'}
    cfg,args = ark.fromArgv(('--include', testfile))
    assertEqual(cfg, testfile_as_python)
    assertEqual(args, [])

def testConvertStrings():
    testfile_as_python = {'A': {'y': 2, 'x': 1}, 'ert': 17, 'B': {'y': 2}, 'y': [{'myarray': [3, 4, 5]}, 'yuiop', 'qwerty', 'added', 'added again', '\n', '\\'], 'f': [1], 'z': 5, 'jgak': 123, 'ggg': {'gotincluded': [1, 2, 3], 'testing_file_form3': 'tests/files/', 'rrr': [5, {'jj': 183}, 'xxx'], 'testing_file_form2': '/parser.cpp', 'testing_file_form1': 'tests/files/ark/parser.cpp'}, 't': {'multi': {'ijkl': 'abcd', 'gotincluded': [1, 2, 3], 'record': 'here', 'testing_file_form1': 'tests/files/ark/parser.cpp', 'testing_file_form2': '/parser.cpp', 'testing_file_form3': 'tests/files/'}, 'e': 4}, 'file': "blah blah `file` here's \\`'s", 'nothing': [0.1, None, 0.4], 'x': [{'tau': [1, 2]}, {'tau': [23, {'hey': 'there'}, 56]}], 'array': [1, 2, 3, 39, 8], 'foo': [3], 'True': True}
    cfg,args = ark.fromArgv(('--include', testfile), convert_strings=True)
    assertEqual(cfg, testfile_as_python)
    assertEqual(args, [])

def testRoundTrip():
    old_cfg,_ = ark.fromArgv(('--include', testfile))
    new_str = ark.toString(old_cfg)
    new_cfg = ark.fromString(new_str)
    assertEqual(old_cfg, new_cfg)

def testNewline():
    old_cfg = dict(a='\n')
    new_str = ark.toString(old_cfg)
    new_cfg = ark.fromString(new_str)
    assert old_cfg == new_cfg

def testSaveLoad():
    old = ark.load(testfile)
    with tempfile.NamedTemporaryFile() as fp:
        ark.save(old, fp.name)
        new = ark.load(fp.name)
        assertEqual(old, new)

def testLoadMultiple():
    cfg1 = ark.load(testfile, testfile)
    cfg2,_ = ark.fromArgv(('--include', testfile, '--include', testfile))
    assertEqual(cfg1, cfg2)

def testRoundTripConvertStrings():
    old_cfg,_ = ark.fromArgv(('--include', testfile), convert_strings=True)
    new_str = ark.toString(old_cfg)
    new_cfg = ark.fromString(new_str, convert_strings=True)
    assertEqual(old_cfg, new_cfg)

def testBadArgv():
    badfile = os.path.join(os.path.dirname(__file__), 'bad.cfg')
    with pytest.raises(RuntimeError):
        ark.fromArgv(('prog', '--include', badfile))
    with pytest.raises(RuntimeError):
        ark.fromArgv(('prog', '--cfg', 'foobar'))

def testCircular():
    d = dict()
    d['foo'] = d
    with pytest.raises(RuntimeError):
        ark.toString(d)

