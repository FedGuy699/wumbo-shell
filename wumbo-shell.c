/*
    Copyright (C) 2026 FedGuy

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

#define MAX_ARGS 64
#define MAX_MATCHES 1024

char *builtin_cmds[] = { "cd", "exit", NULL };

static char *common_prefix(char *a, char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    char *prefix = strndup(a, i);
    return prefix;
}

char *wumbo_generator(const char *text, int state) {
    static char *matches[MAX_MATCHES];
    static int match_count = 0;
    static int match_index = 0;

    if (!state) {
        match_count = 0;
        match_index = 0;
        int len = strlen(text);

        for (int i = 0; builtin_cmds[i]; i++) {
            if (strncmp(builtin_cmds[i], text, len) == 0) {
                matches[match_count++] = strdup(builtin_cmds[i]);
            }
        }

        DIR *d = opendir(".");
        if (d) {
            struct dirent *entry;
            while ((entry = readdir(d)) && match_count < MAX_MATCHES) {
                if (strncmp(entry->d_name, text, len) == 0) {
                    matches[match_count++] = strdup(entry->d_name);
                }
            }
            closedir(d);
        }

        d = opendir("/usr/bin");
        if (d) {
            struct dirent *entry;
            while ((entry = readdir(d)) && match_count < MAX_MATCHES) {
                if (strncmp(entry->d_name, text, len) == 0) {
                    matches[match_count++] = strdup(entry->d_name);
                }
            }
            closedir(d);
        }
    }

    if (match_index < match_count) {
        return strdup(matches[match_index++]);
    }

    return NULL;
}

char **wumbo_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, wumbo_generator);
}

char *get_prompt() {
    static char prompt[PATH_MAX + 8];
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *dir = strrchr(cwd, '/');
        if (dir && *(dir + 1) != '\0') {
            snprintf(prompt, sizeof(prompt), "wumbo %s> ", dir + 1);
        } else {
            snprintf(prompt, sizeof(prompt), "wumbo /> ");
        }
    } else {
        snprintf(prompt, sizeof(prompt), "wumbo ?> ");
    }
    return prompt;
}

int main() {
    rl_attempted_completion_function = wumbo_completion;
    char *line;
    char *args[MAX_ARGS];

    while (1) {
        line = readline(get_prompt());
        if (!line) {
            printf("\n");
            break;
        }

        if (strlen(line) > 0)
            add_history(line);

        int argc = 0;
        char *token = strtok(line, " ");
        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) {
            free(line);
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            free(line);
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (argc < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else if (chdir(args[1]) != 0) {
                perror("cd");
            }
            free(line);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
        } else if (pid == 0) {
            if (execvp(args[0], args) < 0) {
                perror("exec failed");
                exit(1);
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
        }

        free(line);
    }

    return 0;
}