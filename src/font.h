#ifndef FLAPPY_FONT_H_INCLUDED
#define FLAPPY_FONT_H_INCLUDED

long font_size(const char* str);
long font_vertices(const char* str);
void font_print(const char* str, float* buffer, long size);

#endif
