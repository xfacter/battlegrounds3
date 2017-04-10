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

PSP_EBOOT_TITLE = Battlegrounds3
PSP_EBOOT_SFO = $(LOCAL_BIN)/PARAM.SFO
PSP_EBOOT_ICON = $(LOCAL_PBP)/icon0.png
#PSP_EBOOT_ICON1 = $(LOCAL_PBP)/icon1.pmf
#PSP_EBOOT_UNKPNG = $(LOCAL_PBP)/pic0.png
#PSP_EBOOT_PIC1 = $(LOCAL_PBP)/pic1.png
#PSP_EBOOT_SND0 = $(LOCAL_PBP)/snd0.at3
#PSP_EBOOT_PSAR = $(LOCAL_PBP)/data.psar
PSP_EBOOT = $(LOCAL_BIN)/EBOOT.PBP
EXTRA_TARGETS = $(PSP_EBOOT)

#BUILD_PRX = 1
TARGET = $(LOCAL_BIN)/$(PSP_EBOOT_TITLE)

SRC_FILES := $(wildcard $(LOCAL_SRC)/*.c)
SRC_FILES += $(foreach sdir,$(LOCAL_MOD_SRC),$(wildcard $(sdir)/*.c))
OBJ_FILES := $(patsubst $(LOCAL_SRC)/%.c,$(LOCAL_TMP)/%.o,$(SRC_FILES))
OBJS = $(OBJ_FILES)

# show warnings and generate dependency graphs (in .d files)
CFLAGS = -Wall -MMD
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspgum_vfpu -lpspvfpu -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lm
LDFLAGS =


# define targets here (note some definitions also exist in pspsdk's build.mak)
# TODO:
# 	make separate build dirs for different distribution types
# 	move assets to own folder, rsync for all distribution types

debug: CFLAGS += -O0 -DDEBUG -g3 #-gdwarf
debug: mkflags all

staging: CFLAGS += -O2 -g #-gdwarf
#staging: EXTRA_TARGETS = $(PSP_EBOOT)
staging: clean mkflags all #staging_more
	mkdir -p $(LOCAL_DIST_STAGING)
	rsync -r $(PSP_EBOOT) $(LOCAL_RES_PATHS) $(LOCAL_DIST_STAGING)

release: CFLAGS += -O3 #-g -gdwarf
#release: EXTRA_TARGETS = $(PSP_EBOOT)
release: clean mkflags all #release_more
	mkdir -p $(LOCAL_DIST_RELEASE)
	rsync -r $(PSP_EBOOT) $(LOCAL_RES_PATHS) $(LOCAL_DIST_RELEASE)

clean: clean_more

clean_more:
	-rm -rf $(LOCAL_DIST_STAGING) $(LOCAL_DIST_RELEASE)

clean_all: clean
	-rm -rf $(LOCAL_TMP)

.PHONY: mkflags
mkflags: CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
mkflags: ASFLAGS = $(CFLAGS)

## generally no need to edit below this line!

#.PHONY: clean
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# include generated dependency graph files (see -MMD flag)
-include $(OBJ_FILES:.o=.d)

$(OBJ_FILES): $(LOCAL_TMP)/%.o: $(LOCAL_SRC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

