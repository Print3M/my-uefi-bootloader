#include "bootloader.h"
#include "file.h"
#include "font.h"
#include "gop.h"
#include "kernel_loader.h"
#include "memory.h"
#include "utils.h"
#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
	InitializeLib(image_handle, system_table);

	EFI_FILE *kernel_file = load_file(NULL, L"kernel.elf", image_handle, system_table);
	if (kernel_file == NULL) {
		Print(L"[!] Error: Loading kernel file error \n\r");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Kernel file has been loaded successfully \n\r");

	void *kernel_addr = load_kernel_into_memory(image_handle, system_table, kernel_file);
	if (kernel_addr == NULL) {
		Print(L"[!] Error: Loading kernel into memory error \n\r");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Kernel has been successfully loaded into memory \n\r");

	PsfFont *psf_font = load_psf_font(NULL, L"zap-light16.psf", image_handle, system_table);
	if (psf_font == NULL) {
		Print(L"[!] Error: Psf font loading error \n\r");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Psf font loaded successfully \n\r");

	Framebuffer framebuffer;
	if (initialize_gop(&framebuffer) == NULL) {
		Print(L"[!] Error: GOP initialization error \n\r");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] GOP initialized successfully \n\r");

	MemoryData memory_data;
	UINTN mmap_key;
	if (get_memory_data(system_table, &memory_data, &mmap_key) == NULL) {
		Print(L"[!] Error: Memory map gathering error \n\r");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Memory map gathered successfully \n\r");

	// Leave UEFI features and jump to kernel
	if (system_table->BootServices->ExitBootServices(image_handle, mmap_key) != EFI_SUCCESS) {
		Print(L"[!] Error: BootServices->ExitBootServices() error");
	}

	BootloaderData bootloader_data = {
		.framebuffer = &framebuffer,
		.font = psf_font,
		.memory = &memory_data,
	};

	// TODO: better way and understand (can we leave that e_header.e_entry? Maybe kernel_addr)
	void (*kernel_main)(BootloaderData *) =
		((__attribute__((sysv_abi)) void (*)(BootloaderData *)) kernel_addr);

	Print(L"[*] Kernel is starting...");
	kernel_main(&bootloader_data);

	return EFI_SUCCESS;
}
