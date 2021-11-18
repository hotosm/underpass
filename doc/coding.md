# Underpass Coding Style

This documents the prefered coding style for Underpass. The goal
of this coding style is to make code consistent amongst developers,
and to write code that is focused on being readable and
compact. Because the existing code is written with this goal, using
reformatting tools like *indent* or *clang-format* is discouraged.

The easy way to follow this style is to use
[cc-mode](https://www.gnu.org/software/emacs/manual/html_mono/ccmode.html)
in Emacs. Spaces will be used instead of TABS, since different systems
or users may have TAB set differently. Indents are 4 spaces, instead
of 8. 

## Line Length

For decades lines were limited to 80 charcters or less, which was
based on what a CRT could display. With modern monitors this
limitation doesn't exist. The Linux kernel has been using 132 character
line length, so this project does too.

## Line breaks

Line breaks are used to break up long lines, with the remainder
indented to be clear it's a continuation of the line. Some times a
long line is prefered if adding a line break makes the code less
readable.

## Brace Location

The opening brace for most code is on the same line as the line of
code. Putting braces on a line by itself adds to much whitespace.

Some example:

	namespace replication {
	class StateFile;
	};

	class RawChangeset {
      public:
		  void foo(void);
	};

## Naming Convention

Underpass uses [Camel Case](https://en.wikipedia.org/wiki/Camel_case),
which uses capatialization instead of underbars for class definitions,
function or C++ methods, and variable. For example *RawChange* instead
of *raw_change*. Class names capitalize the first letter as well,
where the methods in the class start with a lower case letter.

Names should be descriptive, and when possible, avoid abbrevations
unless they are commonly used.


