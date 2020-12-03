#ifndef FLAPPY_MODEL_H_INCLUDED
#define FLAPPY_MODEL_H_INCLUDED

enum model_format {
    MODEL_FORMAT_UNDEFINED = 0,
    MODEL_FORMAT_V3F,
    MODEL_FORMAT_T2F_V3F,
    MODEL_FORMAT_N3F_V3F,
    MODEL_FORMAT_T2F_N3F_V3F,
};

unsigned int model_buffer_create(int format, long count, const float* vertices);
unsigned int model_buffer_config(int format, int buffer);

#endif
