/**
 * SubReg - A small footprint regular expression engine written in ANSI C.
 * 
 * https://github.com/mattbucknall/subreg
 * 
 * Copyright (c) 2016-2020 Matthew T. Bucknall
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

#include <stdio.h>
#include <string.h>
#include <cutest.h>
#include <subreg.h>


static char ANY_SET[] =
{
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23,       0x25, 0x26, 0x27,
                            0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,      
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B,       0x5D,       0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B,       0x7D, 0x7E,
    
    0x00
};


static char DIGIT_SET[] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39,
    
    0x00
};


static char NON_DIGIT_SET[] =
{
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    
                0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
    
    0x00
};


static char HEX_DIGIT_SET[] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39,
          
          0x41, 0x42, 0x43, 0x44, 0x45, 0x46,      
    
          0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    
    0x00
};


static char NON_HEX_DIGIT_SET[] =
{
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    
                0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40,                                     0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60,                                     0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
    
    0x00
};


static char WHITESPACE_SET[] =
{
          0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20,
    
    0x00
};


static char NON_WHITESPACE_SET[] =
{
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08,                               0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
          0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
    
    0x00
};


static char WORD_SET[] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39,
          0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A,                         0x5F,
          0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A,
    
    0x00
};


static char NON_WORD_SET[] =
{
          0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    
                0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 
    
    
                      0x5B, 0x5C, 0x5D, 0x5E,      
    0x60,                                          
    
                      0x7B, 0x7C, 0x7D, 0x7E,
                      
    0x00
};


static void test_empty_pass(void)
{
    TEST_CHECK( subreg_match("\x09", "\x09", 0, 0, 4) == 1 );
}


static void test_empty_fail(void)
{
    TEST_CHECK( subreg_match("x", "", 0, 0, 4) == 0 );
}


static void test_literal(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = ANY_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match(buffer, buffer, 0, 0, 4) == 1 );
    }
    
    TEST_CHECK(1);
}


static void test_simple_pass(void)
{
    TEST_CHECK( subreg_match("hello", "hello", 0, 0, 4) == 1 );
}


static void test_simple_fail(void)
{
    TEST_CHECK( subreg_match("hello", "goodbye", 0, 0, 4) == 0 );
}


static void test_digit(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = DIGIT_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\d", buffer, 0, 0, 4) == 1 );
    }
}


static void test_non_digit(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = NON_DIGIT_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\D", buffer, 0, 0, 4) == 1 );
    }
}


static void test_hex_digit(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = HEX_DIGIT_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\h", buffer, 0, 0, 4) == 1 );
    }
}


static void test_non_hex_digit(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = NON_HEX_DIGIT_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\H", buffer, 0, 0, 4) == 1 );
    }
}


static void test_whitespace(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = WHITESPACE_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\s", buffer, 0, 0, 4) == 1 );
    }
}


static void test_non_whitespace(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = NON_WHITESPACE_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\S", buffer, 0, 0, 4) == 1 );
    }
}


static void test_word(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = WORD_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\w", buffer, 0, 0, 4) == 1 );
    }
}


static void test_non_word(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = NON_WORD_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match("\\W", buffer, 0, 0, 4) == 1 );
    }
}


static void test_backspace(void)
{
    TEST_CHECK( subreg_match("\\b", "\b", 0, 0, 4) == 1 );
}


static void test_form_feed(void)
{
    TEST_CHECK( subreg_match("\\f", "\f", 0, 0, 4) == 1 );
}


static void test_new_line(void)
{
    TEST_CHECK( subreg_match("\\n", "\n", 0, 0, 4) == 1 );
}


static void test_carriage_return(void)
{
    TEST_CHECK( subreg_match("\\r", "\r", 0, 0, 4) == 1 );
}


static void test_horizontal_tab(void)
{
    TEST_CHECK( subreg_match("\\t", "\t", 0, 0, 4) == 1 );
}


static void test_vertical_tab(void)
{
    TEST_CHECK( subreg_match("\\v", "\v", 0, 0, 4) == 1 );
}


static void test_hex_escape_code(void)
{
    char regex[5];
    char input[2];
    char c;
    
    input[1] = '\0';
    
    for (c = 1; c > 0; c++)
    {
        sprintf(regex, "\\x%02x", c);
        input[0] = c;
        
        TEST_CHECK( subreg_match(regex, input, 0, 0, 4) == 1 );
    }
}


static void test_one_or_more_hex_escape_codes(void) {
    TEST_CHECK( subreg_match("\\x21+", "!!!!", 0, 0, 4) == 1 );
}


static void test_any(void)
{
    char buffer[2];
    const char* c;
    
    buffer[1] = '\0';
    
    for (c = ANY_SET; *c; c++)
    {
        buffer[0] = *c;
        TEST_CHECK( subreg_match(".", buffer, 0, 0, 4) == 1 );
    }
}


static void test_optional_none(void)
{
    TEST_CHECK( subreg_match("x?", "", 0, 0, 4) == 1 );
}


static void test_optional_one(void)
{
    TEST_CHECK( subreg_match("x?", "x", 0, 0, 4) == 1 );
}


static void test_zero_or_more(void)
{
    unsigned int i;
    char buffer[256];
    
    for (i = 0; i < 255; i++)
    {
        buffer[i] = '\0';
        TEST_CHECK( subreg_match(".*", buffer, 0, 0, 4) == 1 );
        buffer[i] = 'x';
    }
}


static void test_one_or_more_pass(void)
{
    unsigned int i;
    char buffer[256];
    
    buffer[0] = 'x';
    
    for (i = 1; i < 255; i++)
    {
        buffer[i] = '\0';
        TEST_CHECK( subreg_match(".+", buffer, 0, 0, 4) == 1 );
        buffer[i] = 'x';
    }
}


static void test_one_or_more_fail(void)
{
    TEST_CHECK( subreg_match(".+", "", 0, 0, 4) == 0 );
}


static void test_or_one_of_two(void)
{
    TEST_CHECK( subreg_match("a|b", "a", 0, 0, 4) == 1 );
}


static void test_or_two_of_two(void)
{
    TEST_CHECK( subreg_match("a|b", "b", 0, 0, 4) == 1 );
}


static void test_or_none_of_two(void)
{
    TEST_CHECK( subreg_match("a|b", "c", 0, 0, 4) == 0 );
}


static void test_or_one_of_three(void)
{
    TEST_CHECK( subreg_match("a|b|c", "a", 0, 0, 4) == 1 );
}


static void test_or_two_of_three(void)
{
    TEST_CHECK( subreg_match("a|b|c", "b", 0, 0, 4) == 1 );
}


static void test_or_three_of_three(void)
{
    TEST_CHECK( subreg_match("a|b|c", "c", 0, 0, 4) == 1 );
}


static void test_or_none_of_three(void)
{
    TEST_CHECK( subreg_match("a|b|c", "d", 0, 0, 4) == 0 );
}


static void test_simple_ncg_pass(void)
{
    TEST_CHECK( subreg_match("(?:foo)", "foo", 0, 0, 4) == 1 );
}


static void test_simple_ncg_fail(void)
{
    TEST_CHECK( subreg_match("(?:foo)", "bar", 0, 0, 4) == 0 );
}


static void test_one_or_more_ncg_pass(void)
{
    TEST_CHECK( subreg_match("(?:foo)+", "foofoofoo", 0, 0, 4) == 1 );
}


static void test_one_or_more_ncg_fail(void)
{
    TEST_CHECK( subreg_match("(?:foo)+", "barfoofoo", 0, 0, 4) == 0 );
}


static void test_overall_capture(void)
{
    subreg_capture_t cap[1];
    
    TEST_CHECK( subreg_match("test", "test", cap, 1, 4) == 1 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "test", 4) == 0 );
}


static void test_single_group_capture(void)
{
    subreg_capture_t cap[2];
    
    TEST_CHECK( subreg_match("(test)", "test", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "test", 4) == 0 );
    TEST_CHECK( cap[1].length == 4 );
    TEST_CHECK( memcmp(cap[1].start, "test", 4) == 0 );
}


static void test_multiple_group_capture(void)
{
    subreg_capture_t cap[3];
    
    TEST_CHECK( subreg_match("(foo) (bar)", "foo bar", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 7 );
    TEST_CHECK( memcmp(cap[0].start, "foo bar", 7) == 0 );
    TEST_CHECK( cap[1].length == 3 );
    TEST_CHECK( memcmp(cap[1].start, "foo", 3) == 0 );
    TEST_CHECK( cap[2].length == 3 );
    TEST_CHECK( memcmp(cap[2].start, "bar", 3) == 0 );
}


static void test_repeat_group_capture(void)
{
    subreg_capture_t cap[3];
    
    TEST_CHECK( subreg_match("(test)+", "testtest", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 8 );
    TEST_CHECK( memcmp(cap[0].start, "testtest", 4) == 0 );
    TEST_CHECK( cap[1].length == 4 );
    TEST_CHECK( memcmp(cap[1].start, "test", 4) == 0 );
    TEST_CHECK( cap[2].length == 4 );
    TEST_CHECK( memcmp(cap[2].start, "test", 4) == 0 );
}


static void test_capture_with_no_array(void)
{
    TEST_CHECK( subreg_match("(hello)", "hello", NULL, 0, 4) == 1 );
}


static void test_single_class_capture(void)
{
    subreg_capture_t cap[2];
    
    TEST_CHECK( subreg_match("(\\d)", "1", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 1 );
    TEST_CHECK( memcmp(cap[0].start, "1", 1) == 0 );
    TEST_CHECK( cap[1].length == 1 );
    TEST_CHECK( memcmp(cap[1].start, "1", 1) == 0 );
}


static void test_single_non_class_capture(void)
{
    subreg_capture_t cap[2];
    
    TEST_CHECK( subreg_match("(\\D)", "a", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 1 );
    TEST_CHECK( memcmp(cap[0].start, "a", 1) == 0 );
    TEST_CHECK( cap[1].length == 1 );
    TEST_CHECK( memcmp(cap[1].start, "a", 1) == 0 );
}


static void test_repeat_class_capture(void)
{
    subreg_capture_t cap[5];

    TEST_CHECK( subreg_match("(\\d)+", "1234", cap, 5, 4) == 5 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "1234", 4) == 0 );
    TEST_CHECK( cap[1].length == 1 );
    TEST_CHECK( *(cap[1].start) == '1' );
    TEST_CHECK( cap[2].length == 1 );
    TEST_CHECK( *(cap[2].start) == '2' );
    TEST_CHECK( cap[3].length == 1 );
    TEST_CHECK( *(cap[3].start) == '3' );
    TEST_CHECK( cap[4].length == 1 );
    TEST_CHECK( *(cap[4].start) == '4' );
}

// test for issue #2 raised by gogo9th on github
static void test_repeat_non_class_capture(void)
{
    subreg_capture_t cap[5];

    TEST_CHECK( subreg_match("(\\D)+", "abcd", cap, 5, 4) == 5 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "abcd", 4) == 0 );
    TEST_CHECK( cap[1].length == 1 );
    TEST_CHECK( *(cap[1].start) == 'a' );
    TEST_CHECK( cap[2].length == 1 );
    TEST_CHECK( *(cap[2].start) == 'b' );
    TEST_CHECK( cap[3].length == 1 );
    TEST_CHECK( *(cap[3].start) == 'c' );
    TEST_CHECK( cap[4].length == 1 );
    TEST_CHECK( *(cap[4].start) == 'd' );
}


static void test_capture_or_first(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("(AB|CD)", "AB", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 2 );
    TEST_CHECK( memcmp(cap[0].start, "AB", 2) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "AB", 2) == 0 );
}


static void test_capture_or_second(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("(AB|CD)", "CD", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 2 );
    TEST_CHECK( memcmp(cap[0].start, "CD", 2) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "CD", 2) == 0 );
}


static void test_capture_or_then_char(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("(AB|CD)C", "ABC", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 3 );
    TEST_CHECK( memcmp(cap[0].start, "ABC", 3) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "AB", 2) == 0 );
}


// test for issue #4 raised by gogo9th on github
static void test_capture_or_1_plus_then_char(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("(AB|CD)+C", "ABC", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 3 );
    TEST_CHECK( memcmp(cap[0].start, "ABC", 3) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "AB", 2) == 0 );
}


static void test_capture_or_2_plus_then_char(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("(AB|CD)+C", "CDC", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 3 );
    TEST_CHECK( memcmp(cap[0].start, "CDC", 3) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "CD", 2) == 0 );
}


static void test_capture_or_plus_1(void)
{
    subreg_capture_t cap[3];

    TEST_CHECK( subreg_match("(AB|CD)+", "ABAB", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "ABAB", 4) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "AB", 2) == 0 );
    TEST_CHECK( cap[2].length == 2 );
    TEST_CHECK( memcmp(cap[2].start, "AB", 2) == 0 );
}


static void test_capture_or_plus_2(void)
{
    subreg_capture_t cap[3];

    TEST_CHECK( subreg_match("(AB|CD)+", "CDCD", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "CDCD", 4) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "CD", 2) == 0 );
    TEST_CHECK( cap[2].length == 2 );
    TEST_CHECK( memcmp(cap[2].start, "CD", 2) == 0 );
}


static void test_capture_or_plus_3(void)
{
    subreg_capture_t cap[3];

    TEST_CHECK( subreg_match("(AB|CD)+", "ABCD", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "ABCD", 4) == 0 );
    TEST_CHECK( cap[1].length == 2 );
    TEST_CHECK( memcmp(cap[1].start, "AB", 2) == 0 );
    TEST_CHECK( cap[2].length == 2 );
    TEST_CHECK( memcmp(cap[2].start, "CD", 2) == 0 );
}


static void test_pos_look_ahead(void)
{
    subreg_capture_t cap[2];
    
    TEST_CHECK( subreg_match("(?=hello)(.*)", "hello world", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 11 );
    TEST_CHECK( memcmp(cap[0].start, "hello world", 11) == 0 );
    TEST_CHECK( cap[1].length == 11 );
    TEST_CHECK( memcmp(cap[1].start, "hello world", 11) == 0 );
}


static void test_neg_look_ahead(void)
{
    subreg_capture_t cap[2];
    
    TEST_CHECK( subreg_match("(?!hello)(.*)", "hello world", cap, 2, 4) == 0 );
}


// test for ? issue #5 raised by gogo9th on github
static void test_optional_capture_no_match(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("B(AAC)?AAD", "BAAD", cap, 2, 4) == 1 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "BAAD", 4) == 0 );
}


static void test_optional_capture_match(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("B(AAC)?AAD", "BAACAAD", cap, 2, 4) == 2 );
    TEST_CHECK( cap[0].length == 7 );
    TEST_CHECK( memcmp(cap[0].start, "BAACAAD", 7) == 0 );
    TEST_CHECK( cap[1].length == 3 );
    TEST_CHECK( memcmp(cap[1].start, "AAC", 3) == 0 );
}


// test for * issue #5 raised by gogo9th on github
static void test_zero_more_capture_no_match(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("B(AAC)*AAD", "BAAD", cap, 2, 4) == 1 );
    TEST_CHECK( cap[0].length == 4 );
    TEST_CHECK( memcmp(cap[0].start, "BAAD", 4) == 0 );
}


static void test_zero_more_capture_match(void)
{
    subreg_capture_t cap[3];

    TEST_CHECK( subreg_match("B(AAC)*AAD", "BAACAACAAD", cap, 3, 4) == 3 );
    TEST_CHECK( cap[0].length == 10 );
    TEST_CHECK( memcmp(cap[0].start, "BAACAACAAD", 10) == 0 );
    TEST_CHECK( cap[1].length == 3 );
    TEST_CHECK( memcmp(cap[1].start, "AAC", 3) == 0 );
    TEST_CHECK( cap[2].length == 3 );
    TEST_CHECK( memcmp(cap[2].start, "AAC", 3) == 0 );
}


static void test_nocase_option_enable_whole(void)
{
    TEST_CHECK( subreg_match("(?i)abcdefghijklmnopqrstuvwxyz0123456789", "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789", NULL, 0, 4) == 1 );
}


static void test_nocase_option_disable_whole(void)
{
    TEST_CHECK( subreg_match("(?I)abcdefghijklmnopqrstuvwxyz0123456789", "AbCdEfGhIjKlMnOpQrStUvWxYz0123456789", NULL, 0, 4) == 0 );
    TEST_CHECK( subreg_match("(?I)abcdefghijklmnopqrstuvwxyz0123456789", "abcdefghijklmnopqrstuvwxyz0123456789", NULL, 0, 4) == 1 );
}


static void test_nocase_option_partial(void)
{
    TEST_CHECK( subreg_match("first (?i)second(?I) third", "first SECOND third", NULL, 0, 4) == 1);
    TEST_CHECK( subreg_match("first (?i)second(?I) third", "first SeCoNd third", NULL, 0, 4) == 1);
    TEST_CHECK( subreg_match("first (?i)second(?I) third", "first SECOND THIRD", NULL, 0, 4) == 0);
}


static void test_invalid_option(void)
{
    char re[8];

    for (char c = 'a'; c <= 'z'; c++)
    {
        if ( c == 'i' ) continue;

        snprintf(re, 8, "(?%c).*", c);
        TEST_CHECK( subreg_match(re, "ignored", NULL, 0, 4) == SUBREG_RESULT_INVALID_OPTION );

        snprintf(re, 8, "(?%c).*", c - 'a' + 'A');
        TEST_CHECK( subreg_match(re, "ignored", NULL, 0, 4) == SUBREG_RESULT_INVALID_OPTION );
    }
}


static void test_invalid_inverted_match(void)
{
    TEST_CHECK( subreg_match("\\!", "x", NULL, 0, 4) == SUBREG_RESULT_INVALID_METACHARACTER );
}


static void test_inverted_match(void)
{
    TEST_CHECK( subreg_match("\\!a", "b", NULL, 0, 4) == 1 );
}


static void test_inverted_non_match(void)
{
    TEST_CHECK( subreg_match("\\!b", "b", NULL, 0, 4) == 0 );
}


static void test_one_or_more_inverted_match(void)
{
    TEST_CHECK( subreg_match("\\!a+", "bcd", NULL, 0, 4) == 1 );
}


static void test_one_or_more_inverted_non_match(void)
{
    TEST_CHECK( subreg_match("\\!a+", "bcda", NULL, 0, 4) == 0 );
}


static void test_capture_inverted_match(void)
{
    subreg_capture_t cap[2];

    TEST_CHECK( subreg_match("foo\"(\\!\"+)\"bar", "foo\"test\"bar", cap, 2, 4) == 2 );
    TEST_CHECK( cap[1].length == 4 );
    TEST_CHECK( memcmp(cap[1].start, "test", 4) == 0 );
}


static void test_inverted_hex_match(void)
{
    TEST_CHECK( subreg_match("\\!\\x21", "A", NULL, 0, 4) == 1 );
}


static void test_inverted_hex_non_match(void)
{
    TEST_CHECK( subreg_match("\\!\\x21", "!", NULL, 0, 4) == 0 );
}


TEST_LIST =
{
    {"empty_pass",                          test_empty_pass},
    {"empty_fail",                          test_empty_fail},
    {"literal",                             test_literal},
    {"simple_pass",                         test_simple_pass},
    {"simple_fail",                         test_simple_fail},
    {"digit",                               test_digit},
    {"non_digit",                           test_non_digit},
    {"hex_digit",                           test_hex_digit},
    {"non_hex_digit",                       test_non_hex_digit},
    {"one_or_more_hex_escape_codes",        test_one_or_more_hex_escape_codes},
    {"whitespace",                          test_whitespace},
    {"non_whitespace",                      test_non_whitespace},
    {"word",                                test_word},
    {"non_word",                            test_non_word},
    {"backspace",                           test_backspace},
    {"form_feed",                           test_form_feed},
    {"new_line",                            test_new_line},
    {"carriage_return",                     test_carriage_return},
    {"horizontal_tab",                      test_horizontal_tab},
    {"vertical_tab",                        test_vertical_tab},
    {"hex_escape_code",                     test_hex_escape_code},
    {"any",                                 test_any},
    {"optional_none",                       test_optional_none},
    {"optional_one",                        test_optional_one},
    {"zero_or_more",                        test_zero_or_more},
    {"one_or_more_pass",                    test_one_or_more_pass},
    {"one_or_more_fail",                    test_one_or_more_fail},
    {"or_one_of_two",                       test_or_one_of_two},
    {"or_two_of_two",                       test_or_two_of_two},
    {"or_none_of_two",                      test_or_none_of_two},
    {"or_one_of_three",                     test_or_one_of_three},
    {"or_two_of_three",                     test_or_two_of_three},
    {"or_three_of_three",                   test_or_three_of_three},
    {"or_none_of_three",                    test_or_none_of_three},
    {"simple_ncg_pass",                     test_simple_ncg_pass},
    {"simple_ncg_fail",                     test_simple_ncg_fail},
    {"one_or_more_ncg_pass",                test_one_or_more_ncg_pass},
    {"one_or_more_ncg_fail",                test_one_or_more_ncg_fail},
    {"overall_capture",                     test_overall_capture},
    {"single_group_capture",                test_single_group_capture},
    {"multiple_group_capture",              test_multiple_group_capture},
    {"repeat_group_capture",                test_repeat_group_capture},
    {"capture_with_no_array",               test_capture_with_no_array},
    {"single_class_capture",                test_single_class_capture},
    {"single_non_class_capture",            test_single_non_class_capture},
    {"repeat_class_capture",                test_repeat_class_capture},
    {"repeat_non_class_capture",            test_repeat_non_class_capture},
    {"capture_or_first",                    test_capture_or_first},
    {"capture_or_second",                   test_capture_or_second},
    {"capture_or_then_char",                test_capture_or_then_char},
    {"capture_or_1_plus_then_char",         test_capture_or_1_plus_then_char},
    {"capture_or_2_plus_then_char",         test_capture_or_2_plus_then_char},
    {"capture_or_plus_1",                   test_capture_or_plus_1},
    {"capture_or_plus_2",                   test_capture_or_plus_2},
    {"capture_or_plus_3",                   test_capture_or_plus_3},
    {"pos_look_ahead",                      test_pos_look_ahead},
    {"neg_look_ahead",                      test_neg_look_ahead},
    {"optional_capture_no_match",           test_optional_capture_no_match},
    {"optional_capture_match",              test_optional_capture_match},
    {"zero_more_capture_no_match",          test_zero_more_capture_no_match},
    {"zero_more_capture_match",             test_zero_more_capture_match},
    {"nocase_option_enable_whole",          test_nocase_option_enable_whole},
    {"nocase_option_disable_whole",         test_nocase_option_disable_whole},
    {"nocase_option_partial",               test_nocase_option_partial},
    {"invalid_option",                      test_invalid_option},
    {"invalid_inverted_match",              test_invalid_inverted_match},
    {"inverted_match",                      test_inverted_match},
    {"inverted_non_match",                  test_inverted_non_match},
    {"one_or_more_inverted_match",          test_one_or_more_inverted_match},
    {"one_or_more_inverted_non_match",      test_one_or_more_inverted_non_match},
    {"capture_inverted_match",              test_capture_inverted_match},
    {"inverted_hex_match",                  test_inverted_hex_match},
    {"inverted_hex_non_match",              test_inverted_hex_non_match},
    {0}
};

