// Copyright 2021 ZHAN YU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
char prompt[6] = "mysh> ";
char* words[516];
static const char INCORRECT_NUM_ARG[] = "Usage: mysh [batch-file]\n";
static const char TOO_LONG_NUM_ARG[] = "Command line maximum length is 512\n";
int is_interactive = 0;
char redirect_error[105] = "Redirection misformatted.\n";
void print_prompt();
int split_line(char* line, char** words);
void alias_display();
int is_exist(char* alias);
void insertAlias(char *alias, char *cmd);
void removeAlias(char *alias);
char* get_value(char *alias);
void execute_alias_command(char *input);
void exe_loop(FILE *fp, char *input);
void cmd_execute(char *cmd[]);
void cmd_redirection_execute(char *cmd[], char *fileName[]);
struct Alias {
    char name[128];
    char value[512];
    struct Alias *next;
};
struct Alias *head = NULL;
struct Alias *cur = NULL;
struct Alias *pre = NULL;
void print_prompt() {
    write(STDOUT_FILENO, prompt, strlen(prompt));
}
int split_line(char* line, char** words) {
        int count = 0;
        char* input = strdup(line);
        while (1) {
                while (isspace(*input)) {
                    input++;
                }
                if (*input == '\0') {
                    return count;
                }
                words[count++] = input;
                while (!isspace(*input) && *input != '\0') {
                    input++;
                }
                if (*input == '\0') {
                    return count;
                }
                *input = '\0';
                input++;
        }
}
// display all alias
void alias_display() {
    struct Alias *ptr = head;
    // start from the beginning
    while (ptr != NULL) {
        fprintf(stdout, "%s %s\n", ptr->name, ptr->value);
        fflush(stdout);
        ptr = ptr->next;
    }
}
int is_exist(char* alias) {
    if (head == NULL) {
        return 0;
    }
}
// insert into the alias
void insertAlias(char *alias, char *val) {
    // first remove the same alias if it exists
    removeAlias(alias);
    struct Alias* new_alias = (struct Alias*) malloc(sizeof(struct Alias));
    strcpy(new_alias->name, alias);
    strcpy(new_alias->value, val);
    new_alias->next = NULL;
    // If head is empty, create new head
    if (head == NULL) {
        head = new_alias;
        return;
    }
    cur = head;
    // insert at the end
    while (cur->next != NULL) {
        cur = cur->next;
    }
    if (strcmp(cur->name, alias) == 0) {
            strcpy(cur->value, val);
            return;
    }
    cur->next = new_alias;
}

// remove from the alias
void removeAlias(char *alias) {
    if (!is_exist(alias)) {
        return;
    }
    if (strcmp(head->name, alias) == 0) {
        if (head->next != NULL) {
            head = head->next;
            return;
        } else {
            head = NULL;
            return;
        }
    }
    if (strcmp(head->name, alias) != 0 && head->next == NULL) {
        return;
    }
    cur = head;
    while (cur->next != NULL && strcmp(cur->name, alias) != 0) {
        pre = cur;
        cur = cur->next;
    }
    if (strcmp(cur->name, alias) == 0) {
        pre->next = pre->next->next;
        free(cur);
    } else {
        return;
    }
}

char* get_value(char *alias) {
    if (head == NULL) {
        // no match
        return NULL;
    }
    cur = head;
    while (cur->next != NULL) {
        if (strcmp(cur->name, alias) == 0) {
            return(cur->value);
        }
        cur = cur->next;
    }
    if (strcmp(cur->name, alias) == 0) {
        return(cur->value);
    }
    return NULL;
}

void execute_alias_command(char *input) {
    char long_arg[50] = "unalias: Incorrect number of arguments.\n";
    char* alias_line[515];
    char* get_count = strdup(input);
    int cnt = split_line(get_count, alias_line);
    // case 1: alias - display all alias
    if (cnt == 1 && strncmp(alias_line[0], "alias", strlen("alias")) == 0) {
        alias_display();
        return;
    }
    char danger[50] = "alias: Too dangerous to alias that.\n";
    // case 2: dangerous alias
    if (cnt > 1 && strncmp(alias_line[1], "alias", strlen("alias")) == 0) {
        write(STDERR_FILENO, danger, strlen(danger));
        return;
    } else if (cnt > 1 && strcmp(alias_line[1], "unalias") == 0) {
        write(STDERR_FILENO, danger, strlen(danger));
            return;
    } else if (cnt > 1 && strncmp(alias_line[1], "exit", strlen("exit")) == 0) {
            write(STDERR_FILENO, danger, strlen(danger));
            return;
    }
    // case 3: display single alias
    if (cnt == 2 && strncmp(alias_line[0], "alias", strlen("alias")) == 0) {
        if (get_value(alias_line[1])) {
            fprintf(stdout, "%s %s\n", alias_line[1], get_value(alias_line[1]));
            fflush(stdout);
            return;
        }
        return;
    }
    // case 4: unalias
    if (strncmp(alias_line[0], "unalias", strlen("unalias")) == 0) {
        // incorrect number of arguments
        if (cnt != 2) {
            write(STDERR_FILENO, long_arg, strlen(long_arg));
            return;
        }
        // execute unalias
        char name[128];
        strcpy(name, alias_line[1]);
        if (get_value(name) == NULL) {
            return;
        } else {
            removeAlias(name);
            return;
        }
    }
    // case 5: add alias
    char *visit = strtok(input, " \t");
    visit = strtok(NULL, " \t");  // get second argument
    visit = strtok(NULL, " \t");
    char newBuffer[512] = {0};
    while (visit != NULL) {
         strcat(newBuffer, visit);
         visit = strtok(NULL, " \t");
         if (visit != NULL) {
             strcat(newBuffer, " ");
         }
    }
    insertAlias(alias_line[1], newBuffer);
}
void exe_loop(FILE *fp, char *input) {
    while (fgets(input, 515, fp) != NULL) {
        if (!is_interactive) {
            write(STDOUT_FILENO, input, strlen(input));
        }
        if (strlen(input) > 512) {
            write(STDERR_FILENO, TOO_LONG_NUM_ARG, strlen(TOO_LONG_NUM_ARG));
            if (is_interactive) {
                print_prompt();
            }
                continue;
        }
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n')) {
            input[strlen(input) - 1] = '\0';
        }
        char* tmp = strdup(input);
        int num_word = split_line(tmp, words);
        // case: empty input
        if (num_word == 0) {
            if (is_interactive) {
                print_prompt();
            }
            continue;
        }
        if (strcmp("exit", words[0]) == 0 && num_word == 1) {
            exit(0);
        }
    if (strncmp(input, "alias", 5) == 0 || strncmp(input, "unalias", 7) == 0) {
            execute_alias_command(input);
            if (is_interactive) {
               print_prompt();
            }
            continue;
        }
        char* pre_token = strtok(tmp, ">");
        char* post_token = NULL;
        if (strlen(pre_token) == strlen(input)) {
            // no redirection
            if (get_value(input) != NULL) {
                strcpy(input, get_value(input));
                if (input[strlen(input) - 1] == '\n') {
                    input[strlen(input) - 1] = '\0';
                }
            }
            int i = 0;
            char *tok = strtok(input, " \t");
            char *cmd[10];
            while (tok != NULL) {
               cmd[i++] = tok;
               tok = strtok(NULL, " \t");
            }
            cmd[i] = NULL;
            cmd_execute(cmd);
        } else {  // redirction
            int num = 0;
            char *tok = strtok(input, ">");
            char *arr[2];
            while (tok != NULL) {
              if (num < 2) {
                 arr[num] = tok;
               }
               tok = strtok(NULL, ">");
               num += 1;
            }
            if (num != 2) {
                write(STDERR_FILENO, redirect_error, strlen(redirect_error));
                if (is_interactive) {
                   print_prompt();
                }
                continue;
            }
            if (arr[1] == NULL) {
                write(STDERR_FILENO, redirect_error, strlen(redirect_error));
                if (is_interactive) {
                    print_prompt();
                }
                continue;
            }
            int num_file = 0;
            char *tok1 = strtok(arr[1], " \t");
            char *fileName[10];
            while (tok1 != NULL) {
               fileName[num_file++] = tok1;
               tok1 = strtok(NULL, " \t");
            }
            if (num_file > 1) {
                write(STDERR_FILENO, redirect_error, strlen(redirect_error));
                if (is_interactive) {
                    print_prompt();
                }
                continue;
            }
            int j = 0;
            char *tok0 = strtok(arr[0], " \t");
            char *cmd_2[10];
            while (tok0 != NULL) {
               cmd_2[j] = tok0;
               tok0 = strtok(NULL, " \t");
               j++;
            }
            cmd_2[j] = NULL;
            cmd_redirection_execute(cmd_2, fileName);
        }
        // print shell prompt
        if (is_interactive) {
            print_prompt();
        }
    }
}

// executing normal commands
void cmd_execute(char *cmd[]) {
    pid_t pid = fork();
    if (pid == 0) {
        if (execv(cmd[0], cmd) != 0) {
            char error_info[50];
            sprintf(error_info, "%s: Command not found.\n", cmd[0]);
            write(STDERR_FILENO, error_info, strlen(error_info));
            _exit(1);
        }
    } else {
        wait(NULL);
    }
}

// execution redirction for the commends
void cmd_redirection_execute(char *cmd[], char *fileName[]) {
    pid_t pid = fork();
    if (pid == 0) {
       int fd = open(fileName[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
       if (fd < 0) {
           char err[100];
           strcpy(err, "Cannot write to file ");
           strcat(err, fileName[0]);
           strcat(err, ".\n");
           write(STDERR_FILENO, err, strlen(err));
       }
       dup2(fd, fileno(stdout));
       close(fd);
       if (execv(cmd[0], cmd) < 0) {
            char error_info[50];
            sprintf(error_info, "%s: Command not found.\n", cmd[0]);
            write(STDERR_FILENO, error_info, strlen(error_info));
            _exit(1);
       }
    } else {
          wait(NULL);
    }
}

int main(int argc, char* argv[]) {
    char input[512];
    FILE *input_file = NULL;
    // Interactive mode
    if (argc == 1) {
        is_interactive = 1;
        write(STDOUT_FILENO, prompt, strlen(prompt));
        input_file = stdin;
        exe_loop(input_file, input);
        exit(0);
     } else if (argc == 2) {  // batch mode
        is_interactive = 0;
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
           char open_error[256];
            sprintf(open_error, "Error: Cannot open file %s.\n", argv[1]);
            write(STDERR_FILENO, open_error, strlen(open_error));
            exit(1);
        }
       exe_loop(fp, input);
       exit(0);
    } else {
        // invalid arguments number
        write(STDERR_FILENO, INCORRECT_NUM_ARG, strlen(INCORRECT_NUM_ARG));
        exit(1);
    }
}



