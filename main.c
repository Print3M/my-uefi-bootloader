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

	Print(L"[*] Starting bootloader... \n\r");

	EFI_FILE *root_dir = NULL;
	EFI_FILE *kernel_file = load_file(root_dir, L"kernel.elf", image_handle);
	if (kernel_file == NULL) {
		print_err(L"Loading kernel file error");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Kernel file has been loaded successfully \n\r");

	void *kernel_addr = load_kernel_into_memory(kernel_file);
	if (kernel_addr == NULL) {
		print_err(L"Loading kernel into memory error");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Kernel has been successfully loaded into memory \n\r");

	Psf1_font *psf_font = load_psf_font(root_dir, L"zap-light16.psf", image_handle);
	if (psf_font == NULL) {
		print_err(L"Psf font loading error");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Psf font loaded successfully \n\r");

	Framebuffer *framebuffer = get_framebuffer();
	if (framebuffer == NULL) {
		print_err(L"Framebuffer initialization error");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Framebuffer initialized successfully \n\r");

	UINTN mmap_key = 0;
	MemoryData *memory_data = get_memory_data(&mmap_key, image_handle);
	if (memory_data == NULL) {
		print_err(L"Memory data gathering error");
		return EFI_LOAD_ERROR;
	}
	Print(L"[+] Memory map gathered successfully \n\r");

	Print(L"3-Mmap_key=%d \n\r", mmap_key);

	// Leave EFI features and jump to kernel
	EFI_STATUS status = BS->ExitBootServices(image_handle, mmap_key);
	if (status != EFI_SUCCESS) {
		print_efi_err(L"ExitBootServices() failed", status);
		return EFI_LOAD_ERROR;
	}

	BootloaderData bootloader_data = {
		.framebuffer = framebuffer,
		.font = psf_font,
		.memory = memory_data,
	};

	// TODO: kernel info:
	// In addition, the UEFI OS loader can treat all memory in the map marked as EfiBootServicesCode and EfiBootServicesData as available free memory. 

	// TODO: better way and understand (can we leave that e_header.e_entry? Maybe kernel_addr)
	void (*kernel_main)(BootloaderData *) =
		((__attribute__((sysv_abi)) void (*)(BootloaderData *)) kernel_addr);

	Print(L"[*] Starting kernel...");
	// kernel_main(&bootloader_data);

	return EFI_SUCCESS;
}
