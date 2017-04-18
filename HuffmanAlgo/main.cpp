#include "Huffman.h"

void printhelp() {
	printf("Usage:\n");
	printf("compression/decompression mode:\n");
	printf("\t-c/-d \t-i \t<inputfile> \t-o \t<outputfile>\n");
}

int main(int argc, char *argv[]) {
	if (argc != 6 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d"))
		|| strcmp(argv[2], "-i") || strcmp(argv[4], "-o")) {
		printhelp();
		return 1;
	}
	if (strcmp(argv[1], "-c") == 0) {
		if (compressFile(argv[3], argv[5])) {
			printf("compressed the file %s into %s\n", argv[3], argv[5]);
		}
	}
	else {
		if (decompressFile(argv[3], argv[5])) {
			printf("decompressed the file %s into %s\n", argv[3], argv[5]);
		}
	}
	return 0;
}
