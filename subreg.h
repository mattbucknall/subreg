/**
 * SubReg - A small footprint regular expression engine written in ANSI C.
 * 
 * https://github.com/mattbucknall/subreg
 * 
 * Copyright (c) 2016 Matthew T. Bucknall
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISIN
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _SUBREG_H_
#define _SUBREG_H_

/**
 * Result code. The nesting depth of groups contained within the regular
 * expression exceeds the limit specified by max_depth.
 */
#define SUBREG_RESULT_MAX_DEPTH_EXCEEDED        -5


/**
 * Result code. The regular expression contains an invalid metacharacter
 * (typically a malformed \ escape sequence).
 */
#define SUBREG_RESULT_INVALID_METACHARACTER     -4


/**
 * Result code. A closing group bracket without a matching opening group
 * bracket has been found.
 */
#define SUBREG_RESULT_SURPLUS_BRACKET           -3


/**
 * Result code. A closing group bracket is missing from the regular expression.
 */
#define SUBREG_RESULT_MISSING_BRACKET           -2


/**
 * Result code. Syntax error found in regular expression. This is a general
 * syntax error response - If SubReg can provide a more descriptive result
 * code, then it will.
 */
#define SUBREG_RESULT_ILLEGAL_EXPRESSION        -1


/**
 * No match occurred.
 */
#define SUBREG_RESULT_NO_MATCH                  0


/**
 * Matches input string against regular expression. See README.md for
 * supported regular expression syntax.
 * 
 * \param regex         Null-terminated string containing regular expression.
 * 
 * \param input         Null-terminated string to match against regex.
 * 
 * \param max_depth     Maximum depth of nested groups to allow in regex.
 *                      This value is used to limit SubReg's system stack
 *                      usage. A value of 4 is probably enough to cover
 *                      most use cases.
 * 
 * \return              >0 if input matches regex, SUBREG_RESULT_NO_MATCH if it
 *                      does not or <0 if an error occurred.
 */
int subreg_match(const char* regex, const char* input, int max_depth);

#endif /* _SUBREG_H_ */
