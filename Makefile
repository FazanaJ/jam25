all: BINARIES_BUILT jam25.z64 $(DSO_LIST)
.PHONY: all  BINARIES_BUILT

ROM_NAME = jam25
BUILD_DIR = build
T3D_INST ?= ../t3d
FAST_COMPILE = true

ifeq ($(FAST_COMPILE),true)
  N64_ROM_ELFCOMPRESS = 1
  DSO_COMPRESS_LEVEL = 1
  ASSET_COMPRESS_LEVEL = 1
else
  N64_ROM_ELFCOMPRESS = 3
  DSO_COMPRESS_LEVEL = 2
  ASSET_COMPRESS_LEVEL = 2
endif

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

src = $(wildcard src/main/*.c) \
	  $(wildcard src/game/*.c)

overlay = $(wildcard src/extensions/*.c) \
          $(wildcard src/objects/*.c) \
          $(wildcard src/scenes/*.c)

asset = $(notdir $(wildcard assets/*.bin))

sprite_paths := $(shell find assets -name "*.png")
sprite_list := $(notdir $(sprite_paths))

gltf_paths := $(shell find assets -name "*.glb")
gltf_list := $(notdir $(gltf_paths))

xm_paths := $(shell find assets -name "*.xm")
xm_list := $(notdir $(xm_paths))

font_paths := $(shell find assets -name "*.ttf")
font_list := $(notdir $(font_paths))

sound_lq_paths := $(shell find assets -name "*.1.wav")
sound_lq_list := $(notdir $(sound_lq_paths))

sound_mq_paths := $(shell find assets -name "*.2.wav")
sound_mq_list := $(notdir $(sound_mq_paths))

sound_hq_paths := $(shell find assets -name "*.3.wav")
sound_hq_list := $(notdir $(sound_hq_paths))

mp3_paths := $(shell find assets -name "*.4.wav")
mp3_list := $(notdir $(mp3_list))

opus_paths := $(shell find assets -name "*.0.wav")
opus_list := $(notdir $(opus_paths))

text_paths := $(shell find assets -name "*.txt")
text_list := $(notdir $(text_paths))

MAIN_ELF_EXTERNS := $(BUILD_DIR)/$(ROM_NAME).externs
DSO_MODULES := $(notdir $(overlay:.c=.dso))
DSO_LIST := $(addprefix filesystem/, $(DSO_MODULES))

asset_list := $(addprefix filesystem/, $(asset)) \
			  $(addprefix filesystem/,$(text_list:.txt=.bin)) \
			  $(addprefix filesystem/,$(sprite_list:.png=.sprite)) \
			  $(addprefix filesystem/,$(xm_list:.xm=.xm64)) \
			  $(addprefix filesystem/,$(gltf_list:.glb=.t3dm)) \
			  $(addprefix filesystem/,$(font_list:.ttf=.font64)) \
			  $(addprefix filesystem/,$(sound_lq_list:.1.wav=.1.wav64)) \
			  $(addprefix filesystem/,$(sound_mq_list:.2.wav=.2.wav64)) \
			  $(addprefix filesystem/,$(sound_hq_list:.3.wav=.3.wav64)) \
			  $(addprefix filesystem/,$(mp3_list:.4.wav=.4.wav64)) \
			  $(addprefix filesystem/,$(opus_list:.0.wav=.0.wav64))

BINARIES_BUILT = $(BUILD_DIR)/binaries_built.stamp
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)
CFLAGS += -Iinclude

N64_CFLAGS += $(DEF_INC_CFLAGS) \
	-Os \
	-Werror=double-promotion \
	-mno-check-zero-division \
	-funsafe-math-optimizations \
	-fsingle-precision-constant \
	-falign-functions=16 \
	-fno-merge-constants \
    -fno-strict-aliasing \
	-ffast-math \
    -mips3 \
	-MMD -MP

MKSPRITE_FLAGS ?=
MKFONT_FLAGS ?=

filesystem/%.bin: $(text_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).txt,$(text_paths)))
	@mkdir -p $(dir $@)
	@echo "    [TXT] $@"
	$(N64_BINDIR)/mkasset -c $(ASSET_COMPRESS_LEVEL) -o filesystem "$(SRC)"
	mv "filesystem/$(notdir $(SRC))" "$@"

filesystem/%.font64: $(font_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).ttf,$(font_paths)))
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -c $(ASSET_COMPRESS_LEVEL) -o filesystem "$(SRC)"

filesystem/%.sprite: $(sprite_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).png,$(sprite_paths)))
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -c $(ASSET_COMPRESS_LEVEL) -o filesystem "$(SRC)"

BASE_SCALE = 32

filesystem/%.t3dm: $(gltf_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).glb,$(gltf_paths)))
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$(SRC)" $@ --base-scale=$(BASE_SCALE)

filesystem/%.xm64: $(xm_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).xm,$(xm_paths)))
	@mkdir -p $(dir $@)
	@echo "    [SEQUENCE] $@"
	@$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o filesystem "$(SRC)"

filesystem/%.1.wav64: $(sound_lq_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).wav,$(sound_lq_paths)))
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 --wav-resample 12000 --wav-mono -o filesystem "$(SRC)"

filesystem/%.2.wav64: $(sound_mq_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).wav,$(sound_mq_paths)))
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 --wav-resample 16000 --wav-mono -o filesystem "$(SRC)"

filesystem/%.3.wav64: $(sound_hq_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).wav,$(sound_hq_paths)))
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 --wav-resample 22000 --wav-mono -o filesystem "$(SRC)"

filesystem/%.4.wav64: $(mp3_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).wav,$(mp3_paths)))
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 --wav-resample 12025 --wav-mono -o filesystem "$(SRC)"

filesystem/%.0.wav64: $(opus_paths)
	$(eval SRC := $(filter %/$(basename $(notdir $@)).wav,$(opus_paths)))
	@mkdir -p $(dir $@)
	@echo "    [OPUS] $@ "
	@$(N64_AUDIOCONV) --wav-compress 3 --wav-resample 12025 --wav-mono -o filesystem "$(SRC)"

$(foreach file,$(overlay),$(eval filesystem/$(notdir $(basename $(file))).dso: $(BUILD_DIR)/$(file:.c=.o)))

BINARIES_BUILT:
	@mkdir -p $(BUILD_DIR)

$(ROM_NAME).z64: N64_ROM_TITLE = "jam25"
$(ROM_NAME).z64: $(BUILD_DIR)/$(ROM_NAME).dfs $(BUILD_DIR)/$(ROM_NAME).msym
$(BUILD_DIR)/$(ROM_NAME).msym: $(BUILD_DIR)/$(ROM_NAME).elf

$(BUILD_DIR)/$(ROM_NAME).dfs: $(DSO_LIST) $(asset_list) BINARIES_BUILT
$(BUILD_DIR)/$(ROM_NAME).elf: $(src:%.c=$(BUILD_DIR)/%.o) $(MAIN_ELF_EXTERNS)
$(MAIN_ELF_EXTERNS): $(DSO_LIST)


clean:
	rm -rf $(BUILD_DIR) *.z64 filesystem bin
.PHONY: clean

-include $(shell find $(BUILD_DIR) -name '*.d' 2>/dev/null)
