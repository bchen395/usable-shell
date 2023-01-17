#ifndef _NYUSH_H_
#define _NYUSH_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

char* readline(void){
    char *buffer;
    size_t size = 1000;
    buffer = (char *)malloc(size * sizeof(char));
    getline(&buffer,&size,stdin);
    return buffer;
}

char** parseline(char* line){
    char** args; 
    args = malloc(100*sizeof(char*));
    char* str = (char*)malloc(strlen(line)+1);
    strcpy(str,line);
    char * rest = NULL;
    char *delim = " \t\r\n";
    char *token;
    int i = 0;
    for (token = strtok_r(str, delim, &rest); token != NULL; token = strtok_r(NULL, delim, &rest)){
        args[i] = malloc(strlen(token)+1);
        strcpy(args[i],token); 
        i++;  
    }
    return args;
}

void exeamor(char **args) {
    pid_t pid; 
    pid_t wpid;
    pid = fork();
    if (pid == 0) {
    int in = 0;
    int out = 0;
    int app = 0;
    int pip = 0;
    char input[100];
    char output[100];
    char append[100];
    int i;
    for(i=0; args[i]!=NULL; i++)
    {
        if(strcmp(args[i],"<") == 0)
        {        
            args[i]=NULL;
            strcpy(input,args[i+1]);
            in=2;           
        }               
        else if(strcmp(args[i],">>") == 0)
        {      
            args[i]=NULL;
            strcpy(append,args[i+1]);
            app=2;
        }    
        else if(strcmp(args[i],">") == 0)
        {      
            args[i]=NULL;
            strcpy(output,args[i+1]);
            out=2;
        }
        else if(strcmp(args[i], "|") == 0)
        {
            args[i] = NULL;
            pip = 2;
        }
    }
    if(in)
    {   
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
            fprintf(stderr, "Error: invalid file\n");
            exit(0);
        }           
        dup2(fd0, 0); 
        close(fd0); 
    }
    if (out)
    {
        int fd1;
        if ((fd1 = open(output, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR)) < 0) {
            exit(0);
        }           
        dup2(fd1, STDOUT_FILENO); 
        close(fd1);
    }
    if (app)
    {
        int fd1;
        if ((fd1 = open(append, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR)) < 0) {
            fprintf(stderr, "Error\n");
            exit(1);
        }           

        dup2(fd1, STDOUT_FILENO); 
        close(fd1);
    }
    if (pip)
    {
        for(i=1; args[i]!=NULL; i++){
            int fds[2];
            pipe(fds);
            if (!fork()) {
                dup2(fds[1], 1);
                execlp(args[i], args[i], NULL);
                fprintf(stderr, "Error\n");
                exit(1);
            }
            dup2(fds[0], 0);
            close(fds[1]);
            execlp(args[i], args[i], NULL);
            fprintf(stderr, "Error\n");
            exit(1);
            }  
    }
    if (execvp(args[0], args) == -1) {
        fprintf(stderr, "Error: invalid program\n");
    }
    exit(1);
  } else if (pid < 0) {
    fprintf(stderr, "Error\n");
  } else {
    int status;
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

void shell_loop(void){
    while(1) {
        char cwd[1000];
        getcwd(cwd, sizeof(cwd));
        char ch = '/';
        char *base;
        base = strrchr(cwd, ch);
        if (strcmp(base, "/") == 0){
            printf("[nyush /]$ ");
        }
        else if (base[0] == '/'){
            base++;
            printf("[nyush %s]$ ", base);
        }
        fflush(stdout);
        char *line = readline();
        char **tokens = parseline(line); 
        if (strcmp(tokens[0], "cd") == 0){   
            if (tokens[1] == NULL || tokens[2] != NULL) {
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }  
            else{
                char* token = tokens[1];
                if (token[0] == '/'){
                    char* direct = tokens[1];
                    strcat(direct,"/");
                        if(chdir(direct) != 0){
                            fprintf(stderr, "Error: invalid directory\n");
                        }   
                    continue;
                }
                else{
                    char* direct = tokens[1]; 
                    char* base2 = cwd;
                    strcat(base2, "/");
                    strcat(base2, direct);
                    strcat(base2, "/");
                    if (chdir(base2) != 0){
                        fprintf(stderr, "Error: invalid directory\n");     
                    }
                    continue;
                }
                }
            }

        if (strcmp(tokens[0], "exit") == 0){
            if (tokens[1]!=NULL){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            break;
        }
        if (tokens[0] != NULL){
            exeamor(tokens); 
        }
    }
}
int main(int argc, char **argv){
    shell_loop();
}
#endif
