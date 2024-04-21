#!/usr/bin/env python3

# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
from typing import Dict

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

import ninja_syntax

LONGWORD = 'a' * 10
LONGWORDWITHSPACES = 'a'*5 + '$ ' + 'a'*5
INDENT = '    '

class TestLineWordWrap(unittest.TestCase):
    def setUp(self) -> None:
        self.out = StringIO()
        self.n = ninja_syntax.Writer(self.out, width=8)

    def test_single_long_word(self) -> None:
        # We shouldn't wrap a single long word.
        self.n._line(LONGWORD)
        self.assertEqual(LONGWORD + '\n', self.out.getvalue())

    def test_few_long_words(self) -> None:
        # We should wrap a line where the second word is overlong.
        self.n._line(' '.join(['x', LONGWORD, 'y']))
        self.assertEqual(' $\n'.join(['x',
                                      INDENT + LONGWORD,
                                      INDENT + 'y']) + '\n',
                         self.out.getvalue())

    def test_comment_wrap(self) -> None:
        # Filenames should not be wrapped
        self.n.comment('Hello /usr/local/build-tools/bin')
        self.assertEqual('# Hello\n# /usr/local/build-tools/bin\n',
                         self.out.getvalue())

    def test_short_words_indented(self) -> None:
        # Test that indent is taking into account when breaking subsequent lines.
        # The second line should not be '    to tree', as that's longer than the
        # test layout width of 8.
        self.n._line('line_one to tree')
        self.assertEqual('''\
line_one $
    to $
    tree
''',
                         self.out.getvalue())

    def test_few_long_words_indented(self) -> None:
        # Check wrapping in the presence of indenting.
        self.n._line(' '.join(['x', LONGWORD, 'y']), indent=1)
        self.assertEqual(' $\n'.join(['  ' + 'x',
                                      '  ' + INDENT + LONGWORD,
                                      '  ' + INDENT + 'y']) + '\n',
                         self.out.getvalue())

    def test_escaped_spaces(self) -> None:
        self.n._line(' '.join(['x', LONGWORDWITHSPACES, 'y']))
        self.assertEqual(' $\n'.join(['x',
                                      INDENT + LONGWORDWITHSPACES,
                                      INDENT + 'y']) + '\n',
                         self.out.getvalue())

    def test_fit_many_words(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=78)
        self.n._line('command = cd ../../chrome; python ../tools/grit/grit/format/repack.py ../out/Debug/obj/chrome/chrome_dll.gen/repack/theme_resources_large.pak ../out/Debug/gen/chrome/theme_resources_large.pak', 1)
        self.assertEqual('''\
  command = cd ../../chrome; python ../tools/grit/grit/format/repack.py $
      ../out/Debug/obj/chrome/chrome_dll.gen/repack/theme_resources_large.pak $
      ../out/Debug/gen/chrome/theme_resources_large.pak
''',
                         self.out.getvalue())

    def test_leading_space(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=14)  # force wrapping
        self.n.variable('foo', ['', '-bar', '-somethinglong'], 0)
        self.assertEqual('''\
foo = -bar $
    -somethinglong
''',
                         self.out.getvalue())

    def test_embedded_dollar_dollar(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=15)  # force wrapping
        self.n.variable('foo', ['a$$b', '-somethinglong'], 0)
        self.assertEqual('''\
foo = a$$b $
    -somethinglong
''',
                         self.out.getvalue())

    def test_two_embedded_dollar_dollars(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=17)  # force wrapping
        self.n.variable('foo', ['a$$b', '-somethinglong'], 0)
        self.assertEqual('''\
foo = a$$b $
    -somethinglong
''',
                         self.out.getvalue())

    def test_leading_dollar_dollar(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=14)  # force wrapping
        self.n.variable('foo', ['$$b', '-somethinglong'], 0)
        self.assertEqual('''\
foo = $$b $
    -somethinglong
''',
                         self.out.getvalue())

    def test_trailing_dollar_dollar(self) -> None:
        self.n = ninja_syntax.Writer(self.out, width=14)  # force wrapping
        self.n.variable('foo', ['a$$', '-somethinglong'], 0)
        self.assertEqual('''\
foo = a$$ $
    -somethinglong
''',
                         self.out.getvalue())

class TestBuild(unittest.TestCase):
    def setUp(self) -> None:
        self.out = StringIO()
        self.n = ninja_syntax.Writer(self.out)

    def test_variables_dict(self) -> None:
        self.n.build('out', 'cc', 'in', variables={'name': 'value'})
        self.assertEqual('''\
build out: cc in
  name = value
''',
                         self.out.getvalue())

    def test_variables_list(self) -> None:
        self.n.build('out', 'cc', 'in', variables=[('name', 'value')])
        self.assertEqual('''\
build out: cc in
  name = value
''',
                         self.out.getvalue())

    def test_implicit_outputs(self) -> None:
        self.n.build('o', 'cc', 'i', implicit_outputs='io')
        self.assertEqual('''\
build o | io: cc i
''',
                         self.out.getvalue())

class TestExpand(unittest.TestCase):
    def test_basic(self) -> None:
        vars = {'x': 'X'}
        self.assertEqual('foo', ninja_syntax.expand('foo', vars))

    def test_var(self) -> None:
        vars = {'xyz': 'XYZ'}
        self.assertEqual('fooXYZ', ninja_syntax.expand('foo$xyz', vars))

    def test_vars(self) -> None:
        vars = {'x': 'X', 'y': 'YYY'}
        self.assertEqual('XYYY', ninja_syntax.expand('$x$y', vars))

    def test_space(self) -> None:
        vars: Dict[str, str] = {}
        self.assertEqual('x y z', ninja_syntax.expand('x$ y$ z', vars))

    def test_locals(self) -> None:
        vars = {'x': 'a'}
        local_vars = {'x': 'b'}
        self.assertEqual('a', ninja_syntax.expand('$x', vars))
        self.assertEqual('b', ninja_syntax.expand('$x', vars, local_vars))

    def test_double(self) -> None:
        self.assertEqual('a b$c', ninja_syntax.expand('a$ b$$c', {}))

if __name__ == '__main__':
    unittest.main()
