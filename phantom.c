#include "phantom.h"

#include <string.h>

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (argc != 3 && argc != 2) {
        printf("Usage:\n");
        printf(" %s <input.md> <output.html> - convert single file\n", argv[0]);
        printf(" %s <input_directory> - convert all .md files in directory\n",
               argv[0]);
        return 1;
    }

    if (argc == 3) {
        if (convert_file(argv[1], argv[2])) {
            fprintf(stderr, "Failed converting %s to %s\n", argv[1], argv[2]);
            return 1;
        }
        printf("Converted %s to %s\n", argv[1], argv[2]);
    } else {
        printf("Directory processing - Work in Progress.\n");
    }

    return 0;
}
