#include "kernel_loader.h"
#include "utils.h"
#include <efi.h>
#include <efilib.h>
#include <elf.h>

BOOLEAN verify_kernel_elf_header(Elf64_Ehdr *e_hdr) {
	return memcmp(&e_hdr->e_ident[EI_MAG0], ELFMAG, SELFMAG) == 0 &&
		   e_hdr->e_ident[EI_CLASS] == ELFCLASS64 && e_hdr->e_ident[EI_DATA] == ELFDATA2LSB &&
		   e_hdr->e_type == ET_EXEC && e_hdr->e_machine == EM_X86_64 &&
		   e_hdr->e_version == EV_CURRENT;
}

void *load_kernel_into_memory(EFI_HANDLE image_handle,
							  EFI_SYSTEM_TABLE *system_table,
							  EFI_FILE *kernel_file) {
	/*
		Returns entry point to the kernel.
	*/
	EFI_STATUS status = 0;
	UINTN kernel_file_sz = 0;
	EFI_FILE_INFO *kernel_file_info = NULL;

	kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &kernel_file_sz, NULL);
	system_table->BootServices->AllocatePool(
		EfiLoaderData, kernel_file_sz, (void **) &kernel_file_info);
	kernel_file->GetInfo(
		kernel_file, &gEfiFileInfoGuid, &kernel_file_sz, (void *) kernel_file_info);

	// Read ELF file header
	Elf64_Ehdr e_hdr;
	UINTN e_hdr_sz = sizeof(e_hdr);
	status = kernel_file->Read(kernel_file, &e_hdr_sz, &e_hdr);
	if (status != EFI_SUCCESS || e_hdr_sz == 0) {
		Print(L"[!] Error: Unsucessfully reading ELF file header \n\r");
		return NULL;
	}

	// Verify ELF header
	if (verify_kernel_elf_header(&e_hdr)) {
		Print(L"[!] Error: Kernel ELF format is bad \n\r");
		return NULL;
	}

	// Allocate memory for ELF program headers
	Elf64_Phdr *p_hdrs;
	UINTN all_p_hdrs_sz = e_hdr.e_phentsize * e_hdr.e_phnum;
	system_table->BootServices->AllocatePool(EfiLoaderData, all_p_hdrs_sz, (void **) &p_hdrs);

	// Load ELF program headers into memory
	kernel_file->SetPosition(kernel_file, e_hdr.e_phoff);
	status = kernel_file->Read(kernel_file, &all_p_hdrs_sz, (void *) p_hdrs);
	if (status != EFI_SUCCESS || all_p_hdrs_sz == 0) {
		Print(L"[!] Error: Unsucessfully reading ELF program headers \n\r");
		return NULL;
	}

	// Get first program header
	Elf64_Phdr *p_hdr = p_hdrs;

	// TODO: Maybe I should use other type of memory than EfiLoaderData
	// Maybe some custom type for kernel???

	// Iterate through program headers and load PT_LOAD segments
	for (int i = 0; i < e_hdr.e_phnum; i++) {
		if (i > 0) {
			// Get next program header
			p_hdr = (Elf64_Phdr *) ((UINT64) p_hdr + e_hdr.e_phentsize);
		}

		if (p_hdr->p_type == PT_LOAD) {
			// Allocate pages for segment at its physical address
			UINT16 num_of_pages = (p_hdr->p_filesz / PAGE_SZ) + 1;
			Elf64_Addr segment_physical_addr = p_hdr->p_paddr;
			system_table->BootServices->AllocatePages(
				AllocateAddress, EfiLoaderData, num_of_pages, &segment_physical_addr);

			// Load segment into memory
			kernel_file->SetPosition(kernel_file, p_hdr->p_offset);
			UINTN segment_file_sz = p_hdr->p_filesz;
			status =
				kernel_file->Read(kernel_file, &segment_file_sz, (void *) segment_physical_addr);
			if (status != EFI_SUCCESS) {
				Print(L"[!] Error: Unsucessfully reading one of ELF's PT_LOAD segments \n\r");
				return EFI_LOAD_ERROR;
			}
		}
	}

	Print(L"[+] Kernel has been successfully loaded into memory \n\r");
	return (void *) e_hdr.e_entry;
}