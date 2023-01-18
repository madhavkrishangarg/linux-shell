#include <stdio.h>
#include <stdlib.h>
#include <string.h> //string handling
#include <unistd.h> //exec and fork
#include <sys/wait.h> //wait
#include <fcntl.h> //file descriptors

#define true 1

void handle_signit(int a){}         //to catch interrupt generated from exec process

void tokenize(char* tokens[], char input[]){
    tokens[0] = strtok(input, "  \n\0");			//breaking inputs in tokens
    int i = 0;
    while(tokens[i] != NULL){
        tokens[i + 1] = strtok(NULL, "  \n\0");
        i++;
        //printf("%s",tokens[i]);
    }
}

int token_to_cmd(char *tokens[],char *cmds[32][16]){	//take tokens array in divide them into commands this include handling of delimiter pipe and commands ends with null
    if (tokens[0] == NULL){
        return 0;}

    int cmds_no = 0, row = 0, col = 0, token = 0;

    while (true){
        //printf("%s %d\n",tokens[token],token);
        if (tokens[token] == NULL){
            cmds[row][col] = NULL;
            cmds_no++;
            break;
        }
        else if (strcmp(tokens[token], "|")==0){
            cmds[row][col] = NULL;
            token++;
            row++;
            cmds_no++;
            col = 0;
        }else{
            cmds[row][col] = tokens[token];
            col++;
            token++;
        }
    }
    return cmds_no;
}

//for file descriptors 0 - stdin, 2 - stdout, 3 - stderr
void set_fd(char *cmds[]){
    for (int i = 0; cmds[i] != NULL; i++){
        int file_descriptor;
        //reference stackoverflow , geekforgeeks
        if (strcmp(cmds[i], ">>")==0){           // command >> filename
            file_descriptor = open(cmds[i + 1], O_APPEND | O_RDWR, S_IRWXU);
            if (file_descriptor < 1) file_descriptor = open(cmds[i + 1], O_CREAT | O_RDWR, S_IRWXU);
            close(1);
            dup(file_descriptor);
        }else if (strcmp(cmds[i], "<")==0){               // command < filename
            file_descriptor = open(cmds[i + 1], O_RDONLY | O_RDWR, S_IRWXU);
            close(0);
            dup(file_descriptor);
        }else if (strcmp(cmds[i], ">")==0){             // command > filename
            file_descriptor = open(cmds[i + 1], O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
            close(1);
            dup(file_descriptor);
        }else if (sizeof(cmds[i]) * sizeof(char) > 2 && *(cmds[i] + 1) == '>'){
            if (*(cmds[i] + 0) == '2'){                 // 2>filename
                file_descriptor = open(cmds[i] + 2, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
                close(2);
                dup(file_descriptor);
            }else if (*(cmds[i] + 0) == '1'){              // 1>filename
                file_descriptor = open(cmds[i] + 2, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
                close(1);
                dup(file_descriptor);
            }else if (*(cmds[i] + 2) == '&' && *(cmds[i] + 3) == '1'){      // 2>&1
                close(2);
                dup(1);
            }
        }
    }
}

void filter_commands(char *cmds[],char *args[]){				//converting to form so that it is ready to be used in execvp()  args[0],args

    int argv_i = 0;											// cmds is one line in 2d array  //args is for passing into execvp
    int cmd_i = 0;

    while (true){
        if (cmds[cmd_i] == NULL){
            args[argv_i] = NULL;
            break;
        }else{
            if (sizeof(cmds[cmd_i]) * sizeof(char) > 2 && *(cmds[cmd_i] + 1) == '>'){
                ++cmd_i;
            }else if (strcmp(cmds[cmd_i], ">")==0 || strcmp(cmds[cmd_i], ">>")==0 || strcmp(cmds[cmd_i], "<")==0){
                cmd_i += 2;
            }else{
                args[argv_i++] = cmds[cmd_i++];
            }
        }
    }
}

void execute(char *cmds[32][16],int ind){
    char *args[16];

    filter_commands(cmds[ind],args);
    set_fd(cmds[ind]);

    if (ind > 0){
        pid_t pid;
        int file_descriptor[2];

        if (pipe(file_descriptor)==1){
            printf("Pipe error!\n");

        }else{

            if ((pid = fork()) < 0){
                printf("Fork Error!");
            }else if (pid == 0){
                close(1);
                dup(file_descriptor[1]);

                close(file_descriptor[0]);
                close(file_descriptor[1]);

                execute(cmds,ind - 1);
            }else{
                close(0);
                dup(file_descriptor[0]);

                close(file_descriptor[0]);
                close(file_descriptor[1]);
                if(strcmp("cd",args[0])!=0) {
                    execvp(args[0], args);
                    printf("%s: Command not found.\n", args[0]);
                }
            }
        }
    }else{
        if(strcmp("cd",args[0])!=0) {
            execvp(args[0], args);
            printf("%s: Command not found.\n", args[0]);
        }
    }
}

int main(){

    signal(SIGINT,handle_signit);
    int num_cmds;
    char input[1024];               //raw input from user max size 1024
    char *tokens[256];       		//break the input into tokens
    char *cmds[32][16]; 			//2d array contaning commands for pipelines can support upto 16 pipelines

    while(true){
        printf("my-shell> $ ");
        input[0]='\0';				//in the case of empty output
        fgets(input,1024,stdin);	//takes 1024 sized input from iostream stdin (scanf only takes one string and ignores the latter ones)
        tokenize(tokens,input);		//make commands from token (need to take care of delimiter pipe "|" which causes output of one process to go as intput to other)
        num_cmds=token_to_cmd(tokens,cmds);
        if (num_cmds > 0){
           // printf("\ncommand: %s %s\n",cmds[0][0],cmds[0][1]);
           // printf("\nno of cmds %d\n",num_cmds);
            if (!strcmp("exit", cmds[0][0])){
                exit(0);
            }

            if(strcmp("cd",cmds[0][0])==0 && cmds[0][1]!=NULL){   //we need to change directory of parent process
                if(!strcmp("-",cmds[0][1])){
                    chdir("..");
                }else{
                    chdir(cmds[0][1]);
                }
            }else{
                pid_t pid = fork();
                if (pid < 0) {
                    printf("Fork Error!\n");
                } else if (pid == 0) {                     //child
                    execute(cmds, num_cmds - 1);
                    exit(0);
                } else if (pid > 0) {                        //parent
                    wait(NULL);
                }
            }
        }
    }
    return 0;
}