#include "utils.h"
#include <efi.h>
#include <efibind.h>
#include <efilib.h>
#include <elf.h>

EFI_STATUS get_image(EFI_HANDLE image_handle, EFI_LOADED_IMAGE_PROTOCOL **loaded_image) {
	EFI_STATUS status = ST->BootServices->OpenProtocol(image_handle,
													   &gEfiLoadedImageProtocolGuid,
													   (void **) loaded_image,
													   image_handle,
													   NULL,
													   EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (status == EFI_UNSUPPORTED) {
		print_err(L"The device doesn't support specified image protocol", status);
	} else if (status != EFI_SUCCESS) {
		print_err(L"Opening EFI Loaded Image Protocol failed", status);
	}
	return status;
}

EFI_STATUS get_root_fs(EFI_HANDLE image_handle,
					   EFI_LOADED_IMAGE_PROTOCOL *loaded_image,
					   EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **root_fs) {
	EFI_STATUS status = ST->BootServices->OpenProtocol(loaded_image->DeviceHandle,
													   &gEfiSimpleFileSystemProtocolGuid,
													   (void **) root_fs,
													   image_handle,
													   NULL,
													   EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (status == EFI_UNSUPPORTED) {
		print_err(L"The device does not support specified file system protocol", status);
	} else if (status != EFI_SUCCESS) {
		print_err(L"Opening EFI Simple File System failed", status);
	}
	return status;
}

EFI_STATUS get_root_dir(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *root_fs, EFI_FILE **dir) {
	EFI_STATUS status = root_fs->OpenVolume(root_fs, dir);
	if (status != EFI_SUCCESS) {
		print_err(L"Accessing root directory failed", status);
	}
	print_err(L"Debug", status);
	return status;
}

EFI_STATUS get_file(EFI_FILE *dir, CHAR16 *file_name, EFI_FILE **file) {
	EFI_STATUS status = dir->Open(dir, file, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (status != EFI_SUCCESS) {
		print_err(L"Fail during opening file", status);
	}
	return status;
}

EFI_FILE *load_file(EFI_FILE *dir, CHAR16 *file_name, EFI_HANDLE image_handle) {

	// Get loaded image (by image_handler)
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image = NULL;
	if (get_image(image_handle, &loaded_image) != EFI_SUCCESS) {
		return NULL;
	}

	// Get file system
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *root_fs = NULL;
	if (get_root_fs(image_handle, loaded_image, &root_fs) != EFI_SUCCESS) {
		return NULL;
	}

	// If directory not specified, get root directory of volume
	if (dir == NULL) {
		if (get_root_dir(root_fs, &dir) != EFI_SUCCESS) {
			return NULL;
		}
	}

	// Open file
	EFI_FILE *file = NULL;
	if (get_file(dir, file_name, &file) != EFI_SUCCESS) {
		return NULL;
	}

	return file;
}