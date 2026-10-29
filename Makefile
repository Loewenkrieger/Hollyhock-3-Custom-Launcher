MAKEFLAGS = rR


SOURCEDIR = src
BUILDDIR = obj-classic
OUTDIR = dist
DEPDIR = .deps-classic
METADIR = meta

AS:=sh4a_nofpueb-elf-gcc
AS_FLAGS:=-gdwarf-5

SDK_DIR?=meta/hollyhock-3/sdk
SDK_PATH=$(if $(filter /%,$(SDK_DIR)),$(SDK_DIR),../../$(SDK_DIR))
ADDRESSES_FILE=meta/hollyhock-3/versions/v02.01.7002.0001/addresses.json

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
WARNINGS=-Wall -Wextra -Wno-prio-ctor-dtor -pedantic -Werror -pedantic-errors
INCLUDES=-I$(SDK_DIR)/include -I$(SOURCEDIR)
DEFINES=
FUNCTION_FLAGS=-flto=auto -ffat-lto-objects -fno-builtin -ffunction-sections -fdata-sections -gdwarf-5 -Os
COMMON_FLAGS=$(FUNCTION_FLAGS) $(INCLUDES) $(WARNINGS) $(DEFINES)

CC:=sh4a_nofpueb-elf-gcc
CC_FLAGS=-std=c23 $(COMMON_FLAGS)

CXX:=sh4a_nofpueb-elf-g++
CXX_FLAGS=-std=c++23 $(COMMON_FLAGS)

LD:=sh4a_nofpueb-elf-g++
LD_FLAGS:=$(FUNCTION_FLAGS) -Wl,--gc-sections --specs=custom-start.specs
LIBS:=-L$(SDK_DIR) -lsdk

READELF:=sh4a_nofpueb-elf-readelf
OBJCOPY:=sh4a_nofpueb-elf-objcopy
STRIP:=sh4a_nofpueb-elf-strip

PYTHON?=python3

VERSIONS := 7002
OVERRIDES := $(OUTDIR)/addresses.2023.0209.1500.override
$(OVERRIDES): $(BUILDDIR)/addresses.7002.prep

APP_ELFS := $(VERSIONS:%=$(OUTDIR)/run.%.elf)
APP_BINS := $(APP_ELFS:.elf=.bin)
APP_HH3S := $(APP_ELFS:.elf=.adjusted.hh3)

AS_SOURCES:=$(shell find $(SOURCEDIR) -name '*.S' -not -name '*.template.*')
CC_SOURCES:=$(shell find $(SOURCEDIR) -name '*.c' -not -name '*.template.*')
CXX_SOURCES:=$(shell find $(SOURCEDIR) -name '*.cpp' -not -name '*.template.*')
OBJECTS := $(addprefix $(BUILDDIR)/,$(AS_SOURCES:.S=.o)) \
	$(addprefix $(BUILDDIR)/,$(CC_SOURCES:.c=.o)) \
	$(addprefix $(BUILDDIR)/,$(CXX_SOURCES:.cpp=.o))

NOLTOOBJS := $(foreach obj, $(OBJECTS), $(if $(findstring /nolto/, $(obj)), $(obj)))

DEPFILES := $(OBJECTS:$(BUILDDIR)/%.o=$(DEPDIR)/%.d)

bin: $(APP_BINS) Makefile
hh3: $(APP_HH3S) Makefile
elf: $(APP_ELFS) Makefile

overrides: $(OVERRIDES) Makefile

all: launcher smoke
.DEFAULT_GOAL := launcher
.SECONDARY: # Prevents intermediate files from being deleted

sdk:
	@test -f "$(SDK_DIR)/Makefile" || { echo 'SDK missing: run git submodule update --init --recursive'; exit 1; }
	$(MAKE) -C "$(SDK_DIR)"

$(SDK_DIR)/libsdk.a:
	$(MAKE) sdk

launcher: sdk
	$(MAKE) bin elf hh3 overrides

smoke: sdk
	$(MAKE) -C smoke/hh3 SDK_DIR="$(SDK_PATH)" APP_NAME=launcher-test

test:
	$(PYTHON) tools/run_tests.py host

target-test: launcher
	$(PYTHON) tools/run_tests.py target --outdir "$(OUTDIR)"

package: launcher smoke
	$(PYTHON) tools/package.py --outdir "$(OUTDIR)"

clean:
	rm -rf $(BUILDDIR) $(DEPDIR)
	rm -f $(APP_BINS) $(APP_ELFS) $(APP_ELFS:%=%.map) $(APP_HH3S) $(OVERRIDES)
	$(MAKE) -C smoke/hh3 SDK_DIR="$(SDK_PATH)" APP_NAME=launcher-test clean


%.bin: %.elf
	$(OBJCOPY) --output-target=binary --set-section-flags .bss=alloc,load,contents,data $^ $@

%.hh3: %.elf
	$(STRIP) -o $@ $^

$(OUTDIR)/run.%.elf: $(OBJECTS) linker.ld $(BUILDDIR)/addresses.%.o $(SDK_DIR)/libsdk.a
	@mkdir -p $(dir $@)
	$(LD) -T linker.ld -Wl,-Map $@.map -o $@ $(LD_FLAGS) $(filter-out linker.ld $(SDK_DIR)/libsdk.a, $^) $(LIBS)

$(BUILDDIR)/run.%.adjusted.elf: $(OUTDIR)/run.%.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) --change-addresses +0x20000 $^ $@

$(OUTDIR)/addresses.%.override:
	@mkdir -p $(dir $@)
	cp $< $@

$(OUTDIR)/run.%.adjusted.hh3: $(BUILDDIR)/run.%.adjusted.hh3
	@mkdir -p $(dir $@)
	cp $< $@

$(NOLTOOBJS): FUNCTION_FLAGS+=-fno-lto

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) -c $< -o $@ $(AS_FLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEPDIR)/$<)
	$(CC) -c $< -o $@ $(CC_FLAGS) $(DEPFLAGS)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEPDIR)/$<)
	$(CXX) -c $< -o $@ $(CXX_FLAGS) $(DEPFLAGS)

$(BUILDDIR)/addresses.7002.prep: $(ADDRESSES_FILE) convert_json.py
	@mkdir -p $(dir $@)
	$(PYTHON) convert_json.py $< $@

$(BUILDDIR)/addresses.%.o: $(SOURCEDIR)/addresses.template.c $(BUILDDIR)/addresses.%.prep
	$(CC) -c $< -o $@ $(CC_FLAGS) --embed-dir=$(BUILDDIR) -D PREP_FILE=\<$(@:$(BUILDDIR)/addresses.%.o=addresses.%.prep)\>

#clangd gets confused on these flags
compile_commands.json: clean
	bear --output $@.tmp -- sh -c "$(MAKE) --keep-going all || exit 0"
	#sed -i 's/-m4a-nofpu//' compile_commands.json.tmp
	mv compile_commands.json.tmp compile_commands.json

.PHONY: bin elf hh3 overrides all sdk launcher smoke test target-test package clean compile_commands.json

-include $(DEPFILES)
