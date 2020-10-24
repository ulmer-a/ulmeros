


all: meta

meta:
	@ echo "[ INFO ] generating version information"
	@ tools/version.sh

.phony: meta
