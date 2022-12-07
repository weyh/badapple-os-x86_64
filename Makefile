PROJECT_NAME := bad_apple_os_x86_64
CC := x86_64-elf-gcc
CFLAGS := -Wextra -Wall -Wfloat-equal -Wcast-align -std=c11 -pedantic-errors -ffreestanding

root_dir := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
build_dir := build
dist_dir := dist
build_vid_dir := $(build_dir)/vid

mini_vid := $(build_vid_dir)/download/mini.mp4

x86_64_asm_source_files := $(shell find src/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/x86_64/%.asm, $(build_dir)/x86_64/%.o, $(x86_64_asm_source_files))

kernel_source_files := $(shell find src/kernel -name *.c)
kernel_object_files := $(patsubst src/kernel/%.c, $(build_dir)/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find src/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/x86_64/%.c, $(build_dir)/x86_64/%.o, $(x86_64_c_source_files))

bad_apple_c_source_files := $(shell find src/bad_apple -name *.c)
bad_apple_c_object_files := $(patsubst src/bad_apple/%.c, $(build_dir)/bad_apple/%.o, $(bad_apple_c_source_files))

x86_64_object_files := $(x86_64_asm_object_files) $(x86_64_c_object_files)

$(x86_64_asm_object_files): $(build_dir)/x86_64/%.o : src/x86_64/%.asm
	mkdir -p $(dir $@) && \
	nasm -f elf64 $(patsubst $(build_dir)/x86_64/%.o, src/x86_64/%.asm, $@) -o $@

$(kernel_object_files): $(build_dir)/kernel/%.o : src/kernel/%.c
	mkdir -p $(dir $@) && \
	$(CC) -I src/interface -c $(patsubst $(build_dir)/kernel/%.o, src/kernel/%.c, $@) -o $@ $(CFLAGS)

$(x86_64_c_object_files): $(build_dir)/x86_64/%.o : src/x86_64/%.c
	mkdir -p $(dir $@) && \
	$(CC) -I src/interface -c $(patsubst $(build_dir)/x86_64/%.o, src/x86_64/%.c, $@) -o $@ $(CFLAGS)

$(bad_apple_c_object_files): $(build_dir)/bad_apple/%.o : src/bad_apple/%.c
	mkdir -p $(dir $@) && \
	$(CC) -I src/interface -I $(build_vid_dir) -c $(patsubst $(build_dir)/bad_apple/%.o, src/bad_apple/%.c, $@) -o $@ $(CFLAGS)

.PHONY: build-x86_64
build-x86_64: $(bad_apple_c_object_files) $(kernel_object_files) $(x86_64_object_files)
	mkdir -p $(dist_dir)/x86_64 && \
	cp -r target/x86_64/boot $(dist_dir)/x86_64 && \
	mv $(dist_dir)/x86_64/boot/grub/grub.cfg.templ $(dist_dir)/x86_64/boot/grub/grub.cfg && \
	sed -i "s/PROJECT_NAME/$(PROJECT_NAME)/g" $(dist_dir)/x86_64/boot/grub/grub.cfg && \
	x86_64-elf-ld -n -o $(dist_dir)/x86_64/boot/$(PROJECT_NAME).bin -T target/x86_64/linker.ld $(bad_apple_c_object_files) $(kernel_object_files) $(x86_64_object_files) && \
	grub-mkrescue /usr/lib/grub/i386-pc -o $(dist_dir)/x86_64/$(PROJECT_NAME).iso $(dist_dir)/x86_64

kall: build-x86_64

asd:
	cp $(dist_dir)/x86_64/$(PROJECT_NAME).bin target/x86_64/boot/$(PROJECT_NAME).bin &&

$(build_vid_dir)/download/ba_vid.mp4:
	mkdir -p $(build_vid_dir)/download && \
	cd $(build_vid_dir)/download && \
	yt-dlp --merge-output-format mp4 -f 'bestvideo+bestaudio[ext=m4a]/bestvideo+bestaudio' -o "ba_vid.%(ext)s" https://youtu.be/FtutLA63Cp8 && \
	cd $(root_dir)

$(mini_vid): $(build_vid_dir)/download/ba_vid.mp4
	ffmpeg -y -i $(build_vid_dir)/download/ba_vid.mp4 -vf "scale=-1:24" -r 12 $(mini_vid)

vid-to-pngs: $(mini_vid)
	mkdir -p $(build_vid_dir)/pngs && \
	ffmpeg -i $(mini_vid) '$(build_vid_dir)/pngs/%04d.png'

vid-to-ascii: vid-to-pngs
	mkdir -p $(build_vid_dir)/txt && \
	echo "32x24f12" > $(build_vid_dir)/txt/out.txt && \
	ascii-image-converter $(build_vid_dir)/pngs/*.png -d 32,24 >> $(build_vid_dir)/txt/out.txt

vid-to-header: vid-to-ascii
	mkdir -p build/bad_apple && \
	echo "#ifndef VID_H\n#define VID_H" > $(build_vid_dir)/vid.h && \
	echo 'const char *const bad_apple_vid[] = {' >> $(build_vid_dir)/vid.h && \
	python3 helper/line_by_line.py $(build_vid_dir)/txt/out.txt >> $(build_vid_dir)/vid.h && \
	echo '};' >> $(build_vid_dir)/vid.h && \
	echo "#define BAD_APPLE_VID_LEN (sizeof(bad_apple_vid) / sizeof(bad_apple_vid[0]))" >> $(build_vid_dir)/vid.h && \
	echo "#endif" >> $(build_vid_dir)/vid.h

vall: vid-to-header

.PHONY: all
all: vall kall

.PHONY: clean
clean:
	rm -rf $(build_dir) $(dist_dir)
