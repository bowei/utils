#!env python
#
# Generates an efficient string matching table. Given a file of
# symbol,token pairs, generates a state machine that matches the string and
# returns the corresponding symbol. Like really dumb regex/lexer.
import sys
from optparse import OptionParser

def indent(amount): return '  ' * amount

matches = []
for l in sys.stdin:
  sym, token = l.strip().split(',')
  matches.append((sym, token))

maxlen = max([len(x) for _,x in matches])
print '''int
match(char* buf, int buf_size) {{
{}int i=0;'''.format(indent(1))
for i in xrange(maxlen):
  print '{}if (i == buf_size) {{ return 0; }}'.format(indent(1))
  print '{}switch (buf[i]) {{'.format(indent(1))
  transition = {}
  for sym, token in matches:
    if i+1 < len(token):
      transition[token[i]] = ('c', None)
    elif i+1 == len(token):
      transition[token[i]] = ('m', sym)
  for k in sorted(transition.keys()):
    action, sym = transition[k]
    if action == 'c':
      print "{}case '{}': break;".format(indent(2), k)
    elif action == 'm':
      print "{}case '{}': return {};".format(indent(2), k, sym)
  print '{}default: return 0;'.format(indent(2))
  print '{}}}'.format(indent(1))
  print '{}++i;'.format(indent(1))
print '{}return 0;'.format(indent(1))
print '}'
