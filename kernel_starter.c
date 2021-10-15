#include "kernel_starter.h"
#include "bootloader.h"
#include "utils.h"
#include <efi.h>
#include <efilib.h>

typedef void __attribute__((sysv_abi)) (*kernel)(BootloaderData *);

EFI_STATUS
get_memory_map(UINTN *mmap_sz,
			   EFI_MEMORY_DESCRIPTOR **mmap,
			   UINTN *mmap_key,
			   UINTN *descriptor_sz) {
	EFI_STATUS status = EFI_SUCCESS;
	UINT32 descriptor_version;

	// Get memory map size
	status = BS->GetMemoryMap(mmap_sz, *mmap, NULL, descriptor_sz, &descriptor_version);
	if (status != EFI_BUFFER_TOO_SMALL) {
		print_efi_err(L"First GetMemoryMap() should return BUFFER_TO_SMALL", status);
		return EFI_ABORTED;
	}
	// Allocate memory for memory map and get it
	status = BS->AllocatePool(EfiLoaderData, *mmap_sz, (void **) mmap);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"Allocation for memory map failed", status);
		return status;
	}
	status = BS->GetMemoryMap(mmap_sz, *mmap, mmap_key, descriptor_sz, &descriptor_version);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"GetMemoryMap() failed", status);
	}

	return status;
}

MemoryData *get_memory_data(UINTN *mmap_key) {
	/*
		Get memory map and related info.
	*/
	EFI_STATUS status = EFI_SUCCESS;

	// Allocate memory_data struct
	MemoryData *memory_data = NULL;
	status = BS->AllocatePool(EfiLoaderData, sizeof(MemoryData), (void **) &memory_data);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"Allocating memory for memory data struct failed", status);
	}

	UINTN mmap_sz = 0;
	UINTN descriptor_sz = 0;
	EFI_MEMORY_DESCRIPTOR *mmap = NULL;
	if (get_memory_map(&mmap_sz, &mmap, mmap_key, &descriptor_sz) != EFI_SUCCESS) {
		return NULL;
	}

	memory_data->descriptor_sz = descriptor_sz;
	memory_data->map_sz = mmap_sz;
	memory_data->memory_map = mmap;
	memory_data->entries = mmap_sz / descriptor_sz;

	return memory_data;
}

void start_kernel(EFI_HANDLE image_handle,
				  Framebuffer *framebuffer,
				  Psf1_font *psf_font,
				  void *kernel_addr) {
	/*
		Should be last function in bootloader.
	*/
	UINTN mmap_key = 0;
	MemoryData *memory_data = get_memory_data(&mmap_key);
	if (memory_data == NULL) {
		print_err(L"Memory data gathering error");
		return;
	}

	// Exit boot services immediately after memory map gathering
	EFI_STATUS status = BS->ExitBootServices(image_handle, mmap_key);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"ExitBootServices() failed", status);
		return;
	}

	// !!!
	// NOTE: Now you are not able to call EFI functions!
	// !!!

	BootloaderData bootloader_data = {
		.framebuffer = framebuffer,
		.font = psf_font,
		.memory = memory_data,
	};

	// Jump to kernel function
	((kernel) kernel_addr)(&bootloader_data);
}