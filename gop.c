#include "gop.h"
#include <efi.h>

Framebuffer* initialize_gop(Framebuffer *framebuffer) {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
	EFI_STATUS status = NULL;

	status = uefi_call_wrapper(
		BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void **) &gop);
	if (status) {
		Print(L"[!] Error: Unable to locate GOP \n\r");
		return NULL;
	} 
	Print(L"[+] GOP located sucessfully \n\r");

	framebuffer->base_address = (void *) gop->Mode->FrameBufferBase;
	framebuffer->buffer_size = gop->Mode->FrameBufferSize;
	framebuffer->width = gop->Mode->Info->HorizontalResolution;
	framebuffer->height = gop->Mode->Info->VerticalResolution;
	framebuffer->pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;
	framebuffer->bytes_per_pixel = BYTES_PER_PIXEL;

	return framebuffer;
}
