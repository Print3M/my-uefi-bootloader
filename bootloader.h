#pragma once
#include "font.h"
#include "gop.h"
#include "memory.h"

typedef struct {
	Framebuffer *framebuffer;
	Psf1_font *font;
	MemoryData *memory;
} BootloaderData;