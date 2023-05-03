// I, Kishan Patel (730477803), pledge that I have neither given nor received unauthorized aid on this assignment.
// I, Thayer Hicks (730475597), pledge that I have neither given nor received unauthorized aid on this assignment.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "tetris.h"

#define MAX_LINE_LENGTH 1024


char* recoverPath = "/playpen/a5/recover";
char* rankPath = "/playpen/a5/rank";
char* checkPath = "/playpen/a5/check";
char* modifyPath = "/playpen/a5/modify";


void print_title(int num_spaces);
char inputCheck(char *expected, char *input);
char *getFirstFour(const char *str);
char *switchFile(char* savePath, char* newPath);
void printBoard(TetrisGameState tGame, char* savePath);
bool vailidateSave(char* savePath);
void printRank(int rankNum, char* fileName, char isBold);
void printRec(int recNum, char* fileName, unsigned score, unsigned lines);
void train();
char* readInput();
bool checkExit(const char *input);
char* intToBinary(int integer);




int main(int argc, char** argv){

    struct winsize ws;

    // Get the terminal window size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

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
    int start_col = ws.ws_col / 3;
    int end_col = 0;

    for (int col = start_col; col >= end_col; --col) {
        if(col == end_col){
            system("clear");
        }
        print_title(col);
        usleep(10000);
    }

    printf("the ultimate Tetris quicksave hacking tool!\n");
    printf("Type 'help' for more info after entering a quicksave path.\n");
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




    printf("Enter your command below to get started: \n");
    while(true){
        char *tokens[MAX_LINE_LENGTH] = {0};
        int tokenCount = 0;
        printf("%s",userName);
        printf("@TShell");
        //K.P: Checks if terminal can support color. If so, prints the save file name in green.
        //T.H: Also prints game save score and lines
        bool saveIsValid = vailidateSave(savePath);
        if (strcmp(getenv("TERM"), "xterm-256color") == 0) {
            if (saveIsValid) {
                //K.P: Print in green if save is valid
                printf("\033[32m[%s][%u/%u]\033[0m> ", getFirstFour(savePath), tGame.score, tGame.lines);
            } else {
                //K.P: Print in red if save is not valid
                printf("\033[31m[%s][%u/%u]\033[0m> ", getFirstFour(savePath), tGame.score, tGame.lines);
            }
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
                int fdsRec[2];
                int fdsRecU[2];
                if (pipe(fdsRec) == -1 || pipe(fdsRecU) == -1) {
                        perror("pipe");
                        exit(1);
                }

                pid_t pid = fork();

                if (pid < 0) {
                        perror("fork");
                        return 1;
                } else if (pid == 0){
                //TH: Close read end of pipe
                        close(fdsRec[0]);
                        close(fdsRecU[0]);
                        //TH: Redirect stdout of recover to pipe
                        dup2(fdsRec[1], STDOUT_FILENO);
                        dup2(fdsRecU[1], STDERR_FILENO);

                        st = execve(recoverPath, tokens, NULL);
                        if(st == -1){
                            perror("execve");
                            exit(1); //K.P: Kill the child process
                        }
                        close(fdsRec[1]);
                        close(fdsRecU[1]);
                } else {
                        int status;
                        waitpid(pid, &status, 0);

                        close(fdsRec[1]);
                        close(fdsRecU[0]);
                        close(fdsRecU[0]);

                        FILE* recoverFile = fdopen(fdsRec[0], "r");
                        if (recoverFile == NULL) {
                                perror("fdopen");
                                exit(1);
                        }

                        char** recNames = malloc(sizeof(char*));
                        int arrSz = 1;
                        char* cur;

                        int recNum = 0;
                        char recLine[MAX_LINE_LENGTH];
                        FILE* rf;
                        TetrisGameState game;

                        printf("Recovered quicksaves:\n");
                        printf("-- ------------------------------ ------- -------\n");
                        printf("#  File Path                      Score   Lines  \n");
                        printf("-- ------------------------------ ------- -------\n");

                        while (fgets(recLine, MAX_LINE_LENGTH, recoverFile)) {
                                recLine[strcspn(recLine, "\n")] = '\0';

                                rf = fopen(recLine, "r");
                                if (rf==NULL) {
                                        perror("fopen");
                                        exit(1);
                                }
                                if (fread(&game, sizeof(game), 1, rf)==0) {
                                        perror("fread");
                                        exit(1);
                                }
                                fclose(rf);

                                if (strlen(recLine) >= 28) {
                                        recLine[47] = '\0';
                                        strcat(recLine, "...");
                                }

                                if (recNum >= arrSz) {
                                        recNames = realloc(recNames, 2 * arrSz * sizeof(char*));
                                        arrSz = 2 * arrSz;
                                }

                                cur = malloc(MAX_LINE_LENGTH);
                                strcpy(cur, recLine);
                                recNames[recNum] = cur;

                                printRec(++recNum, recLine, game.score, game.lines);

                        }
                        close(fdsRec[0]);

                        printf("Would you like to switch to one of these (y/n): ");
                        char response[MAX_LINE_LENGTH];
                        fgets(response, MAX_LINE_LENGTH, stdin);
                        if (strcmp(response, "n\n")==0) {
                                continue;
                        } else if (strcmp(response, "y\n")==0) {
                                printf("Which quicksave (enter a # 1-%d): ", recNum);
                                fgets(response, MAX_LINE_LENGTH, stdin);
                                response[strcspn(response, "\n")] = '\0';
                                int filen = atoi(response);
                                if (filen < 1 || filen > recNum) {
                                        printf("Invalid quicksave number");
                                }
                                printf("Done! ");
                                savePath = switchFile(savePath, recNames[filen-1]);

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
        }

        if(inputCheck("help", tokens[0])) {
            if(tokens[1] != NULL){
                if(inputCheck("check", tokens[1])) {
                    printf("This command calls the `check` program with the current "
                        "quicksave to verify if it will pass legitimacy checks."
                            "can input 'c', 'ch', etc.\n");
                }
                if(inputCheck("rank", tokens[1])) {
                    printf("Rank the current quicksave with a database of other saves. "
                        "Input (Rank or 'r', 'ra', etc.) (Score or Lines) and number"
                            "of lines to return. Can just input"
                                " 'rank' and will default to ten lines and sort by score. \n");
                }
                if(inputCheck("modify", tokens[1])) {
                    printf("Modifies the current save. Input"
                        "(Modify or 'm', 'mo', etc.) (Score or Lines) "
                            "(Number to set value to)\n");
                }
                if(inputCheck("switch", tokens[1])) {
                    printf("Switches the current save to the one you input. Input (Switch) (Save path)\n");
                }
                if(inputCheck("info", tokens[1])) {
                    printf("Prints the info of the given save. Includes scores, lines, and validity.\n");
                }
                if(inputCheck("visualize", tokens[1])) {
                    printf("Prints the visual description of the given save.\n");
                }
                if(inputCheck("undo", tokens[1])) {
                    printf("Undoes the last modify action.\n");
                }
                if(inputCheck("train", tokens[1])) {
                    printf("Fun game to test hex to binary to"
                        " integer conversions.\n");
                }
            }
            else{
                printf("type 'help' followed by action name for more information. "
                    "(check, rank, modify, switch, info, undo, visualize, train)\n");
            }
        }


        if(inputCheck("switch", tokens[0])){
            if(tokenCount != 2){
                fprintf(stderr, "Please enter new quicksave path.\n");
            }
            else {
                //K.P: copies new path into original buffer and then prints the switch.
                savePath = switchFile(savePath, tokens[1]);

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
                    fprintf(stderr, "Error: check takes no extra arguments\n");
                }
                char *checkArgs[] = {checkPath, savePath, NULL};
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
                    fprintf(stderr, "Error: Modify needs 2 commands."
                         "(either score or lines) (number to change value to).\n");
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
        //TH: Special handling of rank check due to recover also starting with an 'r'
        if(tokens[0][0]=='r' && inputCheck("ank", &tokens[0][1])){
            if (tokenCount < 1) {
                fprintf(stderr, "Error: Rank needs 1 commands at minimum. ('rank'). Can also provide"
                    "either score or lines and number rankings to return. ('rank score 100') \n");
            }
            //K.P: Create the working fds. Read and write end for the pipe.
            int fdsInit[2];
            int fdsFin[2];
            //K.P: Initialize the pipe
            if ((pipe(fdsInit) == -1) || (pipe(fdsFin) == -1)) {
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
                close(fdsInit[1]);
                close(fdsFin[0]);
                //K.P: Redirect stdin to the read end of the pipe
                dup2(fdsInit[0], STDIN_FILENO);
                dup2(fdsFin[1], STDOUT_FILENO);
                //TH: If less than 3 args, autofill
                char *rankArgs[5];
                if (tokenCount == 1) {
                        char *rankArgs[] = {"rank", "score", "100", "uplink", NULL};
                } else if (tokenCount == 2) {
                        char *rankArgs[] = {"rank", tokens[1], "100", "uplink", NULL};
                } else {
                        char *rankArgs[] = {"rank", tokens[1], tokens[2], "uplink", NULL};
                }
                st = execve(rankPath, rankArgs, NULL);
                if (st == -1) {
                    perror("execve");
                    exit(1);
                }
            } else {
                //K.P: Parent process
                //K.P: Close the read end of the pipe
                close(fdsInit[0]);
                close(fdsFin[1]);
                //K.P: Write savePath to the write end of the pipe
                write(fdsInit[1], savePath, strlen(savePath));
                //K.P: Close the write end of the pipe
                close(fdsInit[1]);

                //K.P: Wait for the rank process to finish
                int status;
                waitpid(rank_pid, &status, 0);

                char rankLine[MAX_LINE_LENGTH];
                int fd = fdsFin[0];
                FILE* rankFile = fdopen(fdsFin[0], "r");
                if (rankFile == NULL) {
                        perror("RankFile Open");
                        exit(1);
                }
                int numRank = (tokenCount > 2) ? atoi(tokens[2]) : 5;
                char targetFile[MAX_LINE_LENGTH];
                sprintf(targetFile, "%s/%s", userName, savePath);
                char isTarget = 0;

                printf("--- -------------------------------------------------- \n");
                printf("#   File Path                                          \n");
                printf("--- -------------------------------------------------- \n");
                if (tokenCount > 2) {
                        int numPrinted = 0;
                        while (fgets(rankLine, MAX_LINE_LENGTH, rankFile)) {
                                //TH: Max printed file name length is 50 chars
                                //TH: Append null char to end of rankLine instead of new line
                                rankLine[strcspn(rankLine, "\n")] = '\0';
                                if (strcmp(targetFile, rankLine)==0) {
                                        isTarget = 1;
                                }

                                if (strlen(rankLine) >= 48) {
                                        rankLine[47] = '\0';
                                        strcat(rankLine, "...");
                                }

                                numPrinted++;

                                printRank(numPrinted, rankLine, isTarget);
                                isTarget = 0;

                                if (numPrinted == numRank) {
                                         break;
                                }
                        }
                } else {
                        int numPassed = 0;

                        char** passed = malloc(sizeof(char*));
                        int arrSize = 1;
                        char* cur;
                        int numLeft = numRank + 1;

                        while (fgets(rankLine, MAX_LINE_LENGTH, rankFile)) {
                                rankLine[strcspn(rankLine, "\n")] = '\0';
                                if (numLeft!=numRank+1) {
                                        printRank(numPassed, rankLine, 0);
                                        numLeft--;
                                }

                                if (numPassed >= arrSize) {
                                        passed = realloc(passed, 2 * numPassed * sizeof(char*));
                                }

                                cur = malloc(MAX_LINE_LENGTH);
                                strcpy(cur, rankLine);

                                passed[numPassed] = cur;

                                numPassed++;

                                if (strcmp(targetFile, rankLine)==0) {
                                        numLeft--;
                                        for (int i = numPassed - numRank; i <= numPassed; i++) {
                                                if (i < 1) {
                                                        continue;
                                                }

                                                printRank(i, passed[i-1], i == numPassed);
                                        }
                                }

                                if (!numLeft) {
                                        break;
                                }
                        }
                }

                close(fdsFin[0]);
            }
        }
        if (inputCheck("visualize", tokens[0])) {
                //TH: call printBoard function to print the board
                printBoard(tGame, savePath);
        }
        if(inputCheck("train", tokens[0])){
            train();
        }
        if (inputCheck("info", tokens[0])) {
                //TH: Print current file, score, lines
                printf("Current savefile: %s\n", savePath);
                printf("Score: %u\n", tGame.score);
                printf("Lines: %u\n", tGame.lines);
                printf("Is Save Legitimate: %s\n", saveIsValid ? "True" : "False");
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
    free(savePath);
    free(userInput);

}


void print_title(int num_spaces) {
    if (strcmp(getenv("TERM"), "xterm-256color") == 0) {
        printf("\033[34m");
    }

    printf("\033[2J\033[H"); //K.P: Clears the screen and move the cursor to the top-left corner
    for (int i = 0; i < num_spaces; ++i) {
        printf(" ");
    }
    printf("Welcome to...\n");

    printf("%*s   ______     __             _____ __         ____\n", num_spaces, "");
    printf("%*s  /_  __/__  / /__________ _/ ___// /_  ___  / / /\n", num_spaces, "");
    printf("%*s   / / / _ \\/ __/ ___/ __ /\\__ \\/ __  \\/ _ \\/ / / \n", num_spaces, "");
    printf("%*s  / / /  __/ /_/ /  / /_/ /___/ / / / /  __/ / /  \n", num_spaces, "");
    printf("%*s /_/  \\___/\\__/_/   \\___//____/_/ /_/\\___/ _/_/   \n\n", num_spaces, "");

    if (strcmp(getenv("TERM"), "xterm-256color") == 0) {
        printf("\033[0m");
    }
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

char *switchFile(char* savePath, char* newPath) {
        //K.P: copies new path into original buffer and then prints the switch.
        char oldPath[MAX_LINE_LENGTH];
        strncpy(oldPath, savePath, MAX_LINE_LENGTH);
        strncpy(savePath, newPath, MAX_LINE_LENGTH);
        savePath[MAX_LINE_LENGTH - 1] = '\0'; //K.P: Ensure null termination
        printf("Switched current quicksave from %s to %s.\n", oldPath, savePath);
        return savePath;
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

bool vailidateSave(char* savePath){
    //K.P: Validates the given save.
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        close(fd[0]); //K.P: Close read end of the pipe
        dup2(fd[1], STDOUT_FILENO); //K.P: Redirect stdout to write end of the pipe

        char *checkArgs[] = {checkPath, savePath, NULL};
        int st = execve(checkPath, checkArgs, NULL);
        if (st == -1) {
            perror("execve");
            exit(1);
        }
    } else {
        close(fd[1]); //K.P: Close write end of the pipe

        int status;
        waitpid(pid, &status, 0);

        //K.P: Read from the pipe and store the output in a char* buffer
        char *checkOutput = malloc(MAX_LINE_LENGTH);
        ssize_t bytesRead;
        while ((bytesRead = read(fd[0], checkOutput, MAX_LINE_LENGTH)) > 0) {
            checkOutput[bytesRead] = '\0';
        }

        bool saveIsValid;
        char *notLegit = "illegit";

        char *badSave = strstr(checkOutput, notLegit);


        if(badSave){
            saveIsValid = false;
        }
        else{
            saveIsValid = true;
        }
        //K.P: Close read end of the pipe and free the memory
        close(fd[0]);
        free(checkOutput);
        return saveIsValid;
    }
}

//K.P: Here are helper functions for train() and the train().
char* readInput() {
    char *input = malloc(100);
    fgets(input, 100, stdin);
    input[strcspn(input, "\n")] = '\0';
    return input;
}

bool checkExit(const char *input) {
    return strncmp(input, "exit", 4) == 0;
}

char* intToBinary(int integer) {
    char *binary = malloc(9);
    for (int i = 7; i >= 0; i--) {
        binary[7 - i] = (integer & (1 << i)) ? '1' : '0';
    }
    binary[8] = '\0';
    return binary;
}

void printRec(int recNum, char* fileName, unsigned score, unsigned lines) {
        char recStr[3];
        sprintf(recStr, "%d", recNum);
        printf("%s", recStr);
        for (int i = strlen(recStr); i < 4; i++) {
                putchar(' ');
        }

        printf("%s", fileName);
        for (int j = strlen(fileName); j < 30; j++) {
               putchar(' ');
        }

        char scoreStr[7];
        sprintf(scoreStr, "%u", score);
        printf("%s", scoreStr);
        for (int n = strlen(scoreStr); n < 8; n++) {
                putchar(' ');
        }

        printf("%u\n", lines);
}

void printRank(int rankNum, char* fileName, char isBold) {
    if (rankNum < 10) {
        if (isBold) {
            printf("\033[1m%d   >> %s <<\033[0m\n", rankNum, fileName);
        } else {
            printf("%d   %s\n", rankNum, fileName);
        }
    } else {
        if (isBold) {
            printf("\033[1m%d  >> %s <<\033[0m\n", rankNum, fileName);
        } else {
            printf("%d  %s\n", rankNum, fileName);
        }
    }
}


void train() {
    bool isFinished = false;
    srand(time(NULL));
    printf("Welcome to train. type 'exit' to quit\n");
    while (!isFinished) {
        //K.P: Picks a random 8 bit integer.
        int integer = rand() % 256;
        //K.P: Picks a random case to start at,
            // Either decimal, hex, or integer.
        int choice = rand() % 3;
        //K.P: Had to free the arrays every case because otherwise will cause core dump.
        switch (choice) {
            case 0: {
                //K.P: int to choose if int will be converted to hex or bin.
                int hexOrBin = rand() % 2;
                if (hexOrBin == 0) {
                    //K.P: Convert to hex
                    printf("Convert integer: %d into hex -> ", integer);
                    char *hexInput = readInput();
                    if (checkExit(hexInput)) {
                        isFinished = true;
                        free(hexInput);
                        break;
                    }
                    int hexAsInt = strtol(hexInput, NULL, 16);
                    if (hexAsInt == integer) {
                        printf("Correct!\n");
                    } else {
                        printf("No, the correct answer is: %04X\n", integer);
                    }
                    free(hexInput);
                } else if (hexOrBin == 1) {
                    //K.P: Convert to binary.
                    printf("Convert integer: %d into binary -> ", integer);
                    char *binaryInput = readInput();
                    if (checkExit(binaryInput)) {
                        isFinished = true;
                        free(binaryInput);
                        break;
                    }
                    int binToInt = strtol(binaryInput, NULL, 2);
                    if (binToInt == integer) {
                        printf("Correct!\n");
                    } else {
                        char *correctBinary = intToBinary(integer);
                        printf("No, the correct answer is: %s\n", correctBinary);
                        free(correctBinary);
                    }
                    free(binaryInput);
                }
                break;
            }
            case 1: {
                //K.P: int to decide if hex will be converted to binary or int.
                int hexOrInt = rand() % 2;
                if (hexOrInt == 0) {
                    //K.P: Convert to binary
                    printf("Convert hex number: %04X into binary -> ", integer);
                    char *binaryInput = readInput();
                    if (checkExit(binaryInput)) {
                        isFinished = true;
                        free(binaryInput);
                        break;
                    }
                    int binToInt = strtol(binaryInput, NULL, 2);
                    if (binToInt == integer) {
                        printf("Correct!\n");
                    } else {
                        char *correctBinary = intToBinary(integer);
                        printf("No, the correct answer is: %s\n", correctBinary);
                        free(correctBinary);
                    }
                    free(binaryInput);
                } else if (hexOrInt == 1) {
                    //K.P: Convert to int.
                    printf("Convert hex number: %04X to integer -> ", integer);
                    char *intInput = readInput();
                    if (checkExit(intInput)) {
                        isFinished = true;
                        break;
                    }
                    int integerInput = atoi(intInput);
                    free(intInput);
                    if (integerInput == integer) {
                        printf("Correct!\n");
                    } else {
                        printf("The correct answer is: %d\n", integer);
                    }
                }
                break;
            }
            case 2: {
                //K.P: int to decide if binary will be converted to hex or int.
                int hexOrInt = rand() % 2;
                if (hexOrInt == 0) {
                    //K.P: Convert to int
                    char *binary = intToBinary(integer);
                    printf("Given binary: %s convert to integer -> ", binary);
                    char *intInput = readInput();
                    if (checkExit(intInput)) {
                        isFinished = true;
                        free(intInput);
                        break;
                    }
                    int integerInput = atoi(intInput);
                    free(intInput);
                    if (integerInput == integer) {
                        printf("Correct!\n");
                    } else {
                        printf("No, the correct answer is: %d\n", integer);
                    }
                } else if (hexOrInt == 1) {
                    //K.P: Convert to hex
                    char *binary = intToBinary(integer);
                    printf("Given binary: %s convert to hex -> ", binary);
                    char *hexInput = readInput();
                    free(binary);
                    if (checkExit(hexInput)) {
                        isFinished = true;
                        free(hexInput);
                        break;
                    }
                    int hexAsInt = strtol(hexInput, NULL, 16);
                    if (hexAsInt == integer) {
                        printf("Correct!\n");
                    } else {
                        printf("No, the correct answer is: %04X\n", integer);
                    }
                    free(hexInput);
                }
                break;
            }
        }
    }
    if (isFinished) {
        printf("Thanks for playing.\n");
    }
}


