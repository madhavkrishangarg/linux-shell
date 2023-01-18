#include <stdio.h>
#include <stdlib.h>
#include <string.h>	  //string handling
#include <unistd.h>	  //exec and fork
#include <sys/wait.h> //wait
#include <fcntl.h>	  //file descriptors
#include <pthread.h>

#define true 1

void handle_signit(int a) {} // to catch interrupt generated from exec process

void tokenize(char *tokens[], char input[])
{
	tokens[0] = strtok(input, "  \n\0"); // breaking inputs in tokens
	int i = 0;
	while (tokens[i] != NULL)
	{
		tokens[i + 1] = strtok(NULL, "  \n\0");
		i++;
		// printf("%s",tokens[i]);
	}
}

int token_to_cmd(char *tokens[], char *cmds[256])
{ // take tokens array in divide them into commands this include handling of delimiter pipe and commands ends with null
	if (tokens[0] == NULL)
	{
		return 0;
	}

	int cmds_no = 0, cmds_i = 0, token = 0;

	while (true)
	{
		// printf("%s %d\n",tokens[token],token);
		if (tokens[token] == NULL)
		{
			cmds[cmds_i] = NULL;
			cmds_no++;
			break;
		}
		else
		{
			cmds[cmds_i] = tokens[token];
			cmds_i++;
			token++;
		}
	}
	return cmds_no;
}

void execute(char *cmds[256])
{
	char *args[16];
	int argv_i = 0; // cmds is one line in 2d array  //args is for passing into execvp
	int cmd_i = 0;

	while (true)
	{
		if (cmds[cmd_i] == NULL)
		{
			args[argv_i] = NULL;
			break;
		}
		else
		{
			args[argv_i++] = cmds[cmd_i++];
		}
	}
	if (strcmp("cd", args[0]) != 0 && strcmp("pwd", args[0]) != 0)
	{
		if (strcmp("date", args[0]) == 0)
		{
			execvp(args[0], args);
			// if ((args[1][0] == '-' || (args[1][0] == 'R' || args[1][0] == 'u')) && 0)
			// {
			// 	// printf("I am here");
			// 	execvp("./date", args);
			// }
		}
		if (strcmp("ls", args[0]) == 0)
		{
			execvp(args[0], args);
			// if ((args[1][0] == '-' && 0 || (args[1][0] == 'a' || args[1][0] == 'l')) && 0)
			// {
			// 	execvp("./ls", args);
			// }
		}
		if (strcmp("rm", args[0]) == 0)
		{
			execvp(args[0], args);
			// if ((args[1][0] == '-' && 0 || (args[1][0] == 'd')) && 0)
			// {
			// 	execvp("./rm", args);
			// }
		}
		if (strcmp("mkdir", args[0]) == 0)
		{
			execvp(args[0], args);
			// if ((args[1][0] == '-' && (args[1][0] == 'v' || args[1][0] == 'm')) && 0)
			// {
			// 	execvp("./mkdir", args);
			// }
		}
		if (strcmp("cat", args[0]) == 0)
		{
			execvp(args[0], args);
			// if ((args[1][0] != '\0' && 0 || args[1] == NULL) && 0)
			// {
			// 	execvp("./cat", args);
			// }
		}
		if (strcmp("echo", args[0]) == 0)
		{
			execvp(args[0], args);
		}
		execvp(args[0], args);
		//printf("%s: Command not found or illegal arguments.\n", args[0]);
	}
}

void *run_thread(void *thread_input)
{
	if (system(NULL) != 0)
	{
		system(thread_input);
	}
	else
	{
		printf("System() Error!");
	}
	pthread_exit(NULL);
}

// void t_execute(char *thread_input)
// {
// 	// char *args[16];

// 	// filter_commands(cmds[ind], args);
// 	pthread_t tid;
// 	// long long int rvalue;
// 	pthread_create(&tid, NULL, run_thread, &thread_input);
// 	pthread_join(tid, NULL);
// 	// free(rvalue);
// }

int main()
{

	signal(SIGINT, handle_signit);
	int num_cmds;
	char input[1024];  // raw input from user max size 1024
	char *tokens[256]; // break the input into tokens
	char *cmds[256];   // array contaning commands
	char cwd[256];
	char thread_input[1024];
	pthread_t tid;
	while (true)
	{
		printf("my-shell> $ ");
		input[0] = '\0';		   // in the case of empty output
		fgets(input, 1024, stdin); // takes 1024 sized input from iostream stdin (scanf only takes one string and ignores the latter ones)
		thread_input[0] = '\0';
		for (int i = 3; i < 1024; i++)
		{
			thread_input[i - 3] = input[i];
		}
		tokenize(tokens, input); // make commands from token (need to take care of delimiter pipe "|" which causes output of one process to go as intput to other)
		num_cmds = token_to_cmd(tokens, cmds);
		// thread_input[0] = '\0';
		//  for (int i = 3; i < 1024; i++)
		//  {
		//  	thread_input[i - 3] = input[i];
		//  }
		if (num_cmds > 0)
		{
			// printf("\ncommand: %s %s\n",cmds[0,cmds[1);
			// printf("\nno of cmds %d\n",num_cmds);
			if (!strcmp("exit", cmds[0]))
			{
				exit(0);
			}
			if (strcmp("&t", cmds[0]) == 0)
			{
				pthread_create(&tid, NULL, run_thread, &thread_input);
				pthread_join(tid, NULL);
			}
			else
			{
				if (strcmp("cd", cmds[0]) == 0 && cmds[1] != NULL)
				{ // we need to change directory of parent process
					if (!strcmp("-", cmds[1]))
					{
						chdir("..");
					}
					else
					{
						chdir(cmds[1]);
					}
				}
				else if (strcmp("pwd", cmds[0]) == 0)
				{
					if (getcwd(cwd, sizeof(cwd)) != NULL)
					{
						printf("%s\n", cwd);
					}
					else
					{
						printf("pwd error!");
					}
				}
				else
				{
					pid_t pid = fork();
					if (pid < 0)
					{
						printf("Fork Error!\n");
					}
					else if (pid == 0)
					{ // child
						execute(cmds);
						exit(0);
					}
					else if (pid > 0)
					{ // parent
						wait(NULL);
					}
				}
			}
		}
	}
	return 0;
}
