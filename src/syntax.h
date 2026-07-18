#ifndef SYNTAX_H
#define SYNTAX_H

#include "theme.h"

void syntax_highlight_c(const char* line, const Theme* theme);
void syntax_highlight_python(const char* line, const Theme* theme);
void syntax_highlight_javascript(const char* line, const Theme* theme);

#endif