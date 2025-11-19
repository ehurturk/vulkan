CC=glslc

.PHONY = all clean

all: triangle

triangle:
	$(CC) triangle.vert -o tri_vert.spv
	$(CC) triangle.frag -o tri_frag.spv

clean:
	rm -rf *.spv
