/*
 * ANSI to Unicode keyboard mapping
 * Maps VT and ANSI keyboard sequences to unicode private use range.
 *
 * July 2022 Greg Haerr
 */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#if !__ELKS__
#include <poll.h>
#endif

#include "unikey.h"

#define ANSI_UTF8       0       /* =1 to decode UTF-9 in readansi() */
#define CHECK_MOUSE     1       /* =1 to validate ANSI mouse sequences */
#define DEBUG           1       /* =1 for keyname() */
#define ESC             27

static char scroll_reverse = 0; /* report reversed scroll wheel direction */
int kDoubleClickTime = 200;

static int small_atoi(const char *s)
{
    int n = 0;

    while (*s == ' ' || *s == '\t')
        s++;
    while ((unsigned) (*s - '0') <= 9u)
        n = n * 10 + *s++ - '0';
    return n;
}

/*
 * Check and convert from single ANSI or UTF-8 keyboard sequence
 * to unicode key or char value, including single byte ASCII values
 * (0x00 <= ASCII <= 0x7f).
 * Will also decode UTF-8 into single UCS-2 character.
 * Returns -1 if not ASCII or ANSI keyboard sequence (e.g. mouse or DSR).
 */
int ansi_to_unikey(char *buf, int n)
{
    if (n == 0)
        return -1;              /* invalid conversion */
    if (n == 1) {
        int c = buf[0] & 255;       /* ASCII range */
        return (c == 127)? 8: c;    /* map DEL -> BS */
    }
    if (buf[0] == ESC) {
        if (buf[1] == '[') {
            if (n == 3) {                           /* xterm sequences */
                switch (buf[2]) {                   /* ESC [ A etc */
                case 'A':   return kUpArrow;
                case 'B':   return kDownArrow;
                case 'C':   return kRightArrow;
                case 'D':   return kLeftArrow;
                case 'F':   return kEnd;
                case 'H':   return kHome;
                }
            } else if (n == 4 && buf[2] == '1') {   /* ESC [ 1 P etc */
                switch (buf[3]) {
                case 'P':   return kF1;
                case 'Q':   return kF2;
                case 'R':   return kF3;
                case 'S':   return kF4;
                }
            }
            if (n > 3 && buf[n-1] == '~') {         /* vt sequences */
                switch (small_atoi(buf+2)) {
                case 1:     return kHome;
                case 2:     return kInsert;
                case 3:     return kDelete;
                case 4:     return kEnd;
                case 5:     return kPageUp;
                case 6:     return kPageDown;
                case 7:     return kHome;
                case 8:     return kEnd;
                case 11:    return kF1;
                case 12:    return kF2;
                case 13:    return kF3;
                case 14:    return kF4;
                case 15:    return kF5;
                case 17:    return kF6;
                case 18:    return kF7;
                case 19:    return kF8;
                case 20:    return kF9;
                case 21:    return kF10;
                case 23:    return kF11;
                case 24:    return kF12;
                }
            }
        }

        /* allow multi-keystroke sequences using ESC + character -> Alt/Fn key */
        if (n == 2) {
            if (buf[1] >= 'a' && buf[1] <= 'z')     /* ESC {a-z} -> ALT-{A-Z} */
                return buf[1] - 'a' + kAltA;

            if (buf[1] >= '1' && buf[1] <= '9')     /* ESC {1-9} -> Fn{1-9} */
                return buf[1] - '1' + kF1;
            if (buf[1] == '0')
                return kF10;
        }
    }
    return -1;
}

/**
 * Reads single keystroke or control sequence from character device.
 *
 * When reading ANSI UTF-8 text streams, characters and control codes
 * are oftentimes encoded as multi-byte sequences. This function knows
 * how long each sequence is, so that each read consumes a single thing
 * from the underlying file descriptor, e.g.
 *
 *     "a"               ALFA
 *     "\316\261"        ALPHA
 *     "\e[38;5;202m"    ORANGERED
 *     "\e[A"            UP
 *     "\ea"             ALT-A
 *     "\e\e"            ESC ESC
 *     "\001"            CTRL-ALFA
 *     "\e\001"          ESC CTRL-ALFA
 *     "\eOP"            PF1
 *     "\000"            NUL
 *     "\e]ls -lR /\e\\" OSC
 *     "\302\233A"       UP
 *     "\300\200"        NUL
 *
 * This routine generalizes to ascii, utf-8, chorded modifier keys,
 * function keys, color codes, c0/c1 control codes, cursor movement,
 * mouse movement, etc.
 *
 * Userspace buffering isn't required, since ANSI escape sequences and
 * UTF-8 are decoded without peeking. Noncanonical overlong encodings
 * can cause the stream to go out of sync. This function recovers such
 * events by ignoring continuation bytes at the beginning of each read.
 *
 * @param p is guaranteed to receive a NUL terminator if n>0
 * @return number of bytes read (helps differentiate "\0" vs. "")
 * @see ttyinfo.c
 * @see ANSI X3.64-1979
 * @see ISO/IEC 6429
 * @see FIPS-86
 * @see ECMA-48
 */

int readansi(int fd, char *buf, int size) {
  unsigned char c;
  int rc, i, j;
  enum { kAscii, kUtf8, kEsc, kCsi, kSs } t;
  if (size) buf[0] = 0;
  for (j = i = 0, t = kAscii;;) {
    if (i + 2 >= size) {
      errno = ENOMEM;
      return -1;
    }
    if ((rc = read(fd, &c, 1)) != 1) {
      if (rc == -1 && errno == EINTR && i) {
        continue;
      }
      if (rc == -1 && errno == EAGAIN) {    /* Linux may return EAGAIN on fn key seq */
#if !__ELKS__
        struct pollfd pfd[1];
        pfd[0].fd = fd;
        pfd[0].events = POLLIN;
        poll(pfd, 1, 0);
#endif
        continue;
      }
      return rc;
    }
    buf[i++] = c;
    buf[i] = 0;
    switch (t) {
      case kAscii:
        if (c < 0200) {
          if (c == 033) {
            t = kEsc;
          } else {
            return i;
          }
        } else if (c >= 0300) {
          t = kUtf8;
          j = ThomPikeLen(c) - 1;
        }
        break;
      case kUtf8:
        if (!--j) return i;
        break;
      case kEsc:
        switch (c) {
          case '[':
            t = kCsi;
            break;
          case 'N':
          case 'O':
            t = kSs;
            break;
          case 0x20:
          case 0x21:
          case 0x22:
          case 0x23:
          case 0x24:
          case 0x25:
          case 0x26:
          case 0x27:
          case 0x28:
          case 0x29:
          case 0x2A:
          case 0x2B:
          case 0x2C:
          case 0x2D:
          case 0x2E:
          case 0x2F:
            break;
          default:
            return i;
        }
        break;
      case kCsi:
        switch (c) {
          case ':':
          case ';':
          case '<':
          case '=':
          case '>':
          case '?':
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            break;
          default:
            return i;
        }
        break;
      case kSs:
        return i;
      default:
        __builtin_unreachable();
    }
  }
}

static int startswith(const char *s, const char *prefix) {
  for (;;) {
    if (!*prefix) return 1;
    if (!*s) return 0;
    if (*s++ != *prefix++) return 0;
  }
}

static int getparm(char *buf, int n)
{
    while (n-- > 0) {
        while (*buf != ';' && *buf)
                buf++;
        if (*buf)
                buf++;
    }
    return small_atoi(buf);
}

/*
 * Check and respond to ANSI DSR (device status report).
 * Format: ESC [ rows; cols R
 * Returns -1 if not ANSI DSR.
 */
int ansi_dsr(char *buf, int n, int *cols, int *rows)
{
    char *p;
    int r, c;

    if (n < 6 || !startswith(buf, "\033[") || buf[n-1] != 'R')
        return -1;
    p = buf + 2;
    r = getparm(p, 0);
    c = getparm(p, 1);
    //printf("DSR terminal size is %dx%d\r\n", r, c);
    if (r < 10 || r > 100 || c < 10 || c > 250)
        return -1;
    *rows = r;
    *cols = c;
    return 1;
}

#define WRITE(FD, SLIT)             write(FD, SLIT, strlen(SLIT))
#define PROBE_DISPLAY_SIZE          "\0337\033[9979;9979H\033[6n\0338"

/* probe display size - only uses DSR for now, for ELKS/UNIX compatibility */
int tty_getsize(int *cols, int *rows)
{
    int n, x, y;
    char buf[32];

    WRITE(1, PROBE_DISPLAY_SIZE);
    if ((n = readansi(0, buf, sizeof(buf))) > 0) {
        if (ansi_dsr(buf, n, &x, &y) > 0) {
            *cols = x;
            *rows = y;
            return 1;
        }
    }
    return -1;
}
