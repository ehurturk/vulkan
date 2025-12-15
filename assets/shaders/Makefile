# CC=glslc
CC = slangc
CFLAGS = -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain

.PHONY = all clean

all: triangle

triangle:
	$(CC) triangle.slang $(CFLAGS) -o triangle.spv

clean:
	rm -rf *.spv
