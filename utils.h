#pragma once
#include <efi.h>

CHAR16 *efi_status_str(EFI_STATUS status);
void print_err(const CHAR16 *str, EFI_STATUS status);

int memcmp(const void *aptr, const void *bptr, UINT64 n);