/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 *
 * This is a simplified version without locale-aware collation support.
 */

#include "fnmatch.h"
#include <ctype.h>
#include <string.h>

#define EOS '\0'

#define RANGE_MATCH 1
#define RANGE_NOMATCH 0
#define RANGE_ERROR (-1)

static int rangematch(const char *, char, const char *, int, char **, char **);
static int fnmatch1(const char *, const char *, const char *, int);

int fnmatch(const char *pattern, const char *string, int flags) {
    return (fnmatch1(pattern, string, string, flags));
}

static int fnmatch1(const char *pattern, const char *string, const char *stringstart, int flags) {
    const char *bt_pattern, *bt_string;
    char *newp, *news;
    char c;
    char pc, sc;

    bt_pattern = bt_string = NULL;
    for (;;) {
        pc = *pattern++;
        sc = *string;

        switch (pc) {
        case EOS:
            if ((flags & FNM_LEADING_DIR) && sc == '/') {
                return (0);
            }
            if (sc == EOS) {
                return (0);
            }
            goto backtrack;
        case '?':
            if (sc == EOS) {
                return (FNM_NOMATCH);
            }
            if (sc == '/' && (flags & FNM_PATHNAME)) {
                goto backtrack;
            }
            if (sc == '.' && (flags & FNM_PERIOD) && (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/'))) {
                goto backtrack;
            }
            ++string;
            break;
        case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            while (c == '*') {
                c = *++pattern;
            }

            if (sc == '.' && (flags & FNM_PERIOD) && (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/'))) {
                goto backtrack;
            }

            /* Optimize for pattern with * at end or before /. */
            if (c == EOS) {
                if (flags & FNM_PATHNAME) {
                    return ((flags & FNM_LEADING_DIR) || strchr(string, '/') == NULL ? 0 : FNM_NOMATCH);
                }
                else {
                    return (0);
                }
            }
            else if (c == '/' && flags & FNM_PATHNAME) {
                if ((string = strchr(string, '/')) == NULL) {
                    return (FNM_NOMATCH);
                }
                break;
            }

            /*
             * First try the shortest match for the '*' that
             * could work. We can forget any earlier '*' since
             * there is no way having it match more characters
             * can help us, given that we are already here.
             */
            bt_pattern = pattern;
            bt_string = string;
            break;
        case '[':
            if (sc == EOS) {
                return (FNM_NOMATCH);
            }
            if (sc == '/' && (flags & FNM_PATHNAME)) {
                goto backtrack;
            }
            if (sc == '.' && (flags & FNM_PERIOD) && (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/'))) {
                goto backtrack;
            }

            switch (rangematch(pattern, sc, string, flags, &newp, &news)) {
            case RANGE_ERROR:
                goto norm;
            case RANGE_MATCH:
                pattern = newp;
                string = news;
                break;
            case RANGE_NOMATCH:
                goto backtrack;
            }
            break;
        case '\\':
            if (!(flags & FNM_NOESCAPE)) {
                pc = *pattern++;
                if (pc == EOS) {
                    return (FNM_NOMATCH);
                }
            }
            /* FALLTHROUGH */
        default:
        norm:
            if ((flags & FNM_CASEFOLD) ? (tolower((unsigned char)pc) == tolower((unsigned char)sc)) : (pc == sc))
                ;
            else {
                goto backtrack;
            }
            ++string;
            break;
        }
    }
backtrack:
    if (bt_pattern == NULL) {
        return (FNM_NOMATCH);
    }

    /* Restart the pattern. */
    string = bt_string;
    if (*string == EOS) {
        return (FNM_NOMATCH);
    }

    /* Special case: append * to pattern. */
    if (*bt_pattern == EOS) {
        if (bt_string[0] == '/' && (flags & FNM_PATHNAME)) {
            return (FNM_NOMATCH);
        }
        ++bt_string;
        goto backtrack;
    }

    pattern = bt_pattern;
    if (*bt_string == '/' && (flags & FNM_PATHNAME)) {
        return (FNM_NOMATCH);
    }
    ++bt_string;
    goto backtrack;
}

static int rangematch(const char *pattern, char test, const char *string, int flags, char **newp, char **news) {
    int negate, ok;
    char c, c2;

    /*
     * A bracket expression starting with an unquoted circumflex
     * character produces unspecified results (IEEE 1003.2-1992,
     * 3.13.2).  This implementation treats it like '!', for
     * consistency with the regular expression syntax.
     * J.T. Conklin (conklin@ngai.kaleida.com)
     */
    if ((negate = (*pattern == '!' || *pattern == '^'))) {
        ++pattern;
    }

    if (flags & FNM_CASEFOLD) {
        test = tolower((unsigned char)test);
    }

    /*
     * A right bracket shall lose its special meaning and represent
     * itself in a bracket expression if it occurs first in the list.
     * -- POSIX.2 2.8.3.2
     */
    ok = 0;
    for (;;) {
        c = *pattern++;
        if (c == ']') {
            if (ok) {
                break;
            }
            if (c == test) {
                ok = 1;
            }
        }
        else if (c == '-')
            ;
        else if (c == EOS) {
            return (RANGE_ERROR);
        }
        else {
            if (c == '\\' && !(flags & FNM_NOESCAPE)) {
                c = *pattern++;
            }
            if (c == EOS) {
                return (RANGE_ERROR);
            }

            if (flags & FNM_CASEFOLD) {
                c = tolower((unsigned char)c);
            }

            if (*pattern == '-' && *(pattern + 1) != ']') {
                c2 = *(pattern + 2);
                if (c2 == EOS) {
                    return (RANGE_ERROR);
                }
                if (c2 == '\\' && !(flags & FNM_NOESCAPE)) {
                    c2 = *(pattern + 3);
                }
                if (c2 == EOS) {
                    return (RANGE_ERROR);
                }

                if (flags & FNM_CASEFOLD) {
                    c2 = tolower((unsigned char)c2);
                }

                if (c <= test && test <= c2) {
                    ok = 1;
                }
                pattern += 2;
            }
            else if (c == test) {
                ok = 1;
            }
        }
    }
    *newp = (char *)pattern;
    *news = (char *)string + 1;
    return (ok == negate ? RANGE_NOMATCH : RANGE_MATCH);
}
