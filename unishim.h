#ifndef INCLUDE_UNISHIM_H
#define INCLUDE_UNISHIM_H

/* unishim.h - C99/C++ utf-8/utf-16/utf-32 conversion header

This file is released to the public domain under US law,
and also released under any version of the Creative Commons Zero license: 
https://creativecommons.org/publicdomain/zero/1.0/
https://creativecommons.org/publicdomain/zero/1.0/legalcode
*/

#include <stdint.h>
#include <stdlib.h>
#include <iso646.h>

// uint8_t * utf16_to_utf8(uint16_t * utf16, int * status)
// uint16_t * utf8_to_utf16(uint8_t * utf8, int * status)
// uint8_t * utf32_to_utf8(uint32_t * utf32, int * status)
// uint32_t * utf8_to_utf32(uint8_t * utf8, int * status)

// status 0 is success, other indicates failure
// null returned if failure
// returned string is freshly allocated from malloc()
// Input string must NOT be modified by another thread while these functions are running.

/*
allocates and returns a utf-8 string converted from the utf-16 one
assumes that the utf-16 string is in native endian

if an error is encountered, sets status and returns 0:
0 - no error
1 - second surrogate where simple codepoint or first surrogate expected
2 - null where second surrogate expected
3 - first surrogate or simple codepoint where second surrogate expected
4 - failed to allocate return buffer
*/
uint8_t * utf16_to_utf8(uint16_t * utf16, int * status)
{
    *status = 0;
    uint16_t * counter = utf16;
    size_t len = 0;
    
    // get length of corresponding utf-8 string in bytes
    while(counter[0] != 0)
    {
        // trivial codepoint
        if(counter[0] < 0xD800 or counter[0] >= 0xE000)
        {
            if(counter[0] < 0x80)
                len += 1;
            else if(counter[0] < 0x800)
                len += 2;
            else // maximum utf-8 length of a single utf-16 code unit codepoint
                len += 3;
            counter++;
        }
        else
        {
            // surrogate
            // continuation surrogate
            if(counter[0] >= 0xDC00)
            {
                *status = 1;
                return 0;
            }
            // counter[0] is now guaranteed to be a valid first/high surrogate
            // continuation surrogate is null
            else if(counter[1] == 0)
            {
                *status = 2;
                return 0;
            }
            // continuation surrogate is not a continuation surrogate
            else if(counter[1] < 0xDC00 or counter[0] >= 0xE000)
            {
                *status = 3;
                return 0;
            }
            // counter[1] is now guaranteed to be a valid second/low surrogate
            len += 4;
            counter += 2;
        }
    }
    counter = utf16;
    
    uint8_t * utf8 = (uint8_t *)malloc(len+1);
    if(!utf8)
    {
        *status = 4;
        return 0;
    }
    
    size_t i = 0;
    
    // get length of corresponding utf-8 string in bytes
    while(counter[0] != 0)
    {
        if(counter[0] < 0xD800 or counter[0] >= 0xE000)
        {
            if(counter[0] < 0x80)
            {
                utf8[i++] = counter[0];
            }
            else if(counter[0] < 0x800)
            {
                uint8_t low = counter[0]&0x3F;
                uint8_t high = (counter[0]>>6)&0x1F;
                low |= 0x80;
                high |= 0xC0;
                utf8[i++] = high;
                utf8[i++] = low;
            }
            else
            {
                uint8_t low = counter[0]&0x3F;
                uint8_t mid = (counter[0]>>6)&0x3F;
                uint8_t high = (counter[0]>>12)&0x0F;
                low |= 0x80;
                mid |= 0x80;
                high |= 0xE0;
                utf8[i++] = high;
                utf8[i++] = mid;
                utf8[i++] = low;
            }
            counter++;
        }
        else
        {
            // range was validated in length pass
            uint32_t in_low = counter[1]&0x03FF;
            uint32_t in_high = counter[0]&0x03FF;
            uint32_t codepoint = (in_high<<10) | in_low;
            codepoint += 0x10000;
            
            uint8_t low = codepoint&0x3F;
            uint8_t mid = (codepoint>>6)&0x3F;
            uint8_t high = (codepoint>>12)&0x3F;
            uint8_t top = (codepoint>>18)&0x07;
            low |= 0x80;
            mid |= 0x80;
            high |= 0x80;
            top |= 0xF0;
            
            utf8[i++] = top;
            utf8[i++] = high;
            utf8[i++] = mid;
            utf8[i++] = low;
            
            counter += 2;
        }
    }
    
    utf8[i] = 0;
    
    return utf8;
}

/*
allocates and returns a utf-16 string converted from the utf-8 one
assumes that the utf-16 string is in native endian

if an error is encountered, sets status and returns 0:
0 - no error
1 - continuation byte encountered where initial byte expected
2 - null encountered when continuation byte expected
3 - initial byte encountered where continuation byte expected
4 - decoded into utf-16 surrogate, which is not a true codepoint
5 - decoded into value too large to store in utf-16
6 - decoded into value with mismatched size (overlong encoding)
7 - failed to allocate return buffer

error 6 takes priority over error 4 if a surrogate is given overlong encoding
*/
uint16_t * utf8_to_utf16(uint8_t * utf8, int * status)
{
    *status = 0;
    uint8_t * counter = utf8;
    size_t len = 0;
    
    // get length of corresponding utf-8 string in bytes
    while(counter[0] != 0)
    {   
        // trivial byte
        if(counter[0] < 0x80)
        {
            len += 2;
            counter += 1;
        }
        // continuation byte where initial byte expected
        else if(counter[0] < 0xC0)
        {
            *status = 1;
            return 0;
        }
        // two byte
        else if(counter[0] < 0xE0)
        {
            for(int index = 1; index <= 1; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 2;
            counter += 2;
        }
        // three byte
        else if(counter[0] < 0xF0)
        {
            for(int index = 1; index <= 2; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 2;
            counter += 3;
        }
        // four byte
        else if(counter[0] < 0xF8)
        {
            for(int index = 1; index <= 3; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 4;
            counter += 4;
        }
        else
        {
            *status = 1;
            return 0;
        }
    }
    counter = utf8;
    
    uint16_t * utf16 = (uint16_t *)malloc(len+2);
    if(!utf16)
    {
        *status = 7;
        return 0;
    }
    
    size_t i = 0;
    
    while(counter[0] != 0)
    {   
        if(counter[0] < 0x80)
        {
            utf16[i++] = counter[0];
            counter += 1;
        }
        // codepoint-level streem coherence was validated on the length pass
        else if(counter[0] < 0xE0)
        {
            uint16_t high = counter[0]&0x1F;
            uint16_t low = counter[1]&0x3F;
            uint16_t codepoint = (high<<6) | low;
            // overlong codepoint
            if(codepoint < 0x80)
            {
                *status = 6;
                free(utf16);
                return 0;
            }
            else
                utf16[i++] = codepoint;
            counter += 2;
        }
        else if(counter[0] < 0xF0)
        {
            uint16_t high = counter[0]&0x0F;
            uint16_t mid = counter[1]&0x3F;
            uint16_t low = counter[2]&0x3F;
            
            uint16_t codepoint = (high<<12) | (mid<<6) | low;
            // overlong codepoint
            if(codepoint < 0x800)
            {
                *status = 6;
                free(utf16);
                return 0;
            }
            // encoded surrogate
            else if(codepoint >= 0xD800 and codepoint < 0xE000)
            {
                *status = 4;
                free(utf16);
                return 0;
            }
            else
                utf16[i++] = codepoint;
            counter += 3;
        }
        else if(counter[0] < 0xF8)
        {
            uint32_t top = counter[0]&0x07;
            uint32_t high = counter[1]&0x3F;
            uint32_t mid = counter[2]&0x3F;
            uint32_t low = counter[3]&0x3F;
            
            uint32_t codepoint = (top<<18) | (high<<12) | (mid<<6) | low;
            // overlong codepoint
            if(codepoint < 0x10000)
            {
                *status = 6;
                free(utf16);
                return 0;
            }
            // codepoint too large for utf-16
            else if(codepoint >= 0x110000)
            {
                *status = 5;
                free(utf16);
                return 0;
            }
            else
            {
                codepoint -= 0x10000;
                uint16_t upper = codepoint>>10;
                uint16_t lower = codepoint&0x3FF;
                upper += 0xD800;
                lower += 0xDC00;
                utf16[i++] = upper;
                utf16[i++] = lower;
            }
            counter += 4;
        }
        else
        {
            *status = 1;
            return 0;
        }
    }
    return utf16;
}

/*
allocates and returns a utf-8 string converted from the utf-32 one
assumes that the utf-32 string is in native endian

if an error is encountered, sets status and returns 0:
0 - no error
1 - codepoint is a utf-16 surrogate
2 - codepoint is too large to store in utf-16, which is forbidden by modern utf-8
3 - failed to allocate return buffer
*/
uint8_t * utf32_to_utf8(uint32_t * utf32, int * status)
{
    *status = 0;
    uint32_t * counter = utf32;
    size_t len = 0;
    
    // get length of corresponding utf-8 string in bytes
    while(counter[0] != 0)
    {   
        if(counter[0] < 0x80)
            len += 1;
        else if(counter[0] < 0x800)
            len += 2;
        else if(counter[0] < 0x10000)
        {
            if(counter[0] >= 0xD800 and counter[0] < 0xE000)
            {
                *status = 1;
                return 0;
            }
            len += 3;
        }
        else if(counter[0] < 0x110000)
        {
            len += 4;
        }
        else
        {
            *status = 2;
            return 0;
        }
        counter += 1;
    }
    counter = utf32;
    
    uint8_t * utf8 = (uint8_t *)malloc(len+1);
    if(!utf8)
    {
        *status = 3;
        return 0;
    }
    
    size_t i = 0;
    
    while(counter[0] != 0)
    {   
        if(counter[0] < 0x80)
            utf8[i++] = counter[0];
        // codepoint-level stream coherence was validated on the length pass
        else if(counter[0] < 0x800)
        {
            uint8_t high = counter[0]>>6;
            uint8_t low = counter[0]&0x3F;
            high |= 0xC0;
            low |= 0x80;
            utf8[i++] = high;
            utf8[i++] = low;
        }
        else if(counter[0] < 0x10000)
        {
            uint8_t high = counter[0]>>12;
            uint8_t mid = (counter[0]>>6)&0x3F;
            uint8_t low = counter[0]&0x3F;
            high |= 0xE0;
            mid |= 0x80;
            low |= 0x80;
            utf8[i++] = high;
            utf8[i++] = mid;
            utf8[i++] = low;
        }
        else
        {
            uint8_t top = counter[0]>>18;
            uint8_t high = (counter[0]>>12)&0x3F;
            uint8_t mid = (counter[0]>>6)&0x3F;
            uint8_t low = counter[0]&0x3F;
            top |= 0xF0;
            high |= 0x80;
            mid |= 0x80;
            low |= 0x80;
            utf8[i++] = top;
            utf8[i++] = high;
            utf8[i++] = mid;
            utf8[i++] = low;
        }
        counter += 1;
    }
    return utf8;
}

/*
allocates and returns a utf-32 string converted from the utf-8 one
assumes that the utf-32 string is in native endian

if an error is encountered, sets status and returns 0:
0 - no error
1 - continuation byte encountered where initial byte expected
2 - null encountered when continuation byte expected
3 - initial byte encountered where continuation byte expected
4 - decoded into utf-16 surrogate, which is not a true codepoint
5 - decoded into value too large to store in utf-16, which is forbidden by modern utf-8
6 - decoded into value with mismatched size (overlong encoding)
7 - failed to allocate return buffer

error 6 takes priority over error 4 if a surrogate is given overlong encoding
*/
uint32_t * utf8_to_utf32(uint8_t * utf8, int * status)
{
    *status = 0;
    uint8_t * counter = utf8;
    size_t len = 0;
    
    // get length of corresponding utf-8 string in bytes
    while(counter[0] != 0)
    {   
        // trivial byte
        if(counter[0] < 0x80)
        {
            len += 4;
            counter += 1;
        }
        // continuation byte where initial byte expected
        else if(counter[0] < 0xC0)
        {
            *status = 1;
            return 0;
        }
        // two byte
        else if(counter[0] < 0xE0)
        {
            for(int index = 1; index <= 1; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 4;
            counter += 2;
        }
        // three byte
        else if(counter[0] < 0xF0)
        {
            for(int index = 1; index <= 2; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 4;
            counter += 3;
        }
        // four byte
        else if(counter[0] < 0xF8)
        {
            for(int index = 1; index <= 3; index++)
            {
                // unexpected null
                if(counter[index] == 0)
                {
                    *status = 2;
                    return 0;
                }
                // bad continuation byte
                else if(counter[index] < 0x80 or counter[index] >= 0xC0)
                {
                    *status = 3;
                    return 0;
                }
            }
            len += 4;
            counter += 4;
        }
        else
        {
            *status = 1;
            return 0;
        }
    }
    counter = utf8;
    
    uint32_t * utf32 = (uint32_t *)malloc(len+4);
    if(!utf32)
    {
        *status = 7;
        return 0;
    }
    
    size_t i = 0;
    
    while(counter[0] != 0)
    {   
        if(counter[0] < 0x80)
        {
            utf32[i++] = counter[0];
            counter += 1;
        }
        // codepoint-level stream coherence was validated on the length pass
        else if(counter[0] < 0xE0)
        {
            uint16_t high = counter[0]&0x1F;
            uint16_t low = counter[1]&0x3F;
            uint16_t codepoint = (high<<6) | low;
            // overlong codepoint
            if(codepoint < 0x80)
            {
                *status = 6;
                free(utf32);
                return 0;
            }
            else
                utf32[i++] = codepoint;
            counter += 2;
        }
        else if(counter[0] < 0xF0)
        {
            uint16_t high = counter[0]&0x0F;
            uint16_t mid = counter[1]&0x3F;
            uint16_t low = counter[2]&0x3F;
            
            uint16_t codepoint = (high<<12) | (mid<<6) | low;
            // overlong codepoint
            if(codepoint < 0x800)
            {
                *status = 6;
                free(utf32);
                return 0;
            }
            // encoded surrogate
            else if(codepoint >= 0xD800 and codepoint < 0xE000)
            {
                *status = 4;
                free(utf32);
                return 0;
            }
            else
                utf32[i++] = codepoint;
            counter += 3;
        }
        else if(counter[0] < 0xF8)
        {
            uint32_t top = counter[0]&0x07;
            uint32_t high = counter[1]&0x3F;
            uint32_t mid = counter[2]&0x3F;
            uint32_t low = counter[3]&0x3F;
            
            uint32_t codepoint = (top<<18) | (high<<12) | (mid<<6) | low;
            // overlong codepoint
            if(codepoint < 0x10000)
            {
                *status = 6;
                free(utf32);
                return 0;
            }
            // codepoint too large for utf-16
            else if(codepoint >= 0x110000)
            {
                *status = 5;
                free(utf32);
                return 0;
            }
            else
            {
                utf32[i++] = codepoint;
            }
            counter += 4;
        }
        else
        {
            *status = 1;
            return 0;
        }
    }
    return utf32;
}

#endif
