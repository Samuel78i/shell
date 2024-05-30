#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define MAX_PATH 256

/* ----------- DEF -----------*/

// couleurs
#define COLOR_WHITE "\033[00m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_CYAN "\033[36m"
#define COLOR_RED "\033[91m"

// importance
#define IN_BG 0
#define IN_FG 1
#define PIPE 2

// Ã©tat du job
#define RUNNING 0
#define KILLED 1
#define DETACHED 2
#define STOPPED 3
#define DONE 4

// commande, externe puis interne
#define EXT 0
#define PWD 1
#define CD 2
#define EXCL 3
#define EXIT 4
#define JOBS 5
#define BG 6
#define FG 7
#define KILL 8

// mode de recherche pour jobs
#define ALL 0
#define TREE 1
#define SPEC 2
#define TREE_SPEC 3

// nombre de jobs
#define NBR 15

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int terminal;
int ongoing = 1;
pid_t shell_pgid;
struct jsh *shell;

int last_return;

struct job{
    int id;
    int status;
    int exec;
    char *command;
    pid_t pgid;
    struct process *root;
};

struct process{
    int status;
    int type;
    int fd_in;
    int fd_out;
    int fd_err;
    char** argv;
    pid_t pid;
    struct job *root;
    struct process *next;
};

struct jsh{
    char cur_dir[MAX_PATH];
    char pred_dir[MAX_PATH];
    char home_dir[MAX_PATH];
    struct job *jobs[NBR + 1];
};


void handler(int signo){}
