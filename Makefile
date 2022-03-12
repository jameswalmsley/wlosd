

all: build/meson.build
	ninja -C build


build/meson.build:
	meson build

clean:
	rm -rf build

