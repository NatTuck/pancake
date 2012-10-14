
#include <stdio.h>
#include <stdlib.h>

#include <pancake/parser.h>

int
main(int argc, char* argv)
{
    if (argc != 2) {
	printf("Usage: %s input.cl\n");
	exit(1);
    }

    printf("Would compile %s to LLVM assembly.\n");

    return 0;
}
