/**
 * SubReg - A small footprint regular expression engine written in ANSI C.
 * 
 * https://github.com/mattbucknall/subreg
 * 
 * Copyright (c) 2016-2024 Matthew T. Bucknall
 *
 * Contributions:
 *
 *   Character sets and ranges contributed by BeetMacol
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

#include "subreg.h"

#define SUBREG_RESULT_INTERNAL_MATCH        1


#define SUBREG_OPTION_CHAR_SET_NOCASE       'i'
#define SUBREG_OPTION_CHAR_CLEAR_NOCASE     'I'


#define SUBREG_OPTION_NOCASE                (1 << 0)


typedef enum
{
    MODE_NON_CAPTURE,
    MODE_CAPTURE,
    MODE_POS_LOOK_AHEAD,
    MODE_NEG_LOOK_AHEAD

} mode_t;


typedef struct
{
    const char* regex;
    const char* input;
    subreg_capture_t* captures;
    unsigned int max_captures;
    int max_depth;
    unsigned int capture_index;
    int depth;
    int options;
    
} state_t;


static int parse_sub_expr(state_t* state);


static int is_end(char c)
{
    return (c == '\0');
}


static int is_internal_block_boundary(char c)
{
    return (c == '|') || (c == '$');
}


static int is_block_boundary(char c)
{
    return is_end(c) || is_internal_block_boundary(c);
}


static int skip_block(state_t* state)
{
    int depth;
    
    depth = state->depth;
    
    for (;;)
    {
        char rc;
        
        rc = state->regex[0];
        
        if ( is_end(rc) )
        {
            if ( state->depth > depth ) 
                    return SUBREG_RESULT_MISSING_BRACKET;
            
            break;
        }
        else if ( rc == '(' )
        {
            state->depth++;
            
            if ( state->depth > state->max_depth )
                    return SUBREG_RESULT_MAX_DEPTH_EXCEEDED;
        }
        else if ( rc == ')' )
        {
            if ( state->depth == depth ) break;
            state->depth--;
        }
        else if ( is_internal_block_boundary(rc) )
        {
            if ( state->depth == depth ) break;
        }
        else
        {
            if ( rc == '\\' ) state->regex++;
            
            rc = state->regex[0];
            if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
        }
        
        state->regex++;
    }
    
    return SUBREG_RESULT_INTERNAL_MATCH;
}


static int is_bad_result(int result)
{
    return (result < 0);
}


static int is_match_result(int result)
{
    return (result > 0);
}


static int match_digit(char c)
{
    return ((c >= '0') && (c <= '9')) ?
            SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int match_hexadecimal(char c)
{
    return ((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')) ?
            SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int match_word(char c)
{
    return ((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            ((c >= 'a') && (c <= 'z')) ||
            (c == '_') ?
            SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int match_whitespace(char c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\v') ||
            (c == '\f') || (c == '\r')) ?
            SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int match_option(char c)
{
    return ((c >= 'A') && (c <= 'Z')) ||
            ((c >= 'a') && (c <= 'z'));
}


static int match_char(state_t* state, char c1, char c2)
{
    if ( state->options & SUBREG_OPTION_NOCASE )
    {
        if ( c1 >= 'a' && c1 <= 'z' ) c1 = c1 - 'a' + 'A';
        if ( c2 >= 'a' && c2 <= 'z' ) c2 = c2 - 'a' + 'A';
    }

    return (c1 == c2) ?
            SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int match_range(state_t* state, char c, char c1, char c2)
{
    if ( state->options & SUBREG_OPTION_NOCASE )
    {
        if ( c >= 'a' && c <= 'z' ) c = c - 'a' + 'A';
        if ( c1 >= 'a' && c1 <= 'z' ) c1 = c1 - 'a' + 'A';
        if ( c2 >= 'a' && c2 <= 'z' ) c2 = c2 - 'a' + 'A';
    }

    return (c >= c1 && c <= c2) ?
           SUBREG_RESULT_INTERNAL_MATCH : SUBREG_RESULT_NO_MATCH;
}


static int invert_match(char c, int (*match_func)(char))
{
    int result;

    if ( is_end(c) ) return SUBREG_RESULT_NO_MATCH;

    result = match_func(c);

    if ( is_bad_result(result) ) return result;
    else return is_match_result(result) ?
        SUBREG_RESULT_NO_MATCH : SUBREG_RESULT_INTERNAL_MATCH;
}


static int decode_hex(state_t* state, unsigned char* c)
{
    char rc;
    
    rc = state->regex[0];
    
    if ( (rc >= '0') && (rc <= '9') ) c[0] |= (rc - '0');
    else if ( (rc >= 'A') && (rc <= 'F') ) c[0] |= (rc - 'A' + 10);
    else if ( (rc >= 'a') && (rc <= 'f') ) c[0] |= (rc - 'a' + 10);
    else return SUBREG_RESULT_INVALID_METACHARACTER;
         
    state->regex++;
    
    return SUBREG_RESULT_INTERNAL_MATCH;
}


static int decode_non_class_metacharacter(state_t* state, char* c)
{
    char rc;
    unsigned char temp;
    int result;
    
    rc = state->regex[0];
    if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
    
    state->regex++;
    
    switch (rc)
    {
    case 'D':
    case 'H':
    case 'S':
    case 'W':
    case 'd':
    case 'h':
    case 's':
    case 'w':
    case '!':   return SUBREG_RESULT_ILLEGAL_EXPRESSION;
    
    case 'b':   *c = '\b';  break;
    case 'f':   *c = '\f';  break;
    case 'n':   *c = '\n';  break;
    case 'r':   *c = '\r';  break;
    case 't':   *c = '\t';  break;
    case 'v':   *c = '\v';  break;
    
    case 'x':
        temp = 0;
        
        result = decode_hex(state, &temp);
        if ( is_bad_result(result) ) return result;
        
        temp <<= 4;
        
        result = decode_hex(state, &temp);
        if ( is_bad_result(result) ) return result;
        
        c[0] = (char) temp;
        break;
        
    default:
        c[0] = rc;
    }
    
    return SUBREG_RESULT_INTERNAL_MATCH;
}


static int parse_literal(state_t* state)
{
    int result;
    char c;
    char rc;
    mode_t mode;
    const char* input_start;
    unsigned int next_capture_index;
    subreg_capture_t* cap;
    
    c = state->input[0];
    
    rc = state->regex[0];
    if ( !is_end(rc) ) state->regex++;
    
    if ( rc == '(' )
    {
        state->depth++;
    
        if ( state->depth > state->max_depth )
                return SUBREG_RESULT_MAX_DEPTH_EXCEEDED;
        
        input_start = state->input;

        if ( state->regex[0] == '?' )
        {
            state->regex++;
            
            if ( state->regex[0] == ':' ) mode = MODE_NON_CAPTURE;
            else if ( state->regex[0] == '=' ) mode = MODE_POS_LOOK_AHEAD;
            else if ( state->regex[0] == '!' ) mode = MODE_NEG_LOOK_AHEAD;
            else if ( match_option(state->regex[0]) )
            {
                switch(state->regex[0])
                {
                case SUBREG_OPTION_CHAR_SET_NOCASE:
                    state->options |= SUBREG_OPTION_NOCASE;
                    break;

                case SUBREG_OPTION_CHAR_CLEAR_NOCASE:
                    state->options &= ~SUBREG_OPTION_NOCASE;
                    break;

                default:
                    return SUBREG_RESULT_INVALID_OPTION;
                }

                state->regex++;

                rc = state->regex[0];
                if ( rc != ')' ) return SUBREG_RESULT_MISSING_BRACKET;

                state->regex++;

                return SUBREG_RESULT_INTERNAL_MATCH;
            }
            else return SUBREG_RESULT_ILLEGAL_EXPRESSION;
            
            state->regex++;
        }
        else if ( state->max_captures > 0 ) mode = MODE_CAPTURE;
        else mode = MODE_NON_CAPTURE;
        
        result = parse_sub_expr(state);
        if ( is_bad_result(result) ) return result;
        
        rc = state->regex[0];
        if ( rc != ')' ) return SUBREG_RESULT_MISSING_BRACKET;
        
        state->regex++;
        
        if ( mode == MODE_CAPTURE )
        {
            if ( is_match_result(result) )
            {
                next_capture_index = state->capture_index + 1;
                
                if ( next_capture_index > state->max_captures )
                        return SUBREG_RESULT_CAPTURE_OVERFLOW;
            
                cap = &state->captures[state->capture_index];
                
                cap->start = input_start;
                cap->length = state->input - input_start;
                
                state->capture_index = next_capture_index;
            }
        }
        else if ( mode == MODE_POS_LOOK_AHEAD )
        {
            state->input = input_start;
        }
        else if ( mode == MODE_NEG_LOOK_AHEAD )
        {
            state->input = input_start;

            result = is_match_result(result) ?
                    SUBREG_RESULT_NO_MATCH : SUBREG_RESULT_INTERNAL_MATCH;
        }
        
        state->depth--;
        
        return result;
    }
    else if ( rc == '\\' )
    {
        rc = state->regex[0];
        if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
        
        switch (rc)
        {
        case 'D':   result = invert_match(c, match_digit);          break;
        case 'H':   result = invert_match(c, match_hexadecimal);    break;
        case 'S':   result = invert_match(c, match_whitespace);     break;
        case 'W':   result = invert_match(c, match_word);           break;
        case 'd':   result = match_digit(c);                        break;
        case 'h':   result = match_hexadecimal(c);                  break;
        case 's':   result = match_whitespace(c);                   break;
        case 'w':   result = match_word(c);                         break;
        case '!':
            state->regex++;
            rc = state->regex[0];
            if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;

            if ( rc == '\\' )
            {
                state->regex++;
                rc = state->regex[0];
                if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;

                result = decode_non_class_metacharacter(state, &rc);
                if ( is_bad_result(result) ) return result;
            }

            if ( is_end(c) ) return SUBREG_RESULT_NO_MATCH;
            
            result = match_char(state, c, rc);
            if ( is_bad_result(result) ) return result;
            
            result = is_match_result(result) ?
                    SUBREG_RESULT_NO_MATCH : SUBREG_RESULT_INTERNAL_MATCH;

            break;
        
        default:
            result = decode_non_class_metacharacter(state, &rc);
            if ( is_bad_result(result) ) return result;
            
            result = match_char(state, c, rc);
            if ( is_match_result(result) ) state->input++;
            
            return result;
        }
        
        state->regex++;
    }
    else if ( rc == '[' )
    {
        rc = state->regex[0];
        if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
        mode = MODE_CAPTURE; // NON_CAPTURE means inversion
        if ( rc == '^' )
        {
            mode = MODE_NON_CAPTURE;
            state->regex++;
            rc = state->regex[0];
        }
        result = SUBREG_RESULT_NO_MATCH;
        do { // note: ']' as first char does not close the set
            if ( state->regex[1] == '-' && match_word(state->regex[2]) )
            { // range
                result = match_range(state, c, rc, state->regex[2]);
                state->regex += 2;
            }
            else
            { // char
                result = match_char(state, c, rc);
            }
            if ( is_match_result(result) ) {
                do {
                    state->regex++;
                    rc = state->regex[0];
                    if ( is_end(rc) )
                            return SUBREG_RESULT_INVALID_METACHARACTER;
                } while ( rc != ']' );
                break;
            }
            state->regex++;
            rc = state->regex[0];
            if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
        } while ( rc != ']' );
        state->regex++;
        if ( mode == MODE_NON_CAPTURE /*&& !is_bad_result(result)*/ )
        {
            result = is_match_result(result) || is_end(c) ?
                    SUBREG_RESULT_NO_MATCH : SUBREG_RESULT_INTERNAL_MATCH;
        }
    }
    else
    {
        switch (rc)
        {
        case '.':
            result = is_end(c) ? SUBREG_RESULT_NO_MATCH :
                    SUBREG_RESULT_INTERNAL_MATCH;
            break;
            
        default:
            result = match_char(state, c, rc);
        }
    }
    
    if ( is_match_result(result) ) state->input++;
    
    return result;
}


static int parse_repetition(state_t* state)
{
    const char* regex_begin;
    const char* regex_end;
    const char* check_point;
    int result;
    char rc;
    
    regex_begin = state->regex;
    check_point = state->input;
    
    result = parse_literal(state);
    if ( is_bad_result(result) || is_end(state->regex[0]) ) return result;
    
    rc = state->regex[0];
    
    if ( rc == '?' )
    {
        state->regex++;
        if ( !is_match_result(result) ) state->input = check_point;
        return SUBREG_RESULT_INTERNAL_MATCH;
    }
    else if ( rc == '+' )
    {
        if ( !is_match_result(result) ) return SUBREG_RESULT_NO_MATCH;
    }
    else if ( rc == '*' )
    {
        if ( !is_match_result(result) )
        {
            state->regex++;
            state->input = check_point;
            return SUBREG_RESULT_INTERNAL_MATCH;
        }
    }
    else return result;
    
    regex_end = state->regex + 1;
    
    for (;;)
    {
        state->regex = regex_begin;
        check_point = state->input;

        result = parse_literal(state);
        if ( is_bad_result(result) ) return result;
        
        if ( !is_match_result(result) )
        {
            state->input = check_point;
            break;
        }
    }
    
    state->regex = regex_end;
    
    return SUBREG_RESULT_INTERNAL_MATCH;
}


static int parse_concatenation(state_t* state)
{
    do
    {
        int result;
        
        if ( state->regex[0] == ')' ) break;
        
        result = parse_repetition(state);
        if ( is_bad_result(result) || !is_match_result(result) ) return result;
    
    } while ( !is_block_boundary(state->regex[0]) );
    
    return SUBREG_RESULT_INTERNAL_MATCH;
}


static int parse_alternation(state_t* state)
{
    const char* input_begin;
    
    input_begin = state->input;
    
    for (;;)
    {
        int result;
        
        result = parse_concatenation(state);
        if ( is_bad_result(result) ) return result;
        
        if ( is_match_result(result) )
        {
            for (;;)
            {
                result = skip_block(state);
                if ( is_bad_result(result) ) return result;

                if ( state->regex[0] == '|' ) state->regex++;
                else return SUBREG_RESULT_INTERNAL_MATCH;
            }
        }
        
        result = skip_block(state);
        if ( is_bad_result(result) ) return result;
        
        if ( state->regex[0] != '|' ) break;
        
        state->regex++;
        state->input = input_begin;
    }
    
    return SUBREG_RESULT_NO_MATCH;
}


static int parse_sub_expr(state_t* state)
{
    int saved_options;
    int result;

    saved_options = state->options;
    result = parse_alternation(state);
    state->options = saved_options;

    return result;
}


static int parse_expr(state_t* state)
{
    int result;
    
    if ( state->regex[0] == '^' ) state->regex++;
    
    result = parse_sub_expr(state);
    if ( is_bad_result(result) ) return result;
    
    if ( is_match_result(result) )
    {
        if ( state->regex[0] == '$' ) state->regex++;
        
        if ( !is_end(state->regex[0]) )
            return SUBREG_RESULT_ILLEGAL_EXPRESSION;
        
        if ( is_end(state->input[0]) ) return SUBREG_RESULT_INTERNAL_MATCH;
        else return SUBREG_RESULT_NO_MATCH;
    }
    
    return SUBREG_RESULT_NO_MATCH;
}


int subreg_match(const char* regex, const char* input,
        subreg_capture_t captures[], unsigned int max_captures,
        unsigned int max_depth)
{
    state_t state;
    int result;
    
    if ( !regex || !input || (max_captures > 0 && !captures) )
        return SUBREG_RESULT_INVALID_ARGUMENT;
    
    state.regex = regex;
    state.input = input;
    state.captures = captures;
    state.max_captures = max_captures;
    state.max_depth = (int) max_depth;
    state.capture_index = 1;
    state.depth = 0;
    state.options = 0;
    
    result = parse_expr(&state);
    
    if ( result <= 0 )
    {
        return result;
    }
    else
    {
        if ( max_captures > 0 )
        {
            captures[0].start = input;
            captures[0].length = state.input - input;
        }
        
        return (int) (state.capture_index);
    } 
}
