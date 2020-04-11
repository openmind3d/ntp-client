.PHONY: build
build:
	gcc main.c -o ntp

.DEFAULT_GOAL := build