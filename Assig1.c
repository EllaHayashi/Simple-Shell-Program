//By Ella Hayashi
//csc360 Assignment1

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct wordList wordList;
struct wordList
{
	pid_t pid;
	char argument[1024];
	int argNum;
	wordList *next;
	char *directory;
};

//new node
wordList *newitem(char *string, pid_t place, int args, char *direct) 
{
	wordList* newp = (wordList*)malloc(sizeof(wordList));
	if(newp==NULL)
	{
		printf("Error creating a new node.\n");
		exit(0);
	}

	strcpy(newp->argument, string);
	newp->next = NULL;
	newp->pid = place;
	newp->argNum = args;
	newp->directory = direct;
	return newp;
}

//adding the node to the end of the list
void *addEnd(wordList *front, wordList *insertionNode)
{
	wordList *p;
	for(p=front; p->next !=NULL; p=p->next)
	;
	
	p->next = insertionNode;
}

void *emalloc(size_t n)
{
	void *p;
	p = malloc(n);
	if(p==NULL)
	{
		fprintf(stderr, "malloc of %zu bytes failed\n", n);
		exit(1);
	}
	return p;
}

void freeall(wordList *list)
{
	wordList *next;
	for( ; list !=NULL; list = next)
	{
		next = list ->next;
		free(list);
	}
}

//tokenizing function that takes a string and puts it into an array
char **tokenize_intoArray(char *input_line)
{
	char **args = malloc(100*sizeof(char*));
//	char *t = malloc(20*sizeof(char*));
//	t = strtok(input_line, " \n");
	char *t = strtok(input_line, " \n");
	
	int counter = 0;
	args[counter] = t;
	counter++;
	while(t!=NULL)
	{
		t=strtok(NULL, " \n");
		if(t!=NULL)
		{
			args[counter] = t;
			counter++;			
		}
	}
	args[counter]=NULL;
	return(args);
}	

//do Process takes a array, list, and a bg flag (0,1). it returns the list.
wordList *doProcess(char **args, wordList *front, int bg)
{
		int stats = 0;
		if(args[0]==NULL)
		{
			perror("no arguments\n");
			exit(1);	
		}

		pid_t process = fork();

		//error
		if(process<0)
		{
			perror("fork failed\n");
			exit(1);	
		}	
		
		//child
		else if(process==0)
		{
			if(execvp(args[0], args)<0)
			{
				printf("failed execution with arguments\n");
				exit(1);
			}
		}
		//parent		
		else
		{
			//if the flag is set and we need to execute in the background
			if(bg==1)
			{
				//create a new node and add it to the list.
				//calculate number of arguments and save the directory it is in to the node
				int k=0;
				while(args[k]!='\0')
				{
					k++;
				}
			
				char *buf;
				buf = (char *)malloc(100*sizeof(char));
				char *directory = getcwd(buf, 100);	//get the curent working directory
	
				wordList *p = newitem(args[0], process, k-1, directory); //number of arguments is k-l, because we exclude the first argument
				if(front==NULL)
				{
					front = p;
				}	
				else
				{
					addEnd(front, p);
				}

			}
			//no bg flag, so wait for child process to finish
			else
			{
				waitpid(process, &stats, 0);		
			}
		}
	return front;
}

//main
//creates a list and an array of arguments.
//see's if the first argument is bg, or cd, otherwise process normally
//checks at the end to see if there are any background processes that have terminated
int main(int argc, char **argv)
{
	wordList *front=NULL;	//creating the list of background programs
	char *buf;
	buf = (char *)malloc(100*sizeof(char));
	char *directory = getcwd(buf, 100);	//get the curent working directory
	char word[100] = "SSI: ";	
	strcat(word, directory);
	strcat(word, " > ");
	const char* prompt = word;	//our prompt, combination of working directory and strings

	int bailout = 0;
	while (!bailout)
	{	
		char* reply = readline(prompt);
		
/*		if(strcmp(reply, "\n"))
		{
			perror("no argument\n");
		}*/
		if(!strcmp(reply, "exit\n") || !strcmp(reply, "exit"))
		{
			bailout = 1;
		}
		else
		{
			//tokenise line into array
			char **array = tokenize_intoArray(reply);
			
			//if the first argument is bg_pro
			//we will print out all conetents in list
			if(strcmp(array[0], "bglist")==0)	
			{	
				wordList *p;
				int bglistCount = 0;
				for(p=front; p!=NULL; p=p->next)
				{
					printf("%d:  %s %d\n",p->pid, p->directory, p->argNum);
					bglistCount++;
				}		
				printf("Total Background jobs: %d\n", bglistCount);		
			}	
			
			//if the first argument is cd
			//we will check second argument
			else if (strcmp(array[0], "cd")==0)	
			{
				//if second argument is null or ~ then we will take it to the hoem directory
				if (array[1]==NULL||strcmp(array[1],"~")==0)
				{
					char *val;
					val = getenv("HOME");
				
					int returnVal = chdir(val);
					if(returnVal<0)
					{
						printf("Error, could not change directory\n");
					}
					else
					{
						prompt = val;
						char newword[100] = "SSI: ";
						strcat(newword, val);
						strcat(newword, " > ");
						prompt = newword;
					}	
				}
				
				//if the second argument is not .. or ~ then we will go to where the second argument tell us to
				else
				{
					int returnVal = chdir(array[1]);
					if(returnVal<0)
					{
						printf("Error, could not change directory\n");
					}
					else
					{
						char *bufer;
						bufer = (char *)malloc(100*sizeof(char));
						char newword[100] = "SSI: ";
						strcat(newword, getcwd(bufer,100));
						strcat(newword, " > ");
						prompt = newword;

					}
				}
			}

			//if the first argument is bg
			//then we must execute a program in the background
			else if(strcmp(array[0],"bg")==0)
			{
				//create a new temporary array to move contents and get rid of first argument 'bg'
				char **tempArray = malloc(100*sizeof(char*));
				int k = 1;
				while(array[k]!='\0')
				{
					tempArray[k-1] = array[k];
					k++;
				}			
				tempArray[k-1]='\0';
				array = tempArray;
				front = doProcess(array, front, 1);
			}			
			
			//otherwise, do process regularily
			else
			{
				front = doProcess(array, front, 0);
			}
			free(array);
		}
	
		//check to see if there are any background processes, then see if any children have termianted	
		//remove the node from the list that has the termianted child
		if(front != NULL)	
		{
			pid_t ter = waitpid(0, NULL, WNOHANG);
	
			//loop while there is a ter>0
			while(ter>0)
			{	
				//remove the node that equals the ter
				if(front->pid == ter)
				{

					printf("%d:  %s %d has terminated.\n", front->pid, front->directory, front->argNum);
					wordList *temp = front;
					front = front->next;
					free(temp);
				}
				else
				{
					wordList *p;
					for(p=front; p->next!=NULL; p=p->next)
					{
						if(p->next->pid == ter)
						{

							printf("%d:  %s %d has terminated.\n", p->pid, p->directory, p->argNum);
							wordList *temp = p->next;
							if(p->next->next == NULL)
							{
								p->next = NULL;
							}
							else
							{
								p->next = p->next->next;
							}
							free(temp);
						}
					}					
				}
				ter = waitpid(0,NULL,WNOHANG);
			}
		}
		free(reply);
	}

}


