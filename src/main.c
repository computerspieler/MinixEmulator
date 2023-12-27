#include <stdio.h>
#include <stdlib.h>

#include "exec_format.h"
#include "i386.h"

void die()
{
	printf("Usage: emulator [EXECUTABLE]\n");
	exit(EXIT_FAILURE);
}

#define ERROR(str)	{			\
	fprintf(stderr, str "\n");	\
	fclose(fp);					\
	return -1; 					\
}

int main(int argc, char* argv[])
{
	FILE *fp;
	struct exec header;

	if(argc != 2)
		die();

	fp = fopen(argv[1], "r");
	if(!fp) {
		perror("fopen");
		return -1;
	}

	header = (struct exec) {0};
	if(!fread(&header, A_MINHDR, 1, fp)) {
		perror("fread");
		fclose(fp);
		return -1;
	}

	if(header.a_hdrlen - A_MINHDR > 0) {
		if(!fread(&header, header.a_hdrlen - A_MINHDR, 1, fp)) {
			perror("fread");
			fclose(fp);
			return -1;
		}
	}

	if(BADMAG(header))
		ERROR("Invalid header");

	if(header.a_flags & ~(A_NSYM | A_EXEC | A_SEP))
		ERROR("Unsupported file");

	switch(header.a_cpu) {
	case A_I80386:
		printf("CPU ID: I386\n");
		return run_x86_emulator(argc-1, &argv[1], fp, header);
	case A_I8086:
		printf("Targeted CPU: I86\n");
		ERROR("Unsupported CPU");
	case A_M68K:
		printf("Targeted CPU: M68K\n");
		ERROR("Unsupported CPU");
	case A_NS16K:
		printf("Targeted CPU: NS16K\n");
		ERROR("Unsupported CPU");
	case A_SPARC:
		printf("CPU ID: SPARC\n");
		ERROR("Unsupported CPU");
	default:
		printf("CPU ID: Unknown\n");
		ERROR("Unsupported CPU");
	}

	return 0;
}
