# https://en.wikipedia.org/wiki/Python_syntax_and_semantics

# foo function in Python:
def foo(x):
    if x == 0:
        bar()
        baz()
    else:
        qux(x)
        foo(x - 1)

#
# An example of a class defining some properties is:
class MyClass(object):
   def get_a(self):
      return self._a
   def set_a(self, value):
      self._a = value - 1
   a = property(get_a, set_a, doc="Off by one a")

# Python 2.6 style
class My26Class(object):
   @property
   def a(self):
      return self._a
   @a.setter # makes the property writable
   def a(self, value):
      self._a = value - 1

#
# Comments and docstrings
def getline1():
    return sys.stdin.readline()       # Get one line and return it

def getline2():
    return sys.stdin.readline()    """this function
                                      gets one line
                                      and returns it"""

def getline3():
    """Get one line from stdin and return it."""
    return sys.stdin.readline()

def getline4():
    """Get one line
       from stdin
       and return it."""
    return sys.stdin.readline()

#
# Concatenation of adjacent string literals
# NOTE Single quotes are possible too
title = "One Good Turn: " \
        'A Natural History of the Screwdriver and the Screw'


#
# Multi-line string literals
print("""Dear %(recipient)s,

I wish you to leave Sunnydale and never return.

Not Quite Love,
%(sender)s
""" % {'sender': 'Buffy the Vampire Slayer', 'recipient': 'Spike'})

#
# https://en.wikipedia.org/wiki/Here_document
# Python supports multi-line strings as a "verbatim" string.
print("""
Customer: Not much of a cheese shop is it?
Shopkeeper: Finest in the district , sir.
""")

# From Python 3.6 onwards, verbatim f-strings support variable and expression interpolation.
shop_type = "CHEESE"
accolade = "finest"
print(f"""
Customer: Not much of a {shop_type.lower()} shop is it?
Shopkeeper: {accolade.capitalize()} in the district , sir.
""")

# Therefore...
print('''
class DontShowThisAsClass(object):
   def get_a(self):
      return self._a
   def set_a(self, value):
      self._a = value - 1
   a = property(get_a, set_a, doc="Off by one a")

   FIXME Don't show this note
   Currently I have no tidy idea how to distinguish between triple quoted strings
   and triple quoted comments. Guess strings must inside parentheses.
   However, I think this issue should be less annoying
''')


""" Remove this line """
''' Remove this line '''

"""
class DontShowThisAsClass2(object):
   def get_a(self):
      return self._a
   def set_a(self, value):
      self._a = value - 1
   a = property(get_a, set_a, doc="Off by one a")
"""

def bar(x):
    if x == 0:
        bar()
        baz()
    else:
        qux(x)
        bar(x - 1)


# kate: space-indent on; indent-width 4; replace-tabs on;
