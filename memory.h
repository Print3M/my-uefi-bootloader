#pragma once
#include <efi.h>

typedef struct {
	EFI_MEMORY_DESCRIPTOR *memory_map;
	UINTN map_sz;
	UINTN descriptor_sz;
	UINT64 entries;
} MemoryData;

MemoryData *
get_memory_data(EFI_SYSTEM_TABLE *system_table, MemoryData *memory_data, UINTN *mmap_key);