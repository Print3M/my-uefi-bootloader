CC 				= gcc
ARCH            := x86_64

SRC_DIR			= .
OBJ_DIR			= objs
TARGET_DIR		= result
SRCS 			:= $(wildcard $(SRC_DIR)/*.c)
OBJS 			:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
				
EFIINC          = ../inc
EFIINCS         = -I $(EFIINC) -I $(EFIINC)/$(ARCH) -I $(EFIINC)/protocol
LIB             = ../$(ARCH)/lib
EFILIB          = ../$(ARCH)/gnuefi
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = ../gnuefi/elf_$(ARCH)_efi.lds

CFLAGS          = $(EFIINCS) -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall -Wextra -std=c11 -DGNU_EFI_USE_MS_ABI
ifeq ($(ARCH),x86_64)
  CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDLIBS			= -l:libgnuefi.a  -l:libefi.a 
LDFLAGS         = -T $(EFI_LDS) -L $(EFILIB) -L $(LIB) -nostdlib --build-id=sha1 --warn-common --no-undefined --fatal-warnings -znocombreloc -shared -Bsymbolic  

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo !==== COMPILING $^ ====!
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET_DIR)/bootloader.so: $(OBJS)
	@echo !==== LINKING $^ ====!
	@ld $(OBJS) $(EFI_CRT_OBJS) $(LDFLAGS) $(LDLIBS) -o $@
	@echo !==== MAKING EFI PE FORMAT ====!
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --verbose --target=efi-app-$(ARCH) $@ $(TARGET_DIR)/main.efi
	
clean:
	@rm $(OBJ_DIR)/*
	@rm $(TARGET_DIR)/*
