# Root vars
TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
GH_SDK      := $(GOLDHEN_SDK)
PROJDIR     := ../$(shell basename $(CURDIR))/source
INTDIR      := ../$(shell basename $(CURDIR))/build

# Define objects to build
CFILES      := $(wildcard $(PROJDIR)/*.c)
CPPFILES    := $(wildcard $(PROJDIR)/*.cpp)
OBJS        := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) \
               $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES))

# Define target C/C++ flags
LIBS        := -lGoldHEN_Hook -lkernel -lSceLibcInternal -lSceSysmodule -lScePad
EXTRAFLAGS  := -D__FINAL__=1 -D__PS5__=1

# Compiler / Linker flags
INCLUDEDIR  := include
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/$(INCLUDEDIR) -I$(GH_SDK)/include -Iinclude $(EXTRAFLAGS)
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/$(INCLUDEDIR)/c++/v1
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib -L$(GH_SDK) $(LIBS)

# Determine OS bin folder
ifeq ($(OS),Windows_NT)
    CDIR := windows
else
    CDIR := linux
endif

CC          := clang
CXX         := clang++
LD          := ld.lld

BUILD_FOLDER := bin
OUTPUT_PRX   := $(shell basename $(CURDIR))
TARGET       := $(BUILD_FOLDER)/$(OUTPUT_PRX)
TARGET_ELF   := $(BUILD_FOLDER)/$(OUTPUT_PRX)

.PHONY: all clean
.DEFAULT_GOAL := all

all: $(TARGET)

$(INTDIR):
	mkdir -p $(INTDIR)
	mkdir -p $(BUILD_FOLDER)

$(INTDIR)/%.o: $(PROJDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(INTDIR) $(OBJS)
	$(LD) $(GH_SDK)/build/crtprx.o $(INTDIR)/*.o -o $(TARGET_ELF).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(TARGET_ELF).elf -out=$(TARGET_ELF).oelf --lib=$(TARGET).prx --paid 0x3800000000000011

clean:
	rm -rf $(BUILD_FOLDER) $(INTDIR) $(OBJS)
