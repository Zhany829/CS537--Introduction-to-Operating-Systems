// Copyright 2021 <Zhan Yu>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void exit_with_error() {
    printf("my-look: invalid command line\n");
    exit(1);
}
void print_usage(char* argv[]) {
printf("-V: Prints information about this utility.\n");
printf("-h: Prints this help message.\n");
printf("-f <filename> <prefix> Uses <filename> as the input dictionary");
printf(" to search for lines starting with <prefix>.\n");
}
int main(int argc, char **argv) {
    int opt;
    FILE *fp;
    char *prefix;
    char buffer[256];
    if (argc < 2) {
    exit_with_error();
}
    while ((opt = getopt(argc, argv, "Vhf:")) != -1) {
        switch (opt) {
        case 'V':
            printf("my-look from CS537 Spring 2021\n");
            exit(0);
        case 'h':
            print_usage(argv);
            exit(0);
        case 'f':
            if (argc != 4) {
                exit_with_error();
            }
            fp = fopen(optarg, "r");
            if (fp == NULL) {
                printf("my-look: cannot open file\n");
                exit(1);
            }
            prefix = argv[3];
            while (fgets (buffer, sizeof(buffer), fp) != NULL) {
                if (strncasecmp(prefix, buffer, strlen(prefix)) == 0) {
                    printf("%s", buffer);
            }
            }
            if (fclose(fp) != 0) {
                printf("my-look: cannot close file\n");
                exit(1);
            }
            exit(0);
        case '?':
             exit_with_error();
        }
}
        if (argc == 2) {
            prefix = argv[1];
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (strncasecmp(prefix, buffer, strlen(prefix)) == 0) {
                printf("%s", buffer);
            }
        }
        exit(0);
    }
    exit_with_error();
}
