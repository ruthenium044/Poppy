#ifndef SHADER_H
#define SHADER_H

#include <stddef.h>

extern "C" {
    enum xxx_shader_type
	{
		XXX_SHADER_TYPE_VERTEX = 0,
		XXX_SHADER_TYPE_FRAGMENT = 1,
	};

    struct xxx_bufferApi
	{
		void(*destroy)(struct xxx_bufferApi*);
	};

	struct xxx_shaderApi
	{
		struct xxx_shader* (*create)(xxx_shader_type type, char* source, size_t size);
		void(*destroy)(struct xxx_shader*); //in this one delete program
	};

	struct xxx_programApi
	{
		struct xxx_program* (*create)(void);
		void (*destroy)(struct xxx_program* program);
	};

	struct xxx_api
	{
		struct xxx_renderer* (*create)(void);
		void(*destroy)(struct xxx_renderer*);
		struct xxx_shaderApi* shader;
		struct xxx_programApi* program;
	};

	struct xxx_api* xxx_load();
}

#endif