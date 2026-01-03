#ifndef SHADER_H
#define SHADER_H

#include <stddef.h>

struct ppy_renderer;
struct SDL_Window;

enum ppy_shader_type
{
    ppy_SHADER_TYPE_VERTEX = 0,
    ppy_SHADER_TYPE_FRAGMENT = 1,
};

struct ppy_bufferApi
{
    void (*destroy)(struct ppy_bufferApi *);
};

struct ppy_shaderApi
{
    struct ppy_shader *(*create)(ppy_shader_type type, char *source, size_t size);
    void (*destroy)(struct ppy_shader *); // in this one delete program
};

struct ppy_programApi
{
    struct ppy_program *(*create)(void);
    void (*destroy)(struct ppy_program *program);
};

struct ppy_api
{
    ppy_renderer *(*create)(SDL_Window *window);
    void (*draw)(ppy_renderer *);
    void (*destroy)(ppy_renderer *);
    // might need in the future if creating stuff from outside
    // struct ppy_shaderApi* shader;
    // struct ppy_programApi* program;
};

struct ppy_api *ppy_get();

#endif