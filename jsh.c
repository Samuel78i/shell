#include "jsh.h"
#include "aux.c"
#define _XOPEN_SOURCE 700

struct job* get_job_by_id(int id){
    if(id > NBR){ return NULL; }

    return shell->jobs[id];
}

int get_pgid_by_job_id(int id){
    struct job* job = get_job_by_id(id) ;

    if (job == NULL){
        return -1 ;
    }
    return job->pgid ;

}

int set_job_status(int id, int status){
    if(shell->jobs[id] == NULL){ return 1; }
    struct job *job = shell->jobs[id];
    struct process *proc;

    for(proc = job->root; proc; proc = proc->next){
        proc->status = status;
    }

    job->status = status;

    return 0;
}

int free_job(int id){
    if(id > NBR || shell->jobs[id] == NULL){ return 1; }

    struct job *job = shell->jobs[id];
    struct process *proc, *temp;
    for(proc = job->root; proc; ){
        temp = proc->next;
        free(proc);
        proc = temp;
    }

    free(job->command);
    free(job);
    shell->jobs[id] = NULL;

    return 0;
}


int remove_job(int id) {
    if (id > NBR || shell->jobs[id] == NULL) {
        return -1;
    }

    free_job(id);
    shell->jobs[id] = NULL;

    return 0;
}



struct process* get_proc_by_pid(pid_t pid){
    struct process *proc;

    for(int i = 0; i < NBR; i++){
        for(proc = shell->jobs[i]->root; proc; proc = proc->next){
            if(proc->pid == pid){
                return proc;
            }
        }
    }

    return NULL;
}

struct job* get_job_by_pid(pid_t pid){
    struct job *job;

    for(int i = 1; i < NBR; i++){
        job = shell->jobs[i];
        if(job != NULL){
            if(job->pgid == pid){
                return job;
            }
        }
    }

    return NULL;
}

int wait_for_pid(int pid) {
    int status = 0;
    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_job_status(pid, DONE);
    } else if (WIFSIGNALED(status)) {
        set_job_status(pid, KILLED);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_job_status(pid, STOPPED);
    }

    return status;
}

int get_next_job(){
    for(int i = 1; i <= NBR; i++){
        if(shell->jobs[i] == NULL){
            return i;
        }
    }
    return -1;
}

int nbr_proc(int id){
    int n = 0;
    struct process *proc;
    for(proc = shell->jobs[id]->root; proc; proc = proc->next){
        n++;
    }
    return n;
}

int nbr_job(){
    int nombre = 0;
    for(int i = 0; i < NBR; i++){
        if(shell->jobs[i] != NULL && shell->jobs[i]->status != DONE){ nombre++; }
    }
    return nombre;
}


void free_jsh(){
    for(int i = 0; i < NBR; i++){
        free_job(i);
    }
}

int add_job(struct job *job){
    int id = get_next_job();

    if(id < 0){ return -1; }

    job->id = id;
    shell->jobs[id] = job;
    return id;
}

int check_job_status(int id){
    if(id > NBR || shell->jobs[id] == NULL){ return 1; }

    struct process *proc;
    for(proc = shell->jobs[id]->root; proc; proc = proc->next){
        if(proc->status != DONE){ return 1; }
    }

    return 0;
}
int get_job_status (int id){
    if(id > NBR || shell->jobs[id] == NULL){ return 1; }

    struct process *proc;
    for(proc = shell->jobs[id]->root; proc; proc = proc->next){
        if(proc->status == KILLED){ return 1 ;}
        else if(proc->status == STOPPED){  return 3 ;}
        else if(proc->status == DONE){ return 4; }
    }

    return 0;
}

void check_jobs(){
    struct job *job;
    for(int i = 0; i < NBR; i++){
        if(check_job_status(i) == 0){
            job = shell->jobs[i];
            job->status = DONE;
        }
    }
}



int set_proc_status(int pid, int status){
    struct process *proc;

    for(int i = 1; i <= NBR; i++){
        if(shell->jobs[i] == NULL){
            continue;
        }

        for (proc = shell->jobs[i]->root; proc; proc = proc->next) {
            if (proc->pid == pid) {
                proc->status = status;
                return 1;
            }
        }
    }

    return 0;
}

void check_zombie(){
    int status, pid;
    struct job *job;
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
        job = get_job_by_pid(pid);
        if(WIFEXITED(status)) {
            set_job_status(job->id, DONE);
            fprintf(stderr, "[%d] %d Done %s\n", job->id, job->pgid, job->command);
        } else if (WIFSTOPPED(status)) {
            status = -1;
            set_job_status(job->id, STOPPED);
            fprintf(stderr, "[%d] %d Stopped %s\n", job->id, job->pgid, job->command);
        } else if (WIFCONTINUED(status)){
            set_job_status(job->id, RUNNING);
            fprintf(stderr, "[%d] %d Running %s\n", job->id, job->pgid, job->command);
        } else if (WIFSIGNALED(status)) {
            if((WTERMSIG(status) == SIGKILL || WTERMSIG(status) == SIGTERM)){
                set_job_status(job->id, KILLED);
            }
        }
    }
}

int wait_for_job(int id) {
    if (id > NBR || shell->jobs[id] == NULL) {
        return -1;
    }

    int proc_count = nbr_proc(id);
    int wait_pid = -1, wait_count = 0;
    int status = 0;

    do {
        wait_pid = waitpid(-shell->jobs[id]->pgid, &status, WUNTRACED);
        wait_count++;
        if(wait_pid != -1){
            if(WIFEXITED(status)) {
                set_job_status(id, DONE);
            } else if (WIFSIGNALED(status)) {
                if((WTERMSIG(status) == SIGKILL || WTERMSIG(status) == SIGTERM)){
                    set_job_status(id, KILLED);
                }
            } else if (WIFSTOPPED(status)) {
                status = -1;
                set_job_status(id, STOPPED);
                fprintf(stderr, "\n[%d] %d Stopped %s\n", shell->jobs[id]->id, shell->jobs[id]->pgid, shell->jobs[id]->command);
            }
        }

    } while (wait_count < proc_count);

    return WEXITSTATUS(status);
}

int my_waitpid(int pid){
    int status = 0;
    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_proc_status(pid, DONE);
    } else if (WIFSIGNALED(status)) {
        int term_signal = WTERMSIG(status);
        if (term_signal == 30 || term_signal == 31) {
            return 0;
        }
        set_proc_status(pid, KILLED);

    } else if (WSTOPSIG(status)) {
        status = 1;
        set_proc_status(pid, STOPPED);
    }

    return WEXITSTATUS(status);
}

//////////////////// NOS COMMANDES ///////////////////////

void update_cur() {
    char* path = malloc(sizeof(char) * MAX_PATH);

    if (path == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour path");
        exit(EXIT_FAILURE);
    }

    if(getcwd(path, MAX_PATH) == 0){
        perror("Erreur lors de la récupération du répertoire courant");
        free(path);
        exit(EXIT_FAILURE);

    }

    if (shell == NULL) {
        fprintf(stderr, "Erreur : shell est NULL\n");
        free(path);
        exit(EXIT_FAILURE);
    }

    strcpy(shell->pred_dir, shell->cur_dir);
    strcpy(shell->cur_dir, path);

    free(path);
}

int my_pwd(int fd_out){
    update_cur();
    char* path = malloc(sizeof(char) * MAX_PATH);
    strcpy(path, shell->cur_dir);
    strcat(path, "\n");

    write(fd_out, path, strlen(shell->cur_dir) +1);
    free(path) ;
    return 0;
}

int cd(int argc, char** argv){
    if(argc == 1){
        chdir(shell->home_dir);
        update_cur();
        return 0;
    }

    if(strcmp("-", argv[1]) == 0){
        chdir(shell->pred_dir);
        update_cur();
        return 0;
    }

    if(chdir(argv[1]) == 0){
        update_cur();
        return 0;
    }else{
        printf(COLOR_BLUE"cd: %s: No such file or directory\n", argv[1]);
        return 1;
    }
}

int excl(){
    printf("%d\n", last_return);

    return 0;
}

int my_exit(int val, struct process* proc){
    proc->status = DONE;
    check_jobs();

    for(int i = 0; i < NBR; i++){
        if(shell->jobs[i] == NULL){
            continue;
        }else if(shell->jobs[i]->status == STOPPED){
            fprintf(stderr, "exit\nThere are stopped jobs.\n");
            return 1;
        }else if(shell->jobs[i]->status == RUNNING){
            fprintf(stderr, "exit\nThere are running jobs.\n");
            return 1;
        }
    }

    ongoing = false;
    exit(val);
}

int my_exit_bis(FILE *fd_in){
    if(feof(fd_in)){ return last_return; }

    return -1;
}

int our_jobs(struct process *proc){
    check_jobs();

    for(int i = 1; i < NBR; i++){
        if(shell->jobs[i] != NULL && strcmp(shell->jobs[i]->command, "jobs") != 0 ){
            if(shell->jobs[i]->status == DONE){
                printf("[%d] %d Done %s\n", i, shell->jobs[i]->pgid, shell->jobs[i]->command);
                free_job(i);
            }else if(shell->jobs[i]->status == RUNNING){
                printf("[%d] %d Running %s\n", i, shell->jobs[i]->pgid, shell->jobs[i]->command);
            }else if(shell->jobs[i]->status == STOPPED){
                printf("[%d] %d Stopped %s\n", i, shell->jobs[i]->pgid, shell->jobs[i]->command);
            }else if(shell->jobs[i]->status == KILLED){
                printf("[%d] %d Killed %s\n", i, shell->jobs[i]->pgid, shell->jobs[i]->command);
                free_job(i);
            }
        }
    }

    return 0;
}

void bg_job(int id){
    struct job *job;
    job = shell->jobs[id];
    job->status = RUNNING ;
    fprintf(stderr, "[%d] %d Running %s\n", id, job->pgid, job->command);
}

void done_jobs(){
    struct job *job;
    for(int i = 1; i < NBR; i++){
        job = shell->jobs[i];
        if(job != NULL){
            if ((job->status == DONE && job->exec == IN_BG) || ( job->status == DONE )) {
                free_job(i);
            }else if(job->status == KILLED){
                fprintf(stderr, "[%d] %d Killed %s\n", i, job->pgid, job->command);
                free_job(i);
            }
        }
    }
}


int kill_by_sig(int sig, int id, int mode){
    struct job* job = shell->jobs[id];
    struct process* proc;

    if(mode == 1){
        job = get_job_by_id(id);
        kill(-(job->pgid), sig);
    }else{
        proc = get_proc_by_pid(id);
        kill(proc->pid, sig);
    }
    return 0;
}

int my_kill(int argc, char** argv){
    int mode = 0, id;
    if(argv[1][0] == '%' || (argc == 3 && argv[2][0] == '%')){ mode = 1; }

    if(argc == 2){
        id = atoi(argv[1] + mode);
        kill_by_sig(SIGTERM, id, mode);
        return 0;
    }else if(argc == 3){
        id = atoi(argv[2] + mode);
        kill_by_sig(atoi(argv[1] + 1), id, mode);
        return 0;
    }

    return 1;
}

int fg(int argc, char **argv) {
    if (argc < 2) {
        printf("Il manque des arguments\n");
        return -1;
    }

    pid_t pid;
    int job_id = -1;

    if (argv[1][0] == '%') {
        job_id = atoi(argv[1] + 1);
        pid = get_pgid_by_job_id(job_id);
        if (pid < 0) {
            printf("fg %s: le job n'existe pas\n", argv[1]);
            return -1;
        }
    } else {
        pid = atoi(argv[1]);
    }
    if (kill(-pid, SIGCONT) < 0 ){
        printf("fg %d: job non trouvé\n", pid);
        return -1;
    }
    tcsetpgrp(0, pid) ;

    if (job_id > 0) {
        set_job_status(job_id, RUNNING);
        if (wait_for_job(job_id) < 0) {
            wait_for_pid(pid);
        }
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(0, getpid());
    signal(SIGTTOU, SIG_DFL);

    return 0;
}

int bg(int argc, char **argv) {
    if (argc < 2) {
        printf("Il manque des arguments \n");
        return -1;
    }

    pid_t pid;
    int job_id = -1;

    if (argv[1][0] == '%') {
        job_id = atoi(argv[1] + 1);
        pid = get_pgid_by_job_id(job_id);
        if (pid < 0) {
            printf("bg %s: le job n'existe pas\n", argv[1]);
            return -1;
        }
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(-pid, SIGCONT) < 0) {
        printf("bg %d: job non trouvé\n", pid);
        return -1;
    }

    if (job_id > 0) {
        set_job_status(job_id, RUNNING);
    }

    tcsetpgrp(0, getpid());

    return 0;
}


int our_commands(struct process* proc, int fd_out){
    int size = size_len(proc->argv);
    if(size == 0){ return 1; }

    switch(proc->type){
        case PWD:
            return my_pwd(fd_out);
        case CD:
            return cd(size, proc->argv);
        case EXCL:
            return excl();
        case EXIT:
            if(size == 2) {
                return my_exit(atoi(proc->argv[1]), proc);
            }
            else if(size == 1){ return my_exit(last_return, proc); }
            else{ perror("Un soucis avec le nombre d'arguments pour exit.\n"); return 1; }
            break;
        case JOBS:
            return our_jobs(proc);
        case BG:
            return bg(size, proc->argv);
        case FG:
            return fg(size, proc->argv);
        case KILL:
            return my_kill(size, proc->argv);
        default:
            return 1;
    }
}

int type_command(char** commands){
    return(key_from_str(commands[0]));
}

int open_file(char* file_name, int flags){
    int fd = open(file_name, flags, 0666);
    if (fd < 0) {
        perror("");
    }
    return fd;
}

///////////////// Exécution ///////////////

int built_in_command(struct process *proc, int fd_in, int fd_out, int fd_err){
    int status = 0;
    int entree, sortie, erreur;
    entree = dup(0);
    sortie = dup(1);
    erreur = dup(2);

    if (fd_in != 0) {
        dup2(fd_in, 0);
        close(fd_in);
    }


    if(fd_err != 2){
        dup2(fd_err, 2);
        close(fd_err);
    }

    proc->status = DONE;
    status = our_commands(proc, fd_out);

    dup2(entree, 0);
    dup2(sortie, 1);
    dup2(erreur ,2);
    close(entree); close(sortie); close(sortie);

    return status;
}

int launch_proc(struct job *job, struct process *proc, int fd_in, int fd_out, int fd_err, int mode) {
    proc->status = RUNNING;
    pid_t pid;
    int status = 0;

    if (proc->type != EXT) {
        return built_in_command(proc, fd_in, fd_out, fd_err);
    }

    pid = fork();

    if (pid < 0) {
        return -1;
    } else if (pid == 0) {

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        proc->pid = getpid();
        if (job->pgid > 0) {
            setpgid(0, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(0, job->pgid);
        }

        if (fd_in != 0) {
            dup2(fd_in, 0);
            close(fd_in);
        }

        if (fd_out != 1) {
            dup2(fd_out, 1);
            close(fd_out);
        }

        if(fd_err != 2){
            dup2(fd_err, 2);
            close(fd_err);
        }

        if(fd_in == -1 || fd_out == -1 || fd_err == -1){
            exit(1);
        }

        if (execvp(proc->argv[0], proc->argv) < 0) {
            perror("execvp");
            exit(1);
        }
        exit(0);
    } else {
        proc->pid = pid;
        if (job->pgid > 0) {
            setpgid(pid, job->pgid);
        } else {
            job->pgid = proc->pid;
            setpgid(pid, job->pgid);
        }

        if (mode == IN_FG) {
            tcsetpgrp(0, job->pgid);
            status = wait_for_job(job->id);
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            signal(SIGTTOU, SIG_DFL);
        }
    }

    return status;
}

int start_job(struct job *job) {
    int status = 0, fd[2], in_fd = 0;
    struct process *proc = job->root;

    add_job(job);

    for (proc = job->root; proc != NULL; proc = proc->next) {
        if(proc == job->root){
            in_fd = proc->fd_in;
        }
        if (proc->next != NULL) {
            pipe(fd);
            status = launch_proc(job, proc, in_fd, fd[1], proc->fd_err, 2);
            close(fd[1]);
            in_fd = fd[0];
        } else {
            int out_fd = 1;
            if(proc->fd_out != 1){
                out_fd = proc->fd_out;
            }
            status = launch_proc(job, proc, in_fd, out_fd, proc->fd_err, job->exec);
        }
    }

    if(job->exec == IN_BG){ bg_job(job->id); }

    return status;
}

struct process* create_process(char** command, int fd_in, int fd_out, int fd_err) {
    struct process* new_proc = (struct process*) malloc(sizeof(struct process));
    new_proc->status = RUNNING;
    new_proc->type = type_command(command);
    new_proc->argv = command;
    new_proc->pid = -1;
    new_proc->fd_in = fd_in;
    new_proc->fd_out = fd_out;
    new_proc->fd_err = fd_err;
    new_proc->next = NULL;

    return new_proc;
}

struct process* start_proc(char* command){
    char** commands = to_chars(command, " ");
    int fd_in = 0, fd_out = 1, fd_err = 2;
    int n = size_len(commands);

    for (int i = 0; i < n-1; i++) {
        if (strcmp(commands[i], "<") == 0) {
            fd_in = open_file(commands[i + 1], O_RDONLY);
        } else if (strcmp(commands[i], ">") == 0) {
            fd_out = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_EXCL);
        } else if (strcmp(commands[i], ">>") == 0) {
            fd_out = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_APPEND);
        } else if (strcmp(commands[i], ">|") == 0) {
            fd_out = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_TRUNC);
        }else if (strcmp(commands[i], "2>") == 0) {
            fd_err = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_EXCL);
        }else if (strcmp(commands[i], "2>>") == 0) {
            fd_err = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_APPEND);
        }else if (strcmp(commands[i], "2>|") == 0) {
            fd_err = open_file(commands[i + 1], O_WRONLY | O_CREAT | O_TRUNC);
        }
    }

    if(fd_in < 0 || fd_out < 0 || fd_err < 0){
        return NULL;
    }

    return create_process(trim(commands), fd_in, fd_out, fd_err);
}

struct job* setup_job(char* line){
    char* command = strdup(line);
    struct process *root = NULL;
    int exec = IN_FG;
    if(command[strlen(command)-1] == '&'){
        exec = IN_BG;
        command[strlen(command)-1] = '\0';
    }
    rtrim(command);

    if(aPipeIsPresent(command)){
        char** commands = to_chars(command, "|");
        struct process* new_proc = start_proc(commands[0]);
        if(new_proc == NULL){
            return NULL;
        }
        root = new_proc;

        for(int i = 1 ; commands[i] != NULL; i++){
            struct process* temp = start_proc(commands[i]);
            new_proc->next = temp;
            new_proc = temp;
        }
    }else if (substitution(command)) {
        int pipefd[2], pid;
        pipe(pipefd);
        char **temp = to_chars(command, "<(");
        char *new_command[1024];
        new_command[0] = temp[0];
        removeLastClosingParenthesis(temp[1]);
        char **toExec = to_chars(temp[1], " ");
        pid = fork();
        if(pid == -1){
            perror("fork");
            exit(1);
        }else if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            if (execvp(toExec[0], toExec) < 0) {
                perror("execvp");
                exit(1);
            }
        } else{
            close(pipefd[1]);
            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
                strncpy(new_command[1], buffer, bytesRead - 1);
            }
            strcat(new_command[1], "\0");
            struct process *new_proc = create_process(new_command, 0, 1, 2);
            root = new_proc;
            waitpid(pid, NULL, 0);
        }
    }else{
        struct process* new_proc = start_proc(command);
        if(new_proc == NULL){
            return NULL;
        }
        root = new_proc;
    }

    struct job *new_job = (struct job*) malloc(sizeof(struct job));
    new_job->root = root;
    new_job->command = command;
    new_job->pgid = -1;
    new_job->exec = exec;

    return new_job;
}

void format(char *typed){
    char* path = malloc(sizeof(char) * MAX_PATH);
    if (path == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour path");
        exit(EXIT_FAILURE);
    }
    int job_count = nbr_job();
    strcpy(path, shell->cur_dir);
    sprintf(typed, COLOR_BLUE "[%d]", job_count);
    if (strlen(path) + strlen(typed) - 5 > 30) {
        path = &(path[strlen(path) + strlen(typed) - 5 - 25]);
        strcat(typed, COLOR_BLUE);
        strcat(typed, "...");
    }

    strcat(typed, COLOR_BLUE);
    strcat(typed, path);
    strcat(typed, COLOR_WHITE);
    strcat(typed, "\001\033[00m\002$ ");
}

void handle_sigterm(int sig){
// Ne rien faire , simplement ignorer le signal
    (void)sig ;
}

void shell_loop(){
    char* line;
    char* typed = malloc(sizeof(char) * MAX_PATH);
    struct job* job;
    last_return = 0;

    while(ongoing){
        if(signal(SIGTERM,handle_sigterm) == SIG_ERR){
            perror("Erreur lors de l'enregistrement de la gestion du signal SIGTERM") ;
            exit(EXIT_FAILURE) ;
        }
        format(typed);
        line = readline(typed);
        if(line != NULL){
            if(strlen(line) > 0){
                add_history(line);
                job = setup_job(line);

                if(job != NULL) {
                    last_return = start_job(job);
                    check_jobs();
                    check_zombie();

                    if (job->root->type == EXT) {
                        done_jobs();
                    }
                }else {
                    last_return = 1;
                }
            }
            free(line);
        }else{
            exit(last_return);
        }
    }

    free(typed);
    exit(0);
}

void shell_init(){
    struct sigaction sa = {0};
    sa.sa_handler = handler;

    char *home;
    ongoing = true;
    shell = (struct jsh*)malloc(sizeof(struct jsh));
    update_cur();

    if((home = getenv("HOME")) == NULL) {
        home = getpwuid(getuid())->pw_dir;
    }

    strcpy(shell->home_dir, home);
    rl_outstream = stderr;

    pid_t pid = getpid();
    setpgid(pid, pid);
    tcsetpgrp(0, pid);
}

int main(){
    shell_init();
    shell_loop();

    return EXIT_SUCCESS;
}