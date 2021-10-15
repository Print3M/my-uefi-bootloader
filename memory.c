#include "memory.h"
#include <efi.h>

MemoryData *
get_memory_data(EFI_SYSTEM_TABLE *system_table, MemoryData *memory_data, UINTN *mmap_key) {
	/*
		Get memory map and other info.
	*/
	EFI_STATUS status = 0;
	EFI_MEMORY_DESCRIPTOR *mmap = NULL;
	UINTN mmap_sz, descriptor_sz = 0;
	UINT32 descriptor_version = 0;

	// Get required size
	system_table->BootServices->GetMemoryMap(
		&mmap_sz, mmap, mmap_key, &descriptor_sz, &descriptor_version);
	// Allocate memory map and recive it
	system_table->BootServices->AllocatePool(EfiLoaderData, mmap_sz, (void **) &mmap);
	status = system_table->BootServices->GetMemoryMap(
		&mmap_sz, mmap, mmap_key, &descriptor_sz, &descriptor_version);
	if (status != EFI_SUCCESS) {
		return NULL;
	}

	// Set memory data
	memory_data->descriptor_sz = descriptor_sz;
	memory_data->map_sz = mmap_sz;
	memory_data->memory_map = mmap;
	memory_data->entries = mmap_sz / descriptor_sz;

	return memory_data;
}