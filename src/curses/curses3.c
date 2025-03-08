/*
 * curses library workaround - Part III
 *
 * routines mostly used for ttyclock port
 */

#include "curses.h"
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>


/* just to fill the gap for ELKS */
int strnicmp(const char *s1, const char *s2, int n) {
    for (size_t i = 0; i < n; i++) {
        // Converte ambos os caracteres para minúsculas antes de comparar
        char c1 = tolower((unsigned char)s1[i]);
        char c2 = tolower((unsigned char)s2[i]);

        // Se os caracteres forem diferentes, retorna a diferença
        if (c1 != c2) {
            return (int)(c1 - c2);
        }

        // Se chegarmos ao final de uma das strings, retorna a diferença
        if (s1[i] == '\0' || s2[i] == '\0') {
            break;
        }
    }

    // Se chegamos aqui, as strings são iguais até o limite de 'n' caracteres
    return 0;
}

void clrscr()
{
    printf("\033[H\033[J");
}

// Função attrset() simplificada
void attrset(int attr) {
    printf("\033[");

	switch(attr)
	{
	case A_BOLD:
		printf("1m");
		break;
	case A_UNDERLINE:
		printf("4m");
		break;
#if 0
	case A_BLINK:
		printf("5m");
		break
#endif
	case A_REVERSE:
		printf("7m");
		break;
	default:;
	}

}

// Função attrset() simplificada
void wattrset(WINDOW *w, int attr) {
	attrset(attr);
}


void waddstr(WINDOW *w, char *str)
{
    printw(str);
}

void addstr(char *str)
{
    printw(str);
}



void mvwaddch(WINDOW *w, int y, int x, int ch)
{
    mvaddch(y, x, ch);
}

void mvwaddstr(WINDOW *w, int y, int x, char *str)
{
    move(y, x);
    printw(str);
}

void mvwprintw(WINDOW *w, int y, int x, char *fmt, ...)
{
    va_list ptr;

    move(y, x);
    va_start(ptr, fmt);
    vfprintf(stdout,fmt,ptr);
    va_end(ptr);
}

void mvwin(WINDOW *w, int y, int x)
{
    //yoff = y;
    //xoff = x;
}

void wattron(WINDOW *w, int a)
{
    if (a & (A_BLINK|A_BOLD)) attroff(-1);
    attron(a);
}

void wattroff(WINDOW *w, int a)
{
    if (a & (A_BLINK|A_BOLD)) attroff(-1);
    attroff(a);
}

void wbkgdset(WINDOW *w, int a)
{
    printf("\033[7m");
    attron(a);
}

void wrefresh(WINDOW *w)
{
    printf("\033[m");
}

void insertln() {
    printf("\033[s\033[1G\033[L\033[u");
}

void winsertln(WINDOW *w) {
    insertln();
}

void standout() {
	printf("\033[7m");
}
void standend(){
    printf("\033[0m");
}
