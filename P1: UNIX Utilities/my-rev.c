// Copyright 2021 <Zhan Yu>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void exit_with_error() {
    printf("my-rev: invalid command line\n");
    exit(1);
}
void print_usage(char* argv[]) {
printf("-V: Prints information about this utility.\n");
printf("-h: Prints this help message.\n");
printf("-f <filename> Uses <filename> as the input dictionary\n");
printf(" to reverse the letters in each line of file.\n");
}
int main(int argc, char **argv) {
    int opt;
    FILE *fp;
    char buffer[256];
if (argc == 1) {
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            for (int i = strlen(buffer) - 1; i >= 0; i--) {
                if (buffer[i] != '\n') {
                   printf("%c", buffer[i]);
              }
        }
         if (buffer[strlen(buffer) - 1] == '\n') {
                   printf("\n");
}
}
        exit(0);
    }
    while ((opt = getopt(argc, argv, "Vhf:")) != -1) {
        switch (opt) {
        case 'V':
            printf("my-rev from CS537 Spring 2021\n");
            exit(0);
        case 'h':
            print_usage(argv);
            exit(0);
        case 'f':
            if (argc != 3) {
                exit_with_error();
            }
            fp = fopen(optarg, "r");
            if (fp == NULL) {
                printf("my-rev: cannot open file\n");
                exit(1);
            }
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            for (int i = strlen(buffer) - 1; i >= 0; i--) {
                if (buffer[i] != '\n') {
                   printf("%c", buffer[i]);
}
        }
         if (buffer[strlen(buffer) - 1] == '\n') {
                   printf("\n");
}
    }
            if (fclose(fp) != 0) {
                printf("my-rev: cannot close file\n");
                exit(1);
            }
            exit(0);
        case '?':
             exit_with_error();
        }
}
    exit_with_error();
}
