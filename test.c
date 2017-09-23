#include "unishim.h"
#include <stdio.h>
#include <stdlib.h>

// utf-16 (little endian)
uint8_t test[] =
"\x50\x30\x66\x30\x93\x30\xe2\x30\xeb\x30\xb2\x30\xF3\x30\x7E\xD8\x0F\xDC";

int main()
{
    int status = 0;
    
    // utf-16 -> utf-8 -> utf-16 -> utf-8
    //puts((char *)utf16_to_utf8(utf8_to_utf16(utf16_to_utf8((uint16_t *)test, &status), &status), &status));
    
    // utf-16 -> utf-8 -> utf-32
    /*
    uint32_t * s = utf8_to_utf32(utf16_to_utf8((uint16_t *)test, &status), &status);
    while(1)
    {
        uint32_t codepoint = s[0];
        if(codepoint == 0) break;
        uint8_t * bytes = (uint8_t*)&codepoint;
        fputc(bytes[0], stdout);
        fputc(bytes[1], stdout);
        fputc(bytes[2], stdout);
        fputc(bytes[3], stdout);
        s++;
    }
    */
    
    // utf-16 -> utf-8 -> utf-32 -> utf-8
    puts((char *)utf32_to_utf8(utf8_to_utf32(utf16_to_utf8((uint16_t *)test, &status), &status), &status));
}
