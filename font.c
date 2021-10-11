#include "font.h"
#include "file.h"
#include <efi.h>
#include <efilib.h>
#include <elf.h>

PsfFont *load_psf_font(EFI_FILE *directory,
					 CHAR16 *path,
					 EFI_HANDLE image_handle,
					 EFI_SYSTEM_TABLE *system_table) {

	EFI_FILE *psf_file = load_file(directory, path, image_handle, system_table);
	if (psf_file == NULL) {
		Print(L"[!] Error: Couldn't find file with psf font \n\r");
		return NULL;
	}

	// Get psf file's header
	PsfHeader *psf_header;
	system_table->BootServices->AllocatePool(
		EfiLoaderData, sizeof(PsfHeader), (void **) &psf_header);
	UINTN psf_header_sz = sizeof(PsfHeader);
	psf_file->Read(psf_file, &psf_header_sz, psf_header);

    // Verify .psf format header
	if (psf_header->magic == PSF_HEADER_MAGIC) {
		Print(L"[!] Error: Invalid .psf file header \n\r");
		return NULL;
	}

	// Calculate buffer size of all glyphs
	UINTN glyph_buffer_sz;
	if (psf_header->mode == 1) {
		glyph_buffer_sz = psf_header->charsize * 512;
	} else {
		glyph_buffer_sz = psf_header->charsize * 256;
	}

	// Allocate memory and load entire glyph buffer of psf file
	void *glyph_buffer;
	psf_file->SetPosition(psf_file, sizeof(psf_header));
	system_table->BootServices->AllocatePool(
		EfiLoaderData, glyph_buffer_sz, (void **) &glyph_buffer);
	psf_file->Read(psf_file, &glyph_buffer_sz, glyph_buffer);

	// Allocate and construct finished font struct
	PsfFont *psf_font;
	system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(PsfFont), (void **) &psf_font);
	psf_font->psf_header = psf_header;
	psf_font->glyph_buffer = glyph_buffer;
	psf_font->glyph_height = GLYPH_HEIGHT;
	psf_font->glyph_width = GLYPH_WIDTH;
	return psf_font;
}