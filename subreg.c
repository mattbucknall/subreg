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

#include "subreg.h"

#define SUBREG_RESULT_INTERNAL_MATCH    1


typedef struct
{
    const char* regex;
    const char* input;
    int max_depth;
    int depth;
    
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
            state->depth--;
            
            if ( state->depth < depth ) break;
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


static int match_char(char c1, char c2)
{
    return (c1 == c2) ? SUBREG_RESULT_INTERNAL_MATCH :
            SUBREG_RESULT_NO_MATCH;
}


static int match_digit(char c)
{
    return ((c >= '0') && (c <= '9')) ? SUBREG_RESULT_INTERNAL_MATCH :
            SUBREG_RESULT_NO_MATCH;
}


static int match_whitespace(char c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\v') ||
            (c == '\f') || (c == '\r')) ? SUBREG_RESULT_INTERNAL_MATCH :
            SUBREG_RESULT_NO_MATCH;
}


static int invert_match(int result)
{
    if ( is_bad_result(result) ) return result;
    
    return is_match_result(result) ? SUBREG_RESULT_NO_MATCH :
            SUBREG_RESULT_INTERNAL_MATCH;
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
    
    c = state->input[0];
    
    rc = state->regex[0];
    if ( !is_end(rc) ) state->regex++;
    
    if ( rc == '(' )
    {
        state->depth++;
        
        if ( state->depth > state->max_depth )
            return SUBREG_RESULT_MAX_DEPTH_EXCEEDED;
        
        result = parse_sub_expr(state);
        if ( is_bad_result(result) ) return result;
        
        rc = state->regex[0];
        if ( rc != ')' ) return SUBREG_RESULT_MISSING_BRACKET;
        
        state->regex++;
        
        return result;
    }
    else if ( rc == '\\' )
    {
        rc = state->regex[0];
        if ( is_end(rc) ) return SUBREG_RESULT_INVALID_METACHARACTER;
        
        switch (rc)
        {
        case 'S':   result = invert_match(match_whitespace(c)); break;
        case 'd':   result = match_digit(c);                    break;
        case 's':   result = match_whitespace(c);               break;
        
        default:
            result = decode_non_class_metacharacter(state, &rc);
            if ( is_bad_result(result) ) return result;
            
            result = match_char(c, rc);
            if ( is_match_result(result) ) state->input++;
            
            return result;
        }
        
        state->regex++;
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
            result = match_char(c, rc);
        }
    }
    
    if ( is_match_result(result) ) state->input++;
    
    return result;
}


static int parse_repetition(state_t* state)
{
    const char* regex_begin;
    const char* regex_end;
    int result;
    char rc;
    
    regex_begin = state->regex;
    
    result = parse_literal(state);
    if ( is_bad_result(result) || is_end(state->regex[0]) ) return result;
    
    rc = state->regex[0];
    
    if ( rc == '?' )
    {
        state->regex++;
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
            return SUBREG_RESULT_INTERNAL_MATCH;
        }
    }
    else return result;
    
    regex_end = state->regex + 1;
    
    for (;;)
    {
        state->regex = regex_begin;
        
        result = parse_literal(state);
        if ( is_bad_result(result) ) return result;
        
        if ( !is_match_result(result) ) break;
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
    int result;
    result = parse_alternation(state);
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
        int max_depth)
{
    state_t state;
    int result;
    
    state.regex = regex;
    state.input = input;
    state.max_depth = max_depth;
    state.depth = 0;
    
    result = parse_expr(&state);
    
    return result;
}
