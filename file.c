#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_FILE *load_file(EFI_FILE *directory,
				   CHAR16 *file_name,
				   EFI_HANDLE image_handle,
				   EFI_SYSTEM_TABLE *system_table) {
	// TODO: change to OpenProtocol() instead of HandleProtocol (see spec.
	// HandlePrototocol chapter)

	// Just as temp var for next operations
	EFI_STATUS status = 0;

	// IN image_handle
	// IN  gEfiLoadedImageProtocolGuid - EFI_GUID struct with
	// EFI_LOADED_IMAGE_PROTOCOL_GUID const value OUT void** to
	// EFI_LOADED_IMAGE_PROTOCOL struct
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

	status = system_table->BootServices->HandleProtocol(
		image_handle, &gEfiLoadedImageProtocolGuid, (void **) &loaded_image);

	if (status == EFI_UNSUPPORTED) {
		Print(L"[!] Error: The device does not support the specified image "
			  L"protocol \n\r");
		return NULL;
	}

	// IN DeviceHandle
	// IN gEfiSimpleFileSystemProtocolGuid - EFI_GUID struct with
	// EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID const value OUT void ** to
	// EFI_SIMPLE_FILE_SYSTEM_PROTOCOL struct
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system = NULL;

	status = system_table->BootServices->HandleProtocol(
		loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **) &file_system);
	if (status == EFI_UNSUPPORTED) {
		Print(L"[!] Error: The device does not support the specified file "
			  L"system protocol \n\r");
		return NULL;
	}

	// TODO: What if not NULL??
	if (directory == NULL) {
		// IN FileSystem struct
		// OUT Directory* - returns root directory
		status = file_system->OpenVolume(file_system, &directory);
		if (status != EFI_SUCCESS) {
			Print(L"[!] Error: code=%d; accessing root directory "
				  L"cannot be performed \n\r",
				  status);
			return NULL;
		}
	}

	// IN Directory* - current directory
	// OUT LoadedFile** - returned file handle
	// IN FileName* - file name to open
	// IN EFI_FILE_MODE_READ - open mode
	// IN EFI_FILE_READ_ONLY - file attribute
	EFI_FILE *loaded_file = NULL;
	status =
		directory->Open(directory, &loaded_file, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (status != EFI_SUCCESS || loaded_file == NULL) {
		Print(L"[!] Error: Something went wrong during opening directory \n\r");
		return NULL;
	}

	return loaded_file;
}