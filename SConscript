Import('env')

env.Append(CPPPATH = ['src'])
env.Append(CXXFLAGS='-O2 -g -Wall -std=c++11')

objs = env.AddObject(Glob('src/*.cpp'))
env.AddLibrary('ark', objs)
env.AddLibrary('ark', objs, archive=True)
env.AddLibrary('ark_static', objs, archive=True)     # easier to link
env.AddHeaders(Glob('src/*.hpp'), prefix='ark', stage=True)

prgenv = env.Clone()
prgenv.Append(LIBS=['ark_static'])
prgenv.AddProgram('arkcat', 'tools/arkcat.cpp')
prgenv.AddProgram('arkget', 'tools/arkget.cpp')

for f in Split('''
example_ark
example_arkto
example_arkreader
example_merge
example_parser
example_printer
example_tokens
example_xget
'''):
    prgenv.AddExampleProgram( f, 'tests/%s.cpp' % f)

for f in Split('''
ut_arkreader
'''):
    prgenv.AddTestProgram( f, 'tests/%s.cpp' % f)

env.SConscript('python/SConscript')
env.AddDoxygen('doxygen.config')
