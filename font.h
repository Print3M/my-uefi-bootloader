#pragma once
#include "file.h"
#include <efi.h>

#define GLYPH_WIDTH 8
#define GLYPH_HEIGHT 16
#define PSF_HEADER_MAGIC 0x3604

typedef struct {
	UINT16 magic;
	UINT8 mode;
	UINT8 charsize;
} PsfHeader;

typedef struct {
	PsfHeader *psf_header;
	UINT8 glyph_height;
	UINT8 glyph_width;
	void *glyph_buffer;
} PsfFont;

PsfFont *load_psf_font(EFI_FILE *directory,
					   CHAR16 *path,
					   EFI_HANDLE image_handle,
					   EFI_SYSTEM_TABLE *system_table);