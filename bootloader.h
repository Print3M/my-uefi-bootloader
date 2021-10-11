#pragma once
#include "font.h"
#include "gop.h"
#include "memory.h"

typedef struct {
	Framebuffer *framebuffer;
	PsfFont *font;
	MemoryData *memory;
} BootloaderData;