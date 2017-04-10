LOCAL_BIN = bin
LOCAL_SRC = src
LOCAL_TMP = tmp
LOCAL_PBP = $(LOCAL_BIN)
LOCAL_DIST_STAGING = bin.staging
LOCAL_DIST_RELEASE = bin.release

LOCAL_RESOURCES = data maps skies
LOCAL_RES_PATHS:= $(addprefix $(LOCAL_BIN)/,$(LOCAL_RESOURCES))
LOCAL_MODULES = xlib
LOCAL_MOD_SRC:= $(addprefix $(LOCAL_SRC)/,$(LOCAL_MODULES))
LOCAL_MOD_TMP:= $(addprefix $(LOCAL_TMP)/,$(LOCAL_MODULES))
LOCAL_MKDIRS := $(shell mkdir -p $(LOCAL_TMP) $(LOCAL_MOD_TMP))

# Define basic variables for PSPSDK. Refer to $(PSPSDK)/lib/build.mak
PSPSDK=$(shell psp-config --pspsdk-path)
#BUILD_PRX = 1
PSP_EBOOT_TITLE = Battlegrounds3
PSP_EBOOT_ICON = $(LOCAL_PBP)/icon0.png
#PSP_EBOOT_ICON1 = $(LOCAL_PBP)/icon1.pmf
#PSP_EBOOT_UNKPNG = $(LOCAL_PBP)/pic0.png
#PSP_EBOOT_PIC1 = $(LOCAL_PBP)/pic1.png
#PSP_EBOOT_SND0 = $(LOCAL_PBP)/snd0.at3
#PSP_EBOOT_PSAR = $(LOCAL_PBP)/data.psar
PSP_EBOOT_SFO = $(LOCAL_TMP)/PARAM.SFO
PSP_EBOOT = $(LOCAL_BIN)/EBOOT.PBP
EXTRA_TARGETS = $(PSP_EBOOT)
TARGET = $(LOCAL_BIN)/$(PSP_EBOOT_TITLE)

SRC_FILES := $(wildcard $(LOCAL_SRC)/*.c)
SRC_FILES += $(foreach sdir,$(LOCAL_MOD_SRC),$(wildcard $(sdir)/*.c))
OBJ_FILES := $(patsubst $(LOCAL_SRC)/%.c,$(LOCAL_TMP)/%.o,$(SRC_FILES))
OBJS = $(OBJ_FILES)
DEPS = $(OBJS:.o=.d)

# show warnings and generate dependency graphs (in .d files)
CFLAGS = -Wall -MMD
# optimize CXX execution by disabling exception handling and virtual types/functions
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
# additional flags depending on distribution
CFLAGS_DBG = -O0 -g3 -DDEBUG #-gdwarf
CFLAGS_STG = -O2 -g #-gdwarf
CFLAGS_RLS = -O3 -g #-gdwarf

INCDIR =
LIBDIR =
LIBS = -lpspgum_vfpu -lpspvfpu -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lm
LDFLAGS =


# define targets here (note preexisting targets 'all' and 'clean' in $(PSPSDK)/lib/build.mak))
# TODO:
# 	make separate build dirs for different distribution types
# 	move assets to own folder, rsync for all distribution types

debug: CFLAGS_OPT = $(CFLAGS_DBG)
debug: CFLAGS += $(CFLAGS_OPT)
debug: CXXFLAGS += $(CFLAGS_OPT)
debug: ASFLAGS += $(CFLAGS_OPT)
debug: all
# remember to `make clean` again after building a different distribution!

staging: CFLAGS_OPT = $(CFLAGS_STG)
staging: CFLAGS += $(CFLAGS_OPT)
staging: CXXFLAGS += $(CFLAGS_OPT)
staging: ASFLAGS += $(CFLAGS_OPT)
staging: clean_tmp all #staging_more
#	mkdir -p $(LOCAL_DIST_STAGING)
#	rsync -r $(PSP_EBOOT) $(LOCAL_RES_PATHS) $(LOCAL_DIST_STAGING)


release: CFLAGS_OPT = $(CFLAGS_RLS)
release: CFLAGS += $(CFLAGS_OPT)
release: CXXFLAGS += $(CFLAGS_OPT)
release: ASFLAGS += $(CFLAGS_OPT)
release: clean_tmp all #release_more
#	mkdir -p $(LOCAL_DIST_RELEASE)
#	rsync -r $(PSP_EBOOT) $(LOCAL_RES_PATHS) $(LOCAL_DIST_RELEASE)

clean: clean_all

clean_all:
	-rm -rf $(LOCAL_TMP) $(LOCAL_DIST_STAGING) $(LOCAL_DIST_RELEASE)

clean_tmp:
	-rm $(OBJS) $(DEPS)


## generally no need to edit below this line

#.PHONY: clean
include $(PSPSDK)/lib/build.mak

# include generated dependency graph files (Refer to gcc flag '-MMD')
-include $(DEPS)

$(OBJS): $(LOCAL_TMP)/%.o: $(LOCAL_SRC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

