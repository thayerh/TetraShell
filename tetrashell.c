#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "tetris.h"
#define MAX_LINE_LENGTH 1024


char* recoverPath = "/playpen/a5/recover";
char* rankPath = "/playpen/a5/rank";
char* checkPath = "/playpen/a5/check";
char* modifyPath = "/playpen/a5/modify";


void print_title(int num_spaces);
char inputCheck(char *expected, char *input);
char *getFirstFour(const char *str);
void printBoard(TetrisGameState tGame, char* savePath);


int main(int argc, char** argv){
    char* userName = getlogin();
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


    //K.P allows for the animation of the logo. Starts from the right side (column 80) and reprints the title until end_col = col.
    int start_col = 40;
    int end_col = 0;

    for (int col = start_col; col >= end_col; --col) {
        print_title(col);
        usleep(10000);
    }

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

        //TH: Open quicksave for use throughout program
        FILE *file = fopen(savePath, "rb");
        if (file == NULL) {
                perror("fopen failed");
                exit(1);
        }

        TetrisGameState tGame;
        if ((fread(&tGame, sizeof(tGame), 1, file)==0)) {
                perror("fread failed");
                exit(1);
        }

        fclose(file);

        //TH: init array of previous modifies
        TetrisGameState *pastGames;
        int numPast = 0;
        int numAlloc = 0;
// Should try to validate quicksave

    printf("Enter your command below to get started: \n");
    while(true){

        char *tokens[MAX_LINE_LENGTH] = {0};
        int tokenCount = 0;
        printf("%s",userName);
        printf("@TShell");
        //K.P: Checks if terminal can support color. If so, prints the save file name in green.
        //TH: Also prints game save score and lines
        if(strcmp(getenv("TERM"), "xterm-256color") == 0){
            printf("\033[32m[%s][%u/%u]\033[0m> ", getFirstFour(savePath), tGame.score, tGame.lines);
        }
        else{
            printf("[%s][%u/%u]>", getFirstFour(savePath), tGame.score, tGame.lines);
        }


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
        if(inputCheck("exit", tokens[0])){
            exit(1);
        }

        int st;
        //TH: For easiest inputCheck impl, if first letter of input is r, need to differentiate between rank and recover
        if(tokens[0][0]=='r' && inputCheck("ecover", &tokens[0][1])){
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
        if(inputCheck("switch", tokens[0])){
            if(tokenCount != 2){
                fprintf(stderr, "Need new quicksave path.\n");
            }
            else {
                //K.P: copies new path into original buffer and then prints the switch.
                char oldPath[MAX_LINE_LENGTH];
                strncpy(oldPath, savePath, MAX_LINE_LENGTH);
                strncpy(savePath, tokens[1], MAX_LINE_LENGTH);
                savePath[MAX_LINE_LENGTH - 1] = '\0'; //K.P: Ensure null termination
                printf("Switch current quicksave from %s to %s.\n", oldPath, savePath);

                //TH: Change tGame to new safeFile
                file = fopen(savePath, "rb");
                if (file==NULL) {
                        perror("fopen failed");
                        exit(1);
                }
                if((fread(&tGame, sizeof(tGame), 1, file)==0)) {
                        perror("fread failed");
                        exit(1);
                }
                fclose(file);
            }
        }
        if(inputCheck("check", tokens[0])){
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

        if(inputCheck("modify", tokens[0])){
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
                //TH: Save previous tGame
                numPast++;
                if (numPast>numAlloc) {
                        pastGames = realloc(pastGames, 2 * numPast * sizeof(TetrisGameState));
                        numAlloc = 2 * numPast;
                }
                pastGames[numPast - 1] = tGame;
                //TH: Make tGame reflect modified game
                file = fopen(savePath, "rb");
                if (file==NULL) {
                        perror("fopen failed");
                        exit(1);
                }
                if (fread(&tGame, sizeof(tGame), 1, file)==0) {
                        perror("fread failed");
                        exit(1);
                }
                fclose(file);
            }
        }
        //TH: Special handling of rank check due to recover also starting with an 'r'
        if(tokens[0][0]=='r' && inputCheck("ank", &tokens[0][1])){
            if (tokenCount > 3) {
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
                //TH: If less than 3 args, autofill
                char *rankArgs[5];
                if (tokenCount == 1) {
                        char *rankArgs[] = {"rank", "score", "10", "uplink", NULL};
                } else if (tokenCount == 2) {
                        char *rankArgs[] = {"rank", tokens[1], "10", "uplink", NULL};
                } else {
                        char *rankArgs[] = {"rank", tokens[1], tokens[2], "uplink", NULL};
                }
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
        if (inputCheck("visualize", tokens[0])) {
                //TH: call printBoard function to print the board
                printBoard(tGame, savePath);
        }
        if (inputCheck("info", tokens[0])) {
                //TH: Print current file, score, lines
                printf("Current savefile: %s\n", savePath);
                printf("Score: %u\n", tGame.score);
                printf("Lines: %u\n", tGame.lines);
        }
        if (inputCheck("undo", tokens[0])) {
                //TH: If there is a previous file, use it
                if (numPast>0) {
                        tGame = pastGames[--numPast];
                        file = fopen(savePath, "w");
                        if (file==NULL) {
                                perror("fopen failed");
                                exit(1);
                        }
                        if (fwrite(&tGame, sizeof(TetrisGameState), 1, file)==0) {
                                perror("fwrite failed");
                                exit(1);
                        }
                        fclose(file);
                }
        }
    }
}


void print_title(int num_spaces) {
    printf("\033[2J\033[H"); //K.P: Clears the screen and move the cursor to the top-left corner
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf("Welcome to...\n");
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf("  ______     __             _____ __         ____\n");
    usleep(1200);
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf(" /_  __/__  / /__________ _/ ___// /_  ___  / / /\n");
    usleep(1200);
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf("  / / / _ \\/ __/ ___/ __ /\\__ \\/ __  \\/ _ \\/ / / \n");
    usleep(1200);
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf(" / / /  __/ /_/ /  / /_/ /___/ / / / /  __/ / /  \n");
    usleep(1200);
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf("/_/  \\___/\\__/_/   \\___//____/_/ /_/\\___/ _/_/   \n");
    usleep(1200);
}


char inputCheck(char *expected, char *input) {
        int i = 0;
        char nc = '\0';
        char valid = 1;
        //TH: Go through chars in expected and compare them to input if input still has chars left.
        //TH: If input runs out of chars early with at least the first char matching the first char
        //    of expected, break loop
        while (expected[i]!=nc && valid) {
                if (input[i]==nc) {
                        valid = (i==0) ? 0 : 1;
                        break;
                } else if (input[i]!=expected[i]) {
                        valid = 0;
                        break;
                }
                i++;
        }
        //TH: If loop ends when expected[i] is null char, need to make sure valid[i] is also null char
        if (expected[i]==nc) {
                valid = (expected[i]==nc && expected[i]==input[i] && valid) ? 1 : 0;
        }
        return valid;
}


//K.P: Gets the first four characters of the given save. if longer, it will be abbreviated.
char *getFirstFour(const char *str){
    static char firstFour[8];
    if (strlen(str) > 4) {
        strncpy(firstFour, str, 4); //K.P: Copy first 4 characters
        firstFour[4] = '.';
        firstFour[5] = '.';
        firstFour[6] = '.';
        firstFour[7] = '\0'; //K.P: Add the null terminator
        return firstFour;
    } else {
        strncpy(firstFour, str, 4);
        firstFour[4] = '\0';
        return firstFour;
    }
}


void printBoard(TetrisGameState tGame, char* savePath) {
        int i = 0;
        int m;
        printf("Visualizing savefile %s\n", savePath);
        printf("+---- Gameboard -----+   +--- Next ----+\n");
        for (int r = 0; r<20; r++) {
                putchar('|');
                for (int c = 0; c<10; c++) {
                        putchar(tGame.board[i]);
                        putchar(tGame.board[i++]);
                }
                putchar('|');
                if (r<7) {
                        if (r==0 || r==5) {
                                printf("   |             |");
                        } else if (r==6) {
                                printf("   +-------------+");
                        } else {
                                printf("   |  ");
                                for (m = 0; m<4; m++) {
                                        putchar(tetris_pieces[tGame.current_piece][((r-1)*4) + m]);
                                        putchar(tetris_pieces[tGame.current_piece][((r-1)*4) + m]);
                                }
                                printf("   |");
                        }
                }
                putchar('\n');
        }
        printf("+--------------------+\n");
}
