#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#define MAX_LINE_LENGTH 1024


char* recoverPath = "/playpen/a5/recover";
char* rankPath = "/playpen/a5/rank";
char* checkPath = "/playpen/a5/check";
char* modifyPath = "/playpen/a5/modify";



void printTitle(){
    //K.P: Title for the TetraShell
    printf("Welcome to...\n");
    printf("  ______     __             _____ __         ____\n");
    usleep(62500);
    printf(" /_  __/__  / /__________ _/ ___// /_  ___  / / /\n");
    usleep(62500);
    printf("  / / / _ \\/ __/ ___/ __ /\\__ \\/ __  \\/ _ \\/ / / \n");
    usleep(62500);
    printf(" / / /  __/ /_/ /  / /_/ /___/ / / / /  __/ / /  \n");
    usleep(62500);
    printf("/_/  \\___/\\__/_/   \\___//____/_/ /_/\\___/ _/_/   \n");
    usleep(62500);
    printf("\n");
}


int main(int argc, char** argv){
    
    //K.P: initializes the savePath and userInput memory. Initialize the tokens array to split the userInput into chunks.
    char *savePath = malloc(MAX_LINE_LENGTH);
    if (savePath == NULL) {
        perror("Unable to allocate memory for savePath");
        return 1;
    }
    char* userInput = malloc(MAX_LINE_LENGTH);
    if (userInput == NULL) {
        perror("Unable to allocate memory for userInput");
        return 1;
    }

    printTitle();
    printf("the ultimate Tetris quicksave hacking tool!\n");
    printf("Enter the path to the quicksave you'd like to begin hacking: ");
    fgets(savePath, MAX_LINE_LENGTH, stdin);
    //K.P: Remove the new line from the end of the input. 
        for (int i = 0; i < MAX_LINE_LENGTH; i++) {
            if (savePath[i] == '\n') {
                savePath[i] = '\0';
                break;
            }
        }
    printf("Enter your command below to get started: \n");
    while(true){
        char *tokens[MAX_LINE_LENGTH] = {0}; 
        int tokenCount = 0;
        printf("tetrashell> ");
        //K.P: Gets the userInput from stdin.
        fgets(userInput, MAX_LINE_LENGTH, stdin);
        //K.P: Remove the new line from the end of the input.
        for (int i = 0; i < MAX_LINE_LENGTH; i++) {
            if (userInput[i] == '\n') {
                userInput[i] = '\0';
                break;
            }
        }
        //K.P: Use strtok to split the userInput into tokens delimited by spaces.
        char *token = strtok(userInput, " ");
        while (token != NULL) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, " ");
        }

        tokens[tokenCount] = NULL;
        if((strcmp(tokens[0], "exit")) == 0){
            exit(1);
        }

        int st; 
        if(strcmp(tokens[0], "recover") == 0){
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                return 1;
            } else if (pid == 0){ 
                st = execve(recoverPath, tokens, NULL); 
                if(st == -1){
                    perror("execve");
                    exit(1); //K.P: Kill the child process
                    }
                }
            else{
                int status;
                waitpid(pid, &status, 0);
            }
        }

        if(strcmp(tokens[0], "check") == 0){
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                return 1;
            } else if (pid == 0){ 
                if(tokenCount != 1){
                    fprintf(stderr, "Error: too many arguments given. Only need one.\n");
                }
                char *checkArgs[] = {checkPath, savePath, NULL}; // Pass an array of arguments
                st = execve(checkPath, checkArgs, NULL); 
                if (st == -1){
                    perror("execve");
                    exit(1); //K.P: Kill the child process
                }
            }else{
                int status;
                waitpid(pid, &status, 0);
            }
        }
        if(strcmp(tokens[0], "modify") == 0){
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                return 1;
            }
            else if(pid == 0){
                if(tokenCount != 3){
                    fprintf(stderr, "Error: Modify needs 2 commands.\n");
                }
                char *modifyArgs[] = {modifyPath, tokens[1], tokens[2], savePath, NULL};
                st = execve(modifyPath, modifyArgs, NULL); 
                if (st == -1){
                    perror("execve");
                    exit(1); //K.P: Kill the child process
                }
            }
            else{
                int status; 
                waitpid(pid, &status, 0);
            }
        }
        if (strcmp(tokens[0], "rank") == 0) {
            if (tokenCount != 3) {
                fprintf(stderr, "Error: Rank needs 2 commands.\n");
            }
            //K.P: Create the working fds. Read and write end for the pipe.
            int fds[2];
            //K.P: Initialize the pipe
            if (pipe(fds) == -1) {
                perror("pipe");
                exit(1);
            }
            //K.P: Fork for the rank process.
            pid_t rank_pid = fork();
            
            if (rank_pid < 0) {
                perror("fork");
                exit(1);
            } else if (rank_pid == 0) {
                //K.P: Child process
                //K.P: Close the write end of the pipe
                close(fds[1]);
                //K.P: Redirect stdin to the read end of the pipe
                dup2(fds[0], STDIN_FILENO);
                char *rankArgs[] = {"rank", tokens[1], tokens[2], "uplink", NULL};
                st = execve(rankPath, rankArgs, NULL);
                if (st == -1)
                    perror("execve");
                    exit(1);
            } else {
                //K.P: Parent process
                //K.P: Close the read end of the pipe
                close(fds[0]);
                //K.P: Write savePath to the write end of the pipe
                write(fds[1], savePath, strlen(savePath));
                //K.P: Close the write end of the pipe
                close(fds[1]);
                //K.P: Wait for the rank process to finish
                int status;
                waitpid(rank_pid, &status, 0);
            }
        }
     }
 }

