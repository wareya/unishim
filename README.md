# unishim
unishim.h - C90 utf-8/utf-16/utf-32 conversion header

# summary

    uint8_t * utf16_to_utf8(uint16_t * utf16, int * status)
    uint16_t * utf8_to_utf16(uint8_t * utf8, int * status)
    uint8_t * utf32_to_utf8(uint32_t * utf32, int * status)
    uint32_t * utf8_to_utf32(uint8_t * utf8, int * status)

    // status 0 is success, other indicates failure
    // null returned if failure
    // returned string is freshly allocated from malloc()
    // Input string must NOT be modified by another thread while these functions are running.

# explanation

    uint8_t * utf16_to_utf8(uint16_t * utf16, int * status)

allocates and returns a utf-8 string converted from the utf-16 one

assumes that the utf-16 string is in native endian

if an error is encountered, sets status and returns 0:

```
0 - no error
1 - second surrogate where simple codepoint or first surrogate expected
2 - null where second surrogate expected
3 - first surrogate or simple codepoint where second surrogate expected
4 - failed to allocate return buffer
```

```
uint16_t * utf8_to_utf16(uint8_t * utf8, int * status)
```

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

    uint8_t * utf32_to_utf8(uint32_t * utf32, int * status)

allocates and returns a utf-8 string converted from the utf-32 one

assumes that the utf-32 string is in native endian

if an error is encountered, sets status and returns 0:

```
0 - no error
1 - codepoint is a utf-16 surrogate
2 - codepoint is too large to store in utf-16, which is forbidden by modern utf-8
3 - failed to allocate return buffer
```

```
uint32_t * utf8_to_utf32(uint8_t * utf8, int * status)
```

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

