/*Alex Markowitz, admarko
 *Shell Simulator
 *Spring 2016 - 1.0
 *Fall 2017 - 2.0
 */
#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

//Helper to print messages to STDOUT
void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}
char error[30] = "\nAn error has occurred\n";


#define MAX 514     //testing if input > max possible length

//helper to parse input from command line, returns num of different commands
int input_help(char* c, char** cmds){
    char* help = malloc(sizeof(char));
    char* locptr = malloc(sizeof(char));
    locptr = NULL;
    help = strtok_r(c, " \n\a\t\r\v\f", &locptr);
    
    int counter = 0; //commands;
    while (help != NULL) {
        char* temp = malloc(sizeof(char));
	temp = help;
	cmds[counter] = temp;
        counter++;
        help = strtok_r(NULL, " \n\a\t\r\v\f", &locptr);
    }
    cmds[counter] = NULL;
    counter--;
    return counter;   
    free(help);
    free(locptr);
}

//helper for special_word determines whether char c is whitespace
int special_char(char c){
    int isnt_special_char = 1;
    if ((c == '\n') || (c == '\t') || (c == '\v') || (c == '\f') || (c == '\r') || (c == ' ')) {
        isnt_special_char = 0;   
    }
    return isnt_special_char;
}

//helper for main determines whether word *c has whitespace in it
int special_word(char* c){
    int isnt_special_word = 1;
    int len = strlen(c);
    for (int i = 0; i < len; i++) {
        if (special_char(c[i])) {
            isnt_special_word = 0;
        }
    }
    return isnt_special_word;
}

//handler for "exit" command
void exit_help(char** args, int len){
    if (len>1) {
        myPrint(error); //too many arguments with 'exit' command
    }
    exit(0);
}

//handler for "pwd" command
void pwd_help(char** args, int len){
    char* temp = malloc(sizeof(char)*200);
    if (len>1) {
        myPrint(error); //too many arguments with 'pwd' command
    }
    getcwd(temp, 200);  //changes pwd to temp
    myPrint(temp);      //prints new pwd
    myPrint("\n");      //nessecary newline
    free(temp);
}

//handler for "cd" command
void cd_help(char** args, int len){
    //need to test if fails: chdir() == -1, 
    if (len == 1) {                         //if no dir, go HOME
        if (chdir(getenv("HOME")) == -1) {
            myPrint(error);                 //error: chdir error
        }
    }else if (len == 2) {                   //if 1 dir, go to dir
        if (chdir(args[1])== -1) {
            myPrint(error);                 //error: chdir error
        }
    }else if (len > 2 || len < 1) {         //if too many args or error w len
        myPrint(error);                     //too many arguments with cd command
    }
}

//handler for commands with ">" redirection
void redirect_help(char** args, int len) {
    //vars for first else-if
    char* temp = malloc(sizeof(char));
    temp = NULL;
    char* FILE = malloc(sizeof(char));
    FILE = NULL;
    char* locptr = malloc(sizeof(char));
    locptr = NULL;
    char* redirect_loc = malloc(sizeof(char));
    redirect_loc = strchr(args[len], '>');  //pointer to loc of '>'
    //int advred = 0; //Boolean for advanced redirect
    int lenhelp = strlen(args[len-1])-1;    //helper for last case

    //command >file
    if (args[len][0] == '>') {
        FILE = args[len]+1; //get filename
        args[len] = NULL;   //remove after >
    }

    //command>file
    else if (redirect_loc != NULL) {
        temp = strtok_r(args[len], ">", &locptr);  //get command
        FILE = strtok_r(NULL, ">", &locptr);       //get filename
        args[len] = temp;                          //delete everything after command
    }

    //command > file
    else if (args[len-1][0] == '>') {
        FILE = args[len];   //get filename
        args[len] = NULL;   //remove filename
        args[len-1] = NULL; //remove redirection
    }

    //command> file
    else if (args[len-1][lenhelp]=='>') {
        FILE = args[len];           //get filename
        args[len] = NULL;           //remove filename
        args[len-1][lenhelp] = 0;   //remove redirection
    }

    //Any other format with: command, >, file is invalid
    else {
        myPrint(error); //error with format of > redirection
        exit(0);
    }

    //vars for second else-if
    int accessable = access(FILE, F_OK); //F_OK is flag for testing existance of file
    pid_t id = fork();
    int wait_help;      //for wait()
    int openablef;      //for open()
    int redirection;    //for dup2()
    int executable;     //for execvp()
    int open_method;    //for open()
    if (accessable != -1) {
        myPrint(error); //unaccessable
    } else if (id < 0) {
        myPrint(error); //error with fork()
        exit(0);
    } else if (id == 0) {
        //used chart of open methods, and open permissions to determine which were best
        open_method = O_WRONLY | O_CREAT; //Write only, and if DNE, create it.
        
        //for mode - read/write for owner and other users to be true
        mode_t open_permissions = S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH;
        openablef = open(FILE, open_method, open_permissions);
        
        //test if file not openable
        if (openablef < 0) {
            myPrint(error);
            exit(0);
        }
        
        //test for error w dup2() changing STDOUT (=1) to openablef
        redirection = dup2(openablef, 1);
        if (redirection < 0) {
            myPrint(error); 
            exit(0);
        }
	
        //test for execution error
        executable = execvp(args[0], args);
        if (executable == -1) {
            myPrint(error); 
            exit(0);
        }
        
        close(openablef);
        exit(0);

    } else {
        while(id != wait(&wait_help));
        return; 
    }
    free(temp);
    free(FILE);
    free(locptr);
    free(redirect_loc);
}

//handler for all other non-builtin commands
void other_help(char** cmds){
    pid_t id;
    id = fork();
    int wait_help;  //for wait()
    int executable; //for execvp()

    //fork error
    if (id<0) {
        myPrint(error);
        exit(0); //execution error
    } else if (id == 0) {
	executable = execvp(cmds[0], cmds);
        if(executable == -1){
            myPrint(cmds[0]);
            myPrint(error); 
            exit(0); //child
        }
    } else { //no errors
        //waiting for child
        while(id != wait(&wait_help));
        return;
    }
}

void choose_helper(char* cmd_buff){
    char* args[200];
    char* locptr = NULL;    //malloc(sizeof(char));
    char* temp = NULL;      //malloc(sizeof(char));
    int len;
    int i = 0;
    int j = 0;
    int redirect = 0;
    int advredirect = 0;
    temp = strtok_r(cmd_buff, ";\n", &locptr); //split commands by ; or \n
    while (temp != NULL) { //scroll through all commands
        if (!special_char(temp[0])) //ignore initial space " "
            temp = temp + 1;
        
 	//check if (advanced) redirection ocurrs by scrolling through each char
	len = input_help(temp, args); //number of commands
	if (len >= 0) {
            for (i=0; i<=len; i++) { //scroll through each word
                for (j=0; args[i][j] != 0; j++) { //check each letter
                    if(args[i][j] == '>') {
                        redirect = redirect + 1;
                        if (args[i][j+1] == '+') {
                            advredirect++;
                        }
                    }
                }
            }
	
            //if exit is called
            if (strstr(args[0], "exit") != NULL) {
                if (redirect) {
                    myPrint(error); //error: redirection with exit
                    break;
                }
                exit_help(args, len+1);
            }

            //if cd is called
            else if (strstr(args[0], "cd") != NULL) {
                if (redirect) {
                    myPrint(error); //error: redirection with cd
                    break;
                }
                cd_help(args, len+1);
            }

            //if pwd is called
            else if (strstr(args[0], "pwd") != NULL) {
                if (redirect) {
                    myPrint(error); //error: redirection with pwd
                    break;
                }
                pwd_help(args, len+1);
            }

            //test if more than 1 redirections
            else if (redirect > 1) {
                myPrint(error); //more than 1 >
                break;
            }

            //if redirection is called
            else if(redirect == 1){
                redirect_help(args, len);
                break;
            } else {    //all other cases
                other_help(args);
            }

        }
        temp = strtok_r(NULL, ";\n", &locptr); //repeat/reset for every command
    }
}

int main(int argc, char *argv[]){
    char cmd_buff[MAX] = {'\0'};
    char *pinput = malloc(sizeof(char));
    FILE* f;
    int batchmode = 0;
    int len = 0;
    
    if (argc > 2) {     //if too many arguments
        myPrint(error);
        exit(0);
    }

   
    else if (argc == 2) {     //if batch mode is nessecary
        batchmode = batchmode + 1;
        f = fopen(argv[1], "r");
        //test if file D.N.E.
	if (f == NULL) {
            myPrint(error); //error: file D.N.E.
            exit(0);
        }
    }

    while (5) {
	 if (batchmode) {
            pinput = fgets(cmd_buff, 1100, f);
            if (!pinput) { //when nothing is entered
                exit(0);
            }
       
	 //deal with line breaks 
 	 len = strlen(cmd_buff);
   	 char* lastchar = &(cmd_buff[len-1]);
   	 if (strcmp(lastchar, "\n") == 0) {
       	 	*lastchar = '\0';
     }

            if (strcmp(cmd_buff, "\n") != 0) {      //ensure input is not newline char
                if (!special_word(pinput)) {
                    myPrint(cmd_buff);
                    if (strlen(cmd_buff) > MAX) {   //for 1K Command
                        myPrint("\n");              //necessary for ^
			myPrint(error);                         //necessary for ^^
		    } else {
                        myPrint("\n");
                        choose_helper(cmd_buff);
                    }
                }
            }

         //not in batch mode
        } else {
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 514, stdin); //stores 514 bytes of stdin to cmd_buf
            if (!pinput) { //when nothing is entered
              exit(0);
            }
            len = strlen(cmd_buff);
            if (len < 2 || len > 514) {
                myPrint("\n");
                myPrint(error);
            } else {
                choose_helper(cmd_buff);
            }
        }
    }
    if(batchmode)
    fclose(f);
    
    return 0; 
    free(pinput);
}
