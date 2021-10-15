#include "memory.h"
#include "utils.h"
#include <efi.h>
#include <efilib.h>

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
	Print(L"2-Mmap_key=%d \n\r", *mmap_key);
	return status;
}

MemoryData *get_memory_data(UINTN *mmap_key, EFI_HANDLE hnd) {
	/*
		Get memory map and related info.
	*/
	// Allocate memory_data struct
	MemoryData *memory_data = NULL;
	EFI_STATUS status = BS->AllocatePool(EfiLoaderData, sizeof(MemoryData), (void **) &memory_data);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"Allocating memory for memory data struct failed", status);
	}

	UINTN mmap_sz = 0;
	UINTN descriptor_sz = 0;
	EFI_MEMORY_DESCRIPTOR *mmap = NULL;
	if (get_memory_map(&mmap_sz, &mmap, mmap_key, &descriptor_sz) != EFI_SUCCESS) {
		return NULL;
	}

	status = BS->ExitBootServices(hnd, mmap_key);

	memory_data->descriptor_sz = descriptor_sz;
	memory_data->map_sz = mmap_sz;
	memory_data->memory_map = mmap;
	memory_data->entries = mmap_sz / descriptor_sz;

	return memory_data;
}