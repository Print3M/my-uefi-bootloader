#pragma once
#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_FILE *load_file(EFI_FILE *directory,
				   CHAR16 *file_name,
				   EFI_HANDLE image_handle,
				   EFI_SYSTEM_TABLE *system_table);
