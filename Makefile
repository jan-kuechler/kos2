BUILDNAME = Drunken Moose

SRCFILES = $(shell find kernel -name "*.c")
HDRFILES = $(shell find kernel -name "*.h")
ASMFILES = $(shell find kernel -name "*.s")

OBJFILES = $(patsubst kernel/%.c,build/objs/%.o,$(SRCFILES))
ASMOBJS  = $(patsubst kernel/%.s,build/objs/%.s.o,$(ASMFILES))

INCLUDE_DIRS = -Ikernel/include -Ishare/include \
               -Ilib/libc/includes -Ilib/libc/internals \
               -Ilib/libutil/include
               
ASM = nasm
ASMFLAGS = -felf

CC = i586-elf-gcc
CFLAGS = -std=gnu99 -Wall -static -O2 -g -DKERNEL -ffreestanding -nostdlib \
         -nostartfiles -nodefaultlibs -fno-builtin $(INCLUDE_DIRS)
         
LD = i586-elf-ld
LDFLAGS = -Llib -static -Tbuild/scripts/kernel.ld

LIBS = -lutil -lc -lgcc

LUA = lua.exe

all: kernel

kernel: build/kos.sys

iso:
	@build/scripts/cpyfiles.sh iso
	@mkisofs -R -b grldr -no-emul-boot -boot-load-size 4 -boot-info-table -o build/kos.iso build/tmp

run:
	@build/scripts/run.sh

prepare:
	@$(LUA) build/scripts/util.lua prepare

build/kos.sys: $(OBJFILES) $(ASMOBJS)
	@$(LD) $(LDFLAGS) -obuild/kos.sys $(ASMOBJS) $(OBJFILES) $(LIBS) -Map linkmap.txt
	
build/.rules: $(SRCFILES) Makefile
	@rm -f build/.rules
	@$(foreach file,$(SRCFILES),\
	echo -n $(subst kernel,build/objs,$(dir $(file))) >> build/.rules;\
	$(CC) $(CFLAGS) -MM $(file) >> build/.rules;\
	echo -e "\t@$(CC) $(CFLAGS) -o$(patsubst kernel/%.c,build/objs/%.o,$(file)) -c $(file)" >> build/.rules;\
	)
	@$(foreach file,$(ASMFILES),\
	echo "$(patsubst kernel/%.s,build/objs/%.s.o,$(file)): $(file)" >> build/.rules;\
	echo -e "\t@$(ASM) $(ASMFLAGS) -o$(patsubst kernel/%.s,build/objs/%.s.o,$(file)) $(file)" >> build/.rules;\
	)

-include build/.rules

clean: partclean
	@$(LUA) build/scripts/util.lua cleanup
	@rm -f build/.rules
	@rm -rf build/tmp
	
partclean:
	@rm -f $(OBJFILES) $(ASMOBJS)