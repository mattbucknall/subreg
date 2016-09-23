# SubReg
Small footprint regular expression engine written in ANSI C.

## Features
- Single source file, single header - Just statically link with your own code
- MIT license - Permits use in open and closed source projects
- Written in ANSI C - SubReg should run on more or less any platform
- No external dependencies
- No internal use of statically or dynamically allocated memory structures
- Very light use of system stack with parameterized recurrsion limit
- Regular expressions are interpreted directly

## Syntax

SubReg currently supports the following regular expression syntax:
```
.         Matches any character
\b        Matches backspace character (code 0x08)
\f        Matches form feed character (code 0x0C)
\n        Matches new line/line feed character (code 0x0A)
\r        Matches carriage return character (code 0x0D)
\t        Matches horizontal tab character (code 0x09)
\v        Matches vertical tab character (code 0x0B)
\xXX      Matches character 0xXX
\d        Matches any digit character (0-9, codes 0x30-0x39)
\s        Matches any whitespace character (\f, \n, \r, \t, \v or SPACE)
\S        Matches any non-white character

\? \*     Matches meta character
\+ etc.

?         Matches zero or one time
*         Matches zero or more times
+         Matches one or more times
a|b       Matches a or b
()        Grouping

^ $       See limitations
```
A full explaination of regular expression syntax and operation is outside the scope of this document. See https://en.wikipedia.org/wiki/Regular_expression for further info.

## Limitations

Aside from only supporting a limited subset of features typically implemented by larger regular expression engines, SubReg also
has the following additional limitations:

- Only supports ASCII character encoding
- Only supports anchored expressions (i.e. ^ and $ are implied whether specified or not).
- No support for captures (yet)
- Implementation designed to facilitate small memory footprint at the expense of execution speed

## Usage

SubReg consists of only a single source file and a single header file. To use SubReg in your own project,
just link `subreg.c` with the rest of your source code and ensure `subreg.h` is in your include path.

SubReg exposes only a single public function:
```C
int subreg_match(const char* regex, const char* input, int max_depth);
```
This function takes the following arguments:

|Argument  |Description|
|----------|-----------|
|`regex`|Null-terminated string containing regular expression.|
|`input`|Null-terminated string to match against regex.|
|`max_depth`|Maximum depth of nested groups to allow in regex. This value is used to limit SubReg's system stack usage. A value of 4 is probably enough to cover most use cases.|

The function will return a value greater than zero if a match occurred, otherwise one of the following return codes will be returned:

|#define|Value|Description|
|-------|-----|-----------|
|SUBREG_RESULT_NO_MATCH|0|No match occurred|
|SUBREG_RESULT_ILLEGAL_EXPRESSION|-1|Syntax error found in regular expression. This is a general syntax error response - If SubReg can provide a more descriptive syntax error code (as defined below), then it will.|
|SUBREG_RESULT_MISSING_BRACKET|-2|A closing group bracket is missing from the regular expression.|
|SUBREG_RESULT_SURPLUS_BRACKET|-3|A closing group bracket without a matching opening group bracket has been found.|
|SUBREG_RESULT_INVALID_METACHARACTER|-4|The regular expression contains an invalid metacharacter (typically a malformed \ escape sequence)|
|SUBREG_RESULT_MAX_DEPTH_EXCEEDED|-5|The nesting depth of groups contained within the regular expression exceeds the limit specified by `max_depth`.|

## License

```
Copyright (c) 2016 Matthew T. Bucknall

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISIN
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```
