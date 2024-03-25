import inspect

def whoami():
    return inspect.stack()[1][3]

def whosdaddy():
    return inspect.stack()[2][3]

def foo():
    print "I'm '%s', Dad is '%s'" % (whoami(), whosdaddy())
    return "Foo"

def bar(x=foo()):
    print x

bar()
 bar()
  bar().