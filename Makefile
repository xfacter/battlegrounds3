### PROJECT SETTINGS ###
BIN_NAME = bg3
PBP_PATH = Bundle
RES_PATH = Assets
SRC_PATH = src
SRC_EXT = c
#BASE_BIN = bin
#BASE_BUILD = build

# Define basic configuration for PSPSDK. Refer to $(PSPSDK)/lib/build.mak
PSPSDK=$(shell psp-config --pspsdk-path)
BUILD_PRX = 1
PSP_EBOOT_TITLE = Battlegrounds3
PSP_EBOOT_ICON = $(PBP_PATH)/icon0.png
#PSP_EBOOT_ICON1 = $(PBP_PATH)/icon1.pmf
#PSP_EBOOT_UNKPNG = $(PBP_PATH)/pic0.png
#PSP_EBOOT_PIC1 = $(PBP_PATH)/pic1.png
#PSP_EBOOT_SND0 = $(PBP_PATH)/snd0.at3
#PSP_EBOOT_PSAR = $(PBP_PATH)/data.psar
PSP_EBOOT_SFO = $(BUILD_PATH)/PARAM.SFO
PSP_EBOOT = $(BIN_PATH)/EBOOT.PBP
EXTRA_TARGETS = $(PSP_EBOOT)
TARGET = $(BUILD_PATH)/$(BIN_NAME)

# C, C++, ASM flags
CFLAGS = -Wall #-std=c99
CXXFLAGS = -fno-exceptions -fno-rtti # some optimizations
ASFLAGS = $(CFLAGS)
# additional flags depending on build type
CFLAGS_RLS = -O3 -g -D NDEBUG #-gdwarf
CFLAGS_DBG = -O1 -g3 -D DEBUG #-gdwarf

# include and lib paths
INCDIR =
LIBDIR =
LIBS = -lpspgum_vfpu -lpspvfpu -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lm
LDFLAGS =
### END PROJECT SETTINGS ###

# Generally should not need to edit below this line

# Install destination
ifeq ($(DESTDIR),)
	DESTDIR = $(PSP_ROOT)
endif
ifeq ($(DESTDIR),)
	DESTDIR = $(HOME)/psproot
endif
INSTALL_PREFIX = /PSP/GAME
INSTALL_PATH = $(DESTDIR)$(INSTALL_PREFIX)/$(BIN_NAME)

# Programs for installation
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# Build and output paths
release: export BUILD_PATH := build/release
release: export BIN_PATH := bin/release
debug: export BUILD_PATH := build/debug
debug: export BIN_PATH := bin/debug
install: export BIN_PATH := bin/release

SRC_FILES := $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' -type f)
#OBJ_FILES = $(patsubst $(LOCAL_SRC)/%.c,$(BUILD_PATH)/%.o,$(SRC_FILES))
OBJ_FILES = $(SRC_FILES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
OBJS = $(OBJ_FILES)
DEPS = $(OBJS:.o=.d)

.PHONY: release
release: CFLAGS_OPT = $(CFLAGS_RLS)
release: CFLAGS += $(CFLAGS_OPT)
release: dirs deploy
	@$(MAKE) all

.PHONY: debug
debug: CFLAGS_OPT = $(CFLAGS_DBG)
debug: CFLAGS += $(CFLAGS_OPT)
debug: dirs deploy
	@$(MAKE) all

.PHONY: dirs
dirs:
	@echo "Creating directories ..."
	mkdir -p $(sort $(dir $(OBJ_FILES))) $(BIN_PATH)

.PHONY: deploy
deploy:
	rsync -a README.md $(RES_PATH)/ $(BIN_PATH)

.PHONY: install
install:
	@echo "Installing to PSP ..."
	mkdir -p $(INSTALL_PATH)
	rsync -a $(BIN_PATH)/ $(INSTALL_PATH)

.PHONY: uninstall

.PHONY: clean_all
clean_all:
	$(RM) -r bin build

.PHONY: clean
clean: clean_all

.PHONY: all
include $(PSPSDK)/lib/build.mak
# NOTE the PSPSDK makefile defines the standard targets as:
# all: $(EXTRA_TARGETS) $(FINAL_TARGET)
# clean:
#     -rm -f $(FINAL_TARGET) $(EXTRA_CLEAN) $(OBJS) $(PSP_EBOOT_SFO) $(PSP_EBOOT) $(EXTRA_TARGETS)
# $(TARGET).elf: $(OBJS) $(EXPORT_OBJ)
#     $(LINK.c) $^ $(LIBS) -o $@
# NOTE the PSPSDK makefile redefines compile flags as:
# CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
# CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
# ASFLAGS  := $(CFLAGS) $(ASFLAGS)
# LDFLAGS  := $(addprefix -L,$(LIBDIR)) $(LDFLAGS)

# Add dependency files, if they exist
-include $(DEPS)

#$(OBJS): $(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -MP -MMD -c $(CFLAGS) $< -o $@
