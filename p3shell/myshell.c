#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>        // Waitpid
#include <sys/types.h>       // lseek() 
#include <ctype.h> 
#include <limits.h>


#include <fcntl.h>   
#include <unistd.h> 


#define PATH_MAX 4096
#define MAX_ARGUMENTS 10    

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

// If above 512, flush the rest 
void clear_rest() {
    scanf("%*[^\n]");
    getchar();
}


// ONE COMMAND ONLY  (ls)
// Prase and check commands (first command before space) 
char *parse_command(char *cmd_buff, char **str_rest) {
    // Return the first command, space as delimiter, point to remaining str
    // strtok_r according to Arjun on Ed to handle multiple
    char *command = strtok_r(cmd_buff, " ", str_rest); 

    // No input 
    if (command == NULL) {
        return NULL;
    }
    
    return command; 
}

// Parse remaining inputs  (-la /tmp), returns number of arguments 
int parsing_arguments(char *str_rest, char *args[]) {
    int i = 0; 

    while (str_rest != NULL && i < MAX_ARGUMENTS - 1) {
        args[i] = strtok_r(NULL, " ", &str_rest);  // NULL cuz it was read

        if (args[i] == NULL) { 
            break;
        }
        i++;
    }

    args[i] = NULL; 
    return i; 
}


void run_command(char *command, char *args[]) { 
    if (args[0] == NULL || strcmp(args[0], command) != 0) {
        for (int i = MAX_ARGUMENTS - 1; i > 0; i--) {
            args[i] = args[i - 1]; 
        }
        args[0] = command; 
    }

    pid_t pid = fork(); 

    if (pid == 0) { 
        if (execvp(command, args) == -1) {
            myPrint("An error has occurred\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);     // Parent waits 
    } else { 
        myPrint("An error has occurred\n");
    }
}

////////////////////////////////////////////////////////////////////////////////

// Built in commands 
// Helper for cd
void do_cd(char *args[]) {
    // cd /tmp /var would fail: AT MOST one argument 

    if (args[0] == NULL) {
        if (chdir(getenv("HOME")) == -1) {
            myPrint("An error has occurred\n");
        }
        return;
    }

    if (args[1] != NULL) { 
        myPrint("An error has occurred\n");
        return;
    }

    if (chdir(args[0]) == -1) {
        myPrint("An error has occurred\n");
    }
}

// Helper for pwd 
void do_pwd() {
    char *current = malloc(PATH_MAX); 
    if (current == NULL) {
        myPrint("An error has occurred\n");
        exit(EXIT_FAILURE);
    }


    // Notation: char *getcwd(char *buf, size_t size); 
    if (getcwd(current, PATH_MAX) != NULL) {
        myPrint(current);
        myPrint("\n");
    } else {
        myPrint("An error has occurred\n");
    }   

    free(current);
}

void builtin_commands(char *command, char *args[]) {
    // cd.  cd: getenv(HOME);   cd apath: chdir(apath)  
    if (strcmp(command, "cd") == 0) {
        do_cd(args);
        return;
    }

    // pwd 
    if (strcmp(command, "pwd") == 0) {
        
        if (args[0] != NULL) {
            myPrint("An error has occurred\n");
            return;
        }
        do_pwd();
        return; 
    }

}

////////////////////////////////////////////////////////////////////////////////

char *trim_spaces(char *str) {
    char *start = str; 
    char *end = str + strlen(str) - 1; 

    while (*start && isspace(*start)) {
        start++;
    }

    while (end > start && isspace(*end)) {
        *end = '\0';
        end--;
    }

    return start;
}


// Redirection: start with helper
// You want the first (or any) command before ;, and then check if there's >
int redirection_helper(char *cmd, char **txt, char **command) {
    *txt = NULL; 
    char *position_of_redir = strchr(cmd, '>');

    if (position_of_redir != NULL) {
        // Error: multiple '>'
        if (strchr(position_of_redir + 1, '>') != NULL) { // Look for another '>' after the first one
            return -1; // Invalid: multiple '>' characters
        }

        // Terminate the command part at `>`
        *position_of_redir = '\0';

        position_of_redir++;
        *txt = trim_spaces(position_of_redir);

        // If invalid like "ls > abc def" or "ls >"
        if (strchr(*txt, ' ') != NULL || strlen(*txt) == 0) { 
            return -1;
        }
    }
    *command = trim_spaces(cmd);    // Updates command whether or not it is redirected
    // "> output" should be invalid
    if (strlen(*command) == 0) {
        return -1;
    }
    return 0; 
}



// -1: multiple_commands will print error message
int do_redirection(char *cmd) {

    // E.g. ls -la /tmp > output  
    char *new_output = NULL;
    char *new_command = NULL; 


    // new_command =  
    int helper_status = redirection_helper(cmd, &new_output, &new_command);
    if (helper_status == -1) { 
        return -1;        // Invalid (3 cases)
    }

    
    // No built in -- error 
    char *rest_of_str;
    // e.g. ls -la /tmp  -- command = ls
    char *command = parse_command(new_command, &rest_of_str);    

    if (command == NULL) {
        return -1; 
    }
    if (strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0 || strcmp(command, "exit") == 0) {
        return -1; 
    }

    // Check if the output file already exists
    if (access(new_output, F_OK) == 0) {  
        return -1;
    }

    // Open output file, throw error as well 
    int file_descriptor = open(new_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        return -1;
    }

    pid_t pid = fork(); 
    if (pid == 0) {
        dup2(file_descriptor, STDOUT_FILENO);
        close(file_descriptor);

        char *args[MAX_ARGUMENTS]; 
        parsing_arguments(rest_of_str, args);

        // Ensure args[0] is the command
        if (args[0] == NULL || strcmp(args[0], command) != 0) {
            for (int i = MAX_ARGUMENTS - 1; i > 0; i--) {
                args[i] = args[i - 1]; 
            }
            args[0] = command; 
        }

        execvp(command, args);

        myPrint("An error has occurred\n");
        exit(1);
    } else if (pid > 0) {
        close(file_descriptor);              
        waitpid(pid, NULL, 0);  
    } else {
        // Fork failed
        close(file_descriptor);
        return -1;
    }

    return 0; 
}



int advanced_redirection_helper(char *cmd, char **txt, char **command) {
    *txt = NULL; 
    char *position_of_redir = strstr(cmd, ">+");

    if (position_of_redir != NULL) {
        if (strstr(position_of_redir + 2, ">+") != NULL) { 
            return -1; // Invalid: multiple '>+' characters
        }

        // Terminate at `>+`
        *position_of_redir = '\0'; 

        position_of_redir += 2;
        *txt = trim_spaces(position_of_redir);

        // Invalid: "ls >+ abc def" or "ls >+"
        if (strchr(*txt, ' ') != NULL || strlen(*txt) == 0) {
            return -1; 
        }
    }
    *command = trim_spaces(cmd); 

    // Check if command is empty (e.g., ">+ output")
    if (strlen(*command) == 0) {
        return -1;
    }
    return 0; 
}


int do_advanced_redirection(char *cmd) {
    char *new_output = NULL;
    char *new_command = NULL; 

    int helper_status = advanced_redirection_helper(cmd, &new_output, &new_command);
    if (helper_status == -1) {
        return -1;  
    }

    // Open fd 
    int file_descriptor = open(new_output, O_RDWR | O_CREAT, 0644);
    if (file_descriptor == -1) {
        return -1;  
    }

    // Copy fd into temporary file 
    off_t size = lseek(file_descriptor, 0, SEEK_END);     // Gets the size of the file
    char *tmp = NULL;

    if (size > 0) {
        tmp = malloc(size);
        if (tmp == NULL) {
            close(file_descriptor);
            return -1; 
        }
        lseek(file_descriptor, 0, SEEK_SET);    // beginning 
        read(file_descriptor, tmp, size);   // read to tmp 
    }

    
    // Truncate original
    ftruncate(file_descriptor, 0);
    lseek(file_descriptor, 0, SEEK_SET);  


    // Run the command 
    pid_t pid = fork(); 
    if (pid == 0) {
        dup2(file_descriptor, STDOUT_FILENO); 
        close(file_descriptor);

        char *rest_of_str;
        char *command = parse_command(new_command, &rest_of_str);
        char *args[MAX_ARGUMENTS];
        parsing_arguments(rest_of_str, args);

        // Ensure args[0] is the command
        if (args[0] == NULL || strcmp(args[0], command) != 0) {
            for (int i = MAX_ARGUMENTS - 1; i > 0; i--) {
                args[i] = args[i - 1];
            }
            args[0] = command;
        }    

        execvp(command, args);

        myPrint("An error has occurred\n");
        exit(1);
    
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        close(file_descriptor);
        free(tmp);
        return -1;
    }

    // Add stuff from tmp back (e.g. bbb aaa -- the aaa)
    if (tmp != NULL) {
        lseek(file_descriptor, 0, SEEK_END); // Move to the end of the file
        write(file_descriptor, tmp, size);
        free(tmp);
    }

    close(file_descriptor);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

/* ";"
"ls -la; ps; who"
*/
void multiple_commands(char* cmd_buff) {
    char *pointer_to_rest; 
    
    char *first_cmd = strtok_r(cmd_buff, ";", &pointer_to_rest);
    // First command printed, but will be updated as the "first" command of 
    // pointer to rest 

    while(first_cmd != NULL) {

        char *trimmed_cmd = trim_spaces(first_cmd); 

        if (strlen(trimmed_cmd) == 0) {
            first_cmd = strtok_r(NULL, ";", &pointer_to_rest);
            continue;
        }


        if (strstr(trimmed_cmd, ">+") != NULL) {
            int status_of_redirection = do_advanced_redirection(trimmed_cmd);
            if (status_of_redirection == -1) {
                myPrint("An error has occurred\n");
            }
        } else if (strchr(trimmed_cmd, '>') != NULL) {
            int status_of_redirection = do_redirection(trimmed_cmd);
            if (status_of_redirection == -1) {
                myPrint("An error has occurred\n");
            }
        } else {

        ///////////////////////////////////////////
            char *rest_of_str; 
            char *command = parse_command(trimmed_cmd, &rest_of_str);  

            if (command != NULL) { 
                char *args[MAX_ARGUMENTS];
                int argc = parsing_arguments(rest_of_str, args);

                if (strcmp(command, "exit") == 0) {
                    if (argc > 0) {
                        myPrint("An error has occurred\n");
                    } else {
                        exit(0);
                    }
                    

                } else if (strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0) { \
                    builtin_commands(command, args);


                } else {
                    run_command(command, args);
                }
            }
        }

        // Get the next command
        first_cmd = strtok_r(NULL, ";", &pointer_to_rest);  
    }
} 

////////////////////////////////////////////////////////////////////////////////


void do_batch_file(const char *files) { 
    FILE *open_batch = fopen(files, "r");
    if (open_batch == NULL) {
        myPrint("An error has occurred\n");
        exit(1);
    }

    char cmd_buff[514];

    while (fgets(cmd_buff, sizeof(cmd_buff), open_batch)) {

        if (cmd_buff[strlen(cmd_buff) - 1] != '\n' && !feof(open_batch)) {
            myPrint(cmd_buff);

            char extra_buff[514];
            while (fgets(extra_buff, sizeof(extra_buff), open_batch)) {
                myPrint(extra_buff); 

                if (strchr(extra_buff, '\n') != NULL) {
                    break;  
                }
            }

            myPrint("An error has occurred\n");

            continue;
        }


        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // ONLY CHECK IF IT'S BLANK, MULTIPLE COMMMANDS TRIM 
        int is_blank = 1; 
        for (int i = 0; cmd_buff[i] != '\0'; i++) {
            if (!isspace(cmd_buff[i])) {
                is_blank = 0;
                break;
            }
        }
        if (is_blank) {
            continue;
        }

        myPrint(cmd_buff); 
        myPrint("\n"); 

        multiple_commands(cmd_buff); 
    }

    fclose(open_batch);
    exit(0);
}


int main(int argc, char *argv[]) 
{
    char cmd_buff[514];    // fgets 512 + \n + '\0' 
    char *pinput;

    
    // Interactive mode 
    if (argc == 1) { 
        while (1) {
            myPrint("myshell> ");

            pinput = fgets(cmd_buff, 514, stdin);
            if (!pinput) {
                exit(0);
            }

            if (cmd_buff[strlen(cmd_buff) - 1] != '\n') {
                myPrint("An error has occurred\n");

                clear_rest();
                continue; 
            }

            cmd_buff[strcspn(cmd_buff, "\n")] = '\0';  
            multiple_commands(cmd_buff);
        }
    } else if (argc == 2) {      // Batch mode 
        do_batch_file(argv[1]);
    
    } else {
        // Having 2 or more arguments. E.g. ./myshell batchfile abc
        myPrint("An error has occurred\n");
        exit(1);
    }


    return 0; 
}