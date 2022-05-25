/*
ISC License

Copyright (c) 2019-2022 Hiltjo Posthuma <hiltjo@codemadness.org>
Copyright (c) 2021-2022 Sergey Zykov <sergey.zykov@cogentembedded.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 Taken from https://git.codemadness.org/json2tsv (commit 650481927d19d28d035d470bdedd2e2226e4dbd2)
 and introduced minor changes
*/

#ifndef TWEAKJSON_SUCKLESS_H_INCLUDED
#define TWEAKJSON_SUCKLESS_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* sergey.zykov@cogentembedded.com:  */
/*   defined GETNEXT()               */
#define GETNEXT() *json_snippet ? (int)(*json_snippet++) : -1

enum JSONType
{
    JSON_TYPE_ARRAY = 'a',
    JSON_TYPE_OBJECT = 'o',
    JSON_TYPE_STRING = 's',
    JSON_TYPE_BOOL = 'b',
    JSON_TYPE_NULL = '?',
    JSON_TYPE_NUMBER = 'n'
};

enum JSONError
{
    JSON_ERROR_MEM = -2,
    JSON_ERROR_INVALID = -1
};

#define JSON_MAX_NODE_DEPTH 64

struct json_node
{
    enum JSONType type;
    char *name;
    size_t namesiz;
    size_t index; /* count/index for array or object type */
    void* cookie; /* sergey.zykov@cogentembedded.com:                      */
                  /* introduced cookie pointer to track current DOM branch */
};

#ifndef GETNEXT
#define GETNEXT getchar_unlocked
#endif

#define END_MARKER -1 /* sergey.zykov@cogentembedded.com:                      */
                      /* renamed EOF to END_MARKER to avoid stdio.h collisions */

static int
codepointtoutf8(long r, char *s)
{
    if (r == 0)
    {
        return 0; /* NUL byte */
    }
    else if (r <= 0x7F)
    {
        /* 1 byte: 0aaaaaaa */
        s[0] = (char)r;
        return 1;
    }
    else if (r <= 0x07FF)
    {
        /* 2 bytes: 00000aaa aabbbbbb */
        s[0] = (char)(0xC0 | ((r & 0x0007C0) >> 6)); /* 110aaaaa */
        s[1] = (char)(0x80 | (r & 0x00003F));          /* 10bbbbbb */
        return 2;
    }
    else if (r <= 0xFFFF)
    {
        /* 3 bytes: aaaabbbb bbcccccc */
        s[0] = (char)(0xE0 | ((r & 0x00F000) >> 12)); /* 1110aaaa */
        s[1] = (char)(0x80 | ((r & 0x000FC0) >> 6));    /* 10bbbbbb */
        s[2] = (char)(0x80 | (r & 0x00003F));           /* 10cccccc */
        return 3;
    }
    else
    {
        /* 4 bytes: 000aaabb bbbbcccc ccdddddd */
        s[0] = (char)(0xF0 | ((r & 0x1C0000) >> 18)); /* 11110aaa */
        s[1] = (char)(0x80 | ((r & 0x03F000) >> 12));   /* 10bbbbbb */
        s[2] = (char)(0x80 | ((r & 0x000FC0) >> 6));    /* 10cccccc */
        s[3] = (char)(0x80 | (r & 0x00003F));             /* 10dddddd */
        return 4;
    }
}

static int
hexdigit(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    return 0;
}

static int
capacity(char **value, size_t *sz, size_t cur, size_t inc)
{
    size_t need, newsiz;
    char *newp;

    /* check for addition overflow */
    if (cur > SIZE_MAX - inc)
    {
        errno = ENOENT;
        return -1;
    }
    need = cur + inc;

    if (need > *sz)
    {
        if (need > SIZE_MAX / 2)
        {
            newsiz = SIZE_MAX;
        }
        else
        {
            for (newsiz = *sz < 64 ? 64 : *sz; newsiz <= need; newsiz *= 2)
                ;
        }
        newp = realloc(*value, newsiz);
        if (!newp)
            return -1; /* up to caller to free *value */
        *value = newp;
        *sz = newsiz;
    }
    return 0;
}

#define EXPECT_VALUE "{[\"-0123456789tfn"
#define EXPECT_STRING "\""
#define EXPECT_END "}],"
#define EXPECT_OBJECT_STRING EXPECT_STRING "}"
#define EXPECT_OBJECT_KEY ":"
#define EXPECT_ARRAY_VALUE EXPECT_VALUE "]"

#define JSON_INVALID()            \
    do                            \
    {                             \
        ret = JSON_ERROR_INVALID; \
        goto end;                 \
    } while (0);

/* sergey.zykov@cogentembedded.com:                      */
/* introduced json_snippet parameter                     */
/* original code relied on context free GETNEXT macro    */
static int parsejson(const char* json_snippet,
    void (*enter_cb)(struct json_node *, size_t, const char *, void*),
    void* cookie)
{
    struct json_node nodes[JSON_MAX_NODE_DEPTH] = {{(enum JSONType)0}};
    size_t depth = 0, p = 0, len, sz = 0;
    long cp, hi, lo;
    char pri[128], *str = NULL;
    int c, i, escape, iskey = 0, ret = JSON_ERROR_MEM;
    const char *expect = EXPECT_VALUE;

    if (capacity(&(nodes[0].name), &(nodes[0].namesiz), 0, 1) == -1)
        goto end;
    nodes[0].name[0] = '\0';

    while (1)
    {
        c = GETNEXT();
handlechr:
        if (c == END_MARKER)
            break;

        /* skip JSON white-space, (NOTE: no \v, \f, \b etc) */
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            continue;

        if (!c || !strchr(expect, c))
            JSON_INVALID();

        switch (c)
        {
        case ':':
            iskey = 0;
            expect = EXPECT_VALUE;
            break;
        case '"':
            nodes[depth].type = JSON_TYPE_STRING;
            escape = 0;
            len = 0;
            while (1)
            {
                c = GETNEXT();
chr:
                /* END_MARKER or control char: 0x7f is not defined as a control char in RFC8259 */
                if (c < 0x20)
                    JSON_INVALID();

                if (escape)
                {
escchr:
                    escape = 0;
                    switch (c)
                    {
                    case '"': /* FALLTHROUGH */
                    case '\\':
                    case '/':
                        break;
                    case 'b':
                        c = '\b';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'u': /* hex hex hex hex */
                        if (capacity(&str, &sz, len, 4) == -1)
                            goto end;
                        for (i = 12, cp = 0; i >= 0; i -= 4)
                        {
                            c = GETNEXT();
                            if (c == END_MARKER || !isxdigit(c))
                                JSON_INVALID(); /* invalid code point */
                            cp |= (hexdigit(c) << i);
                        }
                        /* RFC8259 - 7. Strings - surrogates.
						 * 0xd800 - 0xdbff - high surrogates */
                        if (cp >= 0xd800 && cp <= 0xdbff)
                        {
                            c = GETNEXT();
                            if (c != '\\')
                            {
                                len += codepointtoutf8(cp, &str[len]);
                                goto chr;
                            }
                            c = GETNEXT();
                            if (c != 'u')
                            {
                                len += codepointtoutf8(cp, &str[len]);
                                goto escchr;
                            }
                            for (hi = cp, i = 12, lo = 0; i >= 0; i -= 4)
                            {
                                c = GETNEXT();
                                if (c == END_MARKER || !isxdigit(c))
                                    JSON_INVALID(); /* invalid code point */
                                lo |= (hexdigit(c) << i);
                            }
                            /* 0xdc00 - 0xdfff - low surrogates */
                            if (lo >= 0xdc00 && lo <= 0xdfff)
                            {
                                cp = (hi << 10) + lo - 56613888; /* - offset */
                            }
                            else
                            {
                                /* handle graceful: raw invalid output bytes */
                                len += codepointtoutf8(hi, &str[len]);
                                if (capacity(&str, &sz, len, 4) == -1)
                                    goto end;
                                len += codepointtoutf8(lo, &str[len]);
                                continue;
                            }
                        }
                        len += codepointtoutf8(cp, &str[len]);
                        continue;
                    default:
                        JSON_INVALID(); /* invalid escape char */
                    }
                    if (capacity(&str, &sz, len, 1) == -1)
                        goto end;
                    str[len++] = (char)c;
                }
                else if (c == '\\')
                {
                    escape = 1;
                }
                else if (c == '"')
                {
                    if (capacity(&str, &sz, len, 1) == -1)
                        goto end;
                    str[len++] = '\0';

                    if (iskey)
                    {
                        /* copy string as key, including NUL byte */
                        if (capacity(&(nodes[depth].name), &(nodes[depth].namesiz), len, 1) == -1)
                            goto end;
                        memcpy(nodes[depth].name, str, len);
                    }
                    else
                    {
                        enter_cb(nodes, depth + 1, str, cookie);
                    }
                    break;
                }
                else
                {
                    if (capacity(&str, &sz, len, 1) == -1)
                        goto end;
                    str[len++] = (char)c;
                }
            }
            if (iskey)
                expect = EXPECT_OBJECT_KEY;
            else
                expect = EXPECT_END;
            break;
        case '[':
        case '{':
            if (depth + 1 >= JSON_MAX_NODE_DEPTH)
                JSON_INVALID(); /* too deep */

            nodes[depth].index = 0;
            if (c == '[')
            {
                nodes[depth].type = JSON_TYPE_ARRAY;
                expect = EXPECT_ARRAY_VALUE;
            }
            else if (c == '{')
            {
                iskey = 1;
                nodes[depth].type = JSON_TYPE_OBJECT;
                expect = EXPECT_OBJECT_STRING;
            }

            enter_cb(nodes, depth + 1, "", cookie);

            depth++;
            nodes[depth].index = 0;
            if (capacity(&(nodes[depth].name), &(nodes[depth].namesiz), 0, 1) == -1)
                goto end;
            nodes[depth].name[0] = '\0';
            break;
        case ']':
        case '}':
            if (!depth ||
                (c == ']' && nodes[depth - 1].type != JSON_TYPE_ARRAY) ||
                (c == '}' && nodes[depth - 1].type != JSON_TYPE_OBJECT))
                JSON_INVALID(); /* unbalanced nodes */

            nodes[--depth].index++;
            expect = EXPECT_END;
            break;
        case ',':
            if (!depth)
                JSON_INVALID(); /* unbalanced nodes */

            nodes[depth - 1].index++;
            if (nodes[depth - 1].type == JSON_TYPE_OBJECT)
            {
                iskey = 1;
                expect = EXPECT_STRING;
            }
            else
            {
                expect = EXPECT_VALUE;
            }
            break;
        case 't': /* true */
            /* sergey.zykov@cogentembedded.com:                                               */
            /* Original code used (GETNEXT() == 'r' && GETNEXT() == 'u' && GETNEXT() == 'e')  */
            /* However compiler optimized this into something non-working, probably because   */
            /* It could fully observe contents of GETNEXT() whilst original used IO operation */
            /* with a side effect.                                                            */
            c = GETNEXT();
            if (c == 'r') {
                c = GETNEXT();
                if (c == 'u') {
                    c = GETNEXT();
                    if (c == 'e') {
                        nodes[depth].type = JSON_TYPE_BOOL;
                        enter_cb(nodes, depth + 1, "true", cookie);
                        expect = EXPECT_END;
                    } else {
                        JSON_INVALID();
                    }
                } else {
                    JSON_INVALID();
                }
            } else {
                JSON_INVALID();
            }
            break;
        case 'f': /* false */
            /* sergey.zykov@cogentembedded.com:                                               */
            /* Original code used                                                             */
            /* (GETNEXT() == 'a' && GETNEXT() == 'l' && GETNEXT() == 's' && GETNEXT() == 'e') */
            /* However compiler optimized this into something non-working, probably because   */
            /* It could fully observe contents of GETNEXT() whilst original used IO operation */
            /* with a side effect.                                                            */
            c = GETNEXT();
            if (c == 'a') {
                c = GETNEXT();
                if (c == 'l') {
                    c = GETNEXT();
                    if (c == 's') {
                        c = GETNEXT();
                        if (c == 'e') {
                            nodes[depth].type = JSON_TYPE_BOOL;
                            enter_cb(nodes, depth + 1, "false", cookie);
                            expect = EXPECT_END;
                        } else {
                            JSON_INVALID();
                        }
                    } else {
                        JSON_INVALID();
                    }
                } else {
                    JSON_INVALID();
                }
            } else {
                JSON_INVALID();
            }
            break;
        case 'n': /* null */
            /* sergey.zykov@cogentembedded.com:                                               */
            /* Original code used                                                             */
            /* (GETNEXT() == 'u' && GETNEXT() == 'l' && GETNEXT() == 'l')                     */
            /* However compiler optimized this into something non-working, probably because   */
            /* It could fully observe contents of GETNEXT() whilst original used IO operation */
            /* with a side effect.                                                            */
            c = GETNEXT();
            if (c == 'u') {
                c = GETNEXT();
                if (c == 'l') {
                    c = GETNEXT();
                    if (c == 'l') {
                        nodes[depth].type = JSON_TYPE_NULL;
                        enter_cb(nodes, depth + 1, "null", cookie);
                        expect = EXPECT_END;
                    } else {
                        JSON_INVALID();
                    }
                } else {
                    JSON_INVALID();
                }
            } else {
                JSON_INVALID();
            }
            break;
        default: /* number */
            nodes[depth].type = JSON_TYPE_NUMBER;
            p = 0;
            pri[p++] = (char)c;
            expect = EXPECT_END;
            while (1)
            {
                c = GETNEXT();
                if (c == END_MARKER ||
                    !c || !strchr("0123456789eE+-.", c) ||
                    p + 1 >= sizeof(pri))
                {
                    pri[p] = '\0';
                    enter_cb(nodes, depth + 1, pri, cookie);
                    goto handlechr; /* do not read next char, handle this */
                }
                else
                {
                    pri[p++] = (char)c;
                }
            }
        }
    }
    if (depth)
        JSON_INVALID(); /* unbalanced nodes */

    ret = 0; /* success */
end:
    for (depth = 0; depth < sizeof(nodes) / sizeof(nodes[0]); depth++)
        free(nodes[depth].name);
    free(str);

    return ret;
}

#endif
