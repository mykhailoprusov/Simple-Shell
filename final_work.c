#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/stat.h>


#define MAX_INPUT_LENGTH 512


char original_path[MAX_INPUT_LENGTH]; // Global variable to store original PATH

typedef struct History {
		char* commandsTyped[20][50]; // array of arrays where each array is a command
		int r; // rear
		int f; // front
		int counter; // total number of nodes created (just for the code logic:front will be always the next after the rear if the counter is greater than 20)
	} History;

// Data structure to store aliases
typedef struct Aliases {
	char* aliasesStored[10][51]; // 10 alliases = each elemnt of the array is an allias + stored command for it. First element in the array is the allias and the rest is the command alliased
	int counter; // variable to keep track of the number of the elements in the array
} Aliases;
void display_Prompt() {
    printf("Enter Prompt> ");  
}


void add_to_history(History* history, char** thecommand){

	int i = 0;
	
	while(thecommand[i] != NULL){
		char* temp;
		temp = malloc((strlen(thecommand[i]) * sizeof(char)) +1);
		strcpy(temp, thecommand[i]);
		
		if(history->commandsTyped[history->r][i] != NULL){
			int k = 0;
			// if you go into cycle, and there are history command stored
			// we need to free them
			// we use while loop to continuasly free the whole array of commands stored
			while(history->commandsTyped[history->r][k] != NULL){
				free(history->commandsTyped[history->r][k]);
				history->commandsTyped[history->r][k] = NULL;
				k++;
			}
		}
		history->commandsTyped[history->r][i] = temp;
		
		
		i++;
	}


	// update the front only when you go into cycle ( exceed 20 elements in the history array)
	// then the front must be moved further
	if(history->counter >= 20){
		history->f = (history->r +1)%20;
	}
	
	history->r = (history->r+1)%20;
	history->counter = history->counter+1;

	


}
// function for execution of commands
void execute_command(char **thecommand) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed.\n");
    } else if (pid == 0) {
        execvp(thecommand[0], thecommand);
        perror(thecommand[0]);
        exit(1);
    } else {
        wait(NULL);
    }
}
// invoke command from history
void invoke_command(char **thecommand,History* history){ 
	if(thecommand[0][1] == '-' ){ //!-5 !-12 if the command is to invoke last added - number provided
			char nums[50];
			
			// move numeric contents of the user input into a buffer 
			for(int j = 2; j<strlen(thecommand[0]); j++){
				
				nums[j-2]= thecommand[0][j];
			}
			nums[strlen(nums)+1] = '\0';
			// convert the numeric value provided by user into the inter from string
			int destination = (history->counter < 20) ? history->r - atoi(nums) : 20 - atoi(nums);
			
			// given that the front may vary because of cycles as well as having array that starts from
			// zero index but the user input would be not from 0 but from 1
			// we use this loop which takes into consideration these facts and iterates until it 
			// find the necessary entry
			// it then fills in the thecommand array with the correct values from the history
			
			int counter2 = history->f;
			for(int i =1; i<21;i++){
				if(i == destination){
					
					int j = 0;
					while(history->commandsTyped[counter2][j] != NULL){
						thecommand[j] = history->commandsTyped[counter2][j];
						j++;
					}
					thecommand[j] = NULL;
					
					break;
				}
				counter2 = (counter2 + 1) % 20;
			}
		}
	
	else if(strlen(thecommand[0]) == 2 && thecommand[0][1] == '!'){ // !! invoke last command code
			// iterates over the last element which is rear - 1 because rear indicates the next one to be
			// filled in
	        	int j = 0;
			while(history->commandsTyped[history->r -1][j] != NULL){
				thecommand[j] = history->commandsTyped[history->r -1][j];
				j++;
				}
			thecommand[j] = NULL;
		
	}
	else if(isdigit(thecommand[0][1]) ){ //!2 !19 if the command is to invoke the !n's n's command from history
			char nums[50];
			
			
			for(int j = 1; j<strlen(thecommand[0]); j++){
				
				nums[j-1]= thecommand[0][j];
			}
			nums[strlen(nums)+1] = '\0';
			int destination = atoi(nums);
			int counter2 = history->f;
			for(int i =1; i<21;i++){
				if(i == destination){
					int j = 0;
					while(history->commandsTyped[counter2][j] != NULL){
						thecommand[j] = history->commandsTyped[counter2][j];
						j++;
					}
					thecommand[j] = NULL;
					
					break;
				}
				counter2 = (counter2 + 1) % 20;
			}
	}
	else{
		printf("Error: Invalid input for invoking commands from the history\n");
	}
	
}
// this function checks the command for invoking history is valid
int checker(char **thecommand, History* history){
	if(thecommand[0][1] == '-' && thecommand[1] == NULL){ //!-5 !-12 commands
			char nums[50];
			// if the user typed !- ( so the length is 2) inform that there are not enough arguments
			if(strlen(thecommand[0]) == 2){
				printf("Invalid input !- . Please, supply the number to substract\n");
				return 0;
			} 
			
			// check if the rest of the command after !- are digits
			for(int j = 2; j<strlen(thecommand[0]); j++){
				
				if(!isdigit(thecommand[0][j])){
					printf("Invalid input for the !- command. The values after '!-' must be digits\n");
					return 0;
					
				}
			}
			
			// as before, we convert the string to the integer by putting string into buffer nums and 
			// then converting using atoi()
			for(int j = 2; j<strlen(thecommand[0]); j++){
				
				nums[j-2]= thecommand[0][j];
			}
			nums[strlen(nums)+1] = '\0';
			int destination = (history->counter < 20) ? history->r - atoi(nums) : 20 - atoi(nums);
			if((destination <= history->counter) && destination <= 20){
				return 1;
			}
			
			else{
				printf("Number for the history invocation out of bound\n");
				return 0;
			}

		}
	
	else if(strlen(thecommand[0]) == 2 && thecommand[0][1] == '!' && thecommand[1] == NULL){ // !! command
			return 1;
	}
	else if(isdigit(thecommand[0][1]) && thecommand[1] == NULL){ //!2 !19  commands  
			char nums[50];
			// check if the values for the command !<number> supplied, are digits
			for(int j = 1; j<strlen(thecommand[0]); j++){
				
				if(!isdigit(thecommand[0][j])){
					printf("Invalid input for the !<number> command. The values after '!' must be digits\n");
					return 0;
					
				}
			}
			// put string digits into the buffer
			for(int j = 1; j<strlen(thecommand[0]); j++){
				
				nums[j-1]= thecommand[0][j];
			}
			nums[strlen(nums)+1] = '\0';
			int destination = atoi(nums); // get the actual number supplied
			// if the command is !0 , discard it as we start from !1
			if(destination == 0){
				printf("Number for the history invocation out of bound\n");
				return 0;
			}
			if((destination <= history->counter) && destination <= 20){
				return 1;
			}
			else{
				printf("Number for the history invocation out of bound\n");
				return 0;
			}
			
	}
	else if(thecommand[0][0] == '!'){
		printf("Invalid input for the history invocation. Please, supply the number\n");
		return 0;
	}
	else{
		printf("Invalid input\n");
		return 0;
		}
	
	
}
// this is a function to check if alias was alredy stored (used in the case if you load aliases from the file)
void checkIfAliasStored(Aliases* aliases, char **thecommand, int* position){
    
	
	for(int i = 0; i < aliases->counter; i++){
			if(strcmp(aliases->aliasesStored[i][0], thecommand[0]) == 0){
				*position = i;
			}
	

		}

}

void invokeAlias(char** thecommand, Aliases* aliases, int position){


	int j = 0;

    for (int i = 0; i < 10; i++) {
        if (i == position) {
            int k = 1; // Start copying from index 1 of aliases->aliasesStored[position] because the first element there is a name of the alias not the actual value
            while (aliases->aliasesStored[position][k] != NULL) {
                thecommand[j++] = aliases->aliasesStored[position][k++];
            }
            thecommand[j] = NULL;
            break;
        }
    }	
}
// this is a function to check if alias was alredy stored (not for loadAliases case)
void check_dublicats(Aliases* aliases, char **thecommand, int* position){
    
	
	for(int i = 0; i < aliases->counter; i++){
			if(strcmp(aliases->aliasesStored[i][0], thecommand[1]) == 0){
				*position = i;
			}
	

		}

}

void add_alias(Aliases* aliases, char **thecommand, int position, int loadAliasesMode) {
	if(loadAliasesMode == 0){
		if(position == -1){
			int i = 0;
			
			while(thecommand[i] != NULL){
				char* temp;
				// need to malloc the user unput to store that 
				temp = malloc((strlen(thecommand[i]) * sizeof(char)) +1);
				strcpy(temp, thecommand[i]);
				

				// check if the entry is "alias", if yes , then skip it
				if(strcmp(temp, "alias") == 0){
					i++;
					continue;
				}
				aliases->aliasesStored[aliases->counter][i-1] = temp;
				
				i++;
			}
			aliases->counter = aliases->counter + 1;
			printf("Alias was added\n");
		}
		else{
			printf("Such alias already exists. Overwriting it.\n");
			
			int i = 0;
			
			while(thecommand[i] != NULL){
				char* temp;
				temp = malloc((strlen(thecommand[i]) * sizeof(char)) +1);
				strcpy(temp, thecommand[i]);
				if(aliases->aliasesStored[position][i] != NULL){ // free the memory of previous alias to overwrite it
					
					int k = 0;
					while(aliases->aliasesStored[position][k] != NULL){
						free(aliases->aliasesStored[position][k]);
						aliases->aliasesStored[position][k]= NULL;
						k++;
					}
				}
				
				if(strcmp(temp, "alias") == 0){
					i++;
					continue;
				}
				aliases->aliasesStored[position][i-1] = temp;
				
				
				i++;
			}
		
		}
	}
	else{
		if(position == -1){
			int i = 0;
			
			while(thecommand[i] != NULL){
				char* temp;
				temp = malloc((strlen(thecommand[i]) * sizeof(char)) +1);
				strcpy(temp, thecommand[i]);
				

				aliases->aliasesStored[aliases->counter][i] = temp;
				
				i++;
			}
			aliases->counter = aliases->counter + 1;
			
		}
		else{
			printf("Such alias already exists. Overwriting it.\n");
			
			int i = 0;
			
			while(thecommand[i] != NULL){
				char* temp;
				temp = malloc((strlen(thecommand[i]) * sizeof(char)) +1);
				strcpy(temp, thecommand[i]);
				if(aliases->aliasesStored[position][i] != NULL){
				
					int k = 0;
					while(aliases->aliasesStored[position][k] != NULL){
						free(aliases->aliasesStored[position][k]);
						aliases->aliasesStored[position][k]= NULL;
						k++;
					}
				}
				

				aliases->aliasesStored[position][i] = temp;
				
				
				i++;
			}
		
		}
	
	}


}

int parse_Input(char *input, char **thecommand,History* history, Aliases* aliases, int loadAliases) {
    char *token = strtok(input, " \t|><&;");
    int counter = 0;
    
    while (token != NULL) {
        thecommand[counter] = token;
        counter++;
        token = strtok(NULL, " \t|><&;");
    }
    thecommand[counter] = NULL;
	int flag = 1;
	if(thecommand[0] != NULL && thecommand[0][0] == '!'){ // invoke commands from history if the checker passed
		if(checker(thecommand,history)){
			invoke_command(thecommand,history); 
		}
		else{
			flag = 0; // if don't pass the checker return flag 0
		}               
	}                                                     
	else if(thecommand[0] != NULL && strcmp(thecommand[0], "exit") != 0 && loadAliases == 0){
		add_to_history(history, thecommand);
	}
	
	if(thecommand[0] != NULL && thecommand[1] == NULL && loadAliases == 0){
		int position = -1;
		checkIfAliasStored(aliases, thecommand, &position);
		if(position != -1){
			invokeAlias(thecommand,aliases,position);
		}
	}
	if(loadAliases){ // if it loads the aliases, then it adds them into the the data structre after parsing
	    if(aliases->counter == 10){
		    printf("Aliases storage is full. No more aliases can be set\n");
		}		
	    else{
	    	    
		    int position = -1;
		    check_dublicats(aliases, thecommand, &position);
		    add_alias(aliases, thecommand, position,1);
            }
	}
	
	
	return flag;
}

void print_history(History* history) {

    int count = history->counter;
    int effectiveSize = (count < 20) ? count : 20;
    int counter2 = history->f;
    int currentNumber = 1;
    
	
	for(int i = 0; i < effectiveSize; i++){
		printf("%d. ",currentNumber);
		for (int j = 0; j < 50; j++) {	
        		if (history->commandsTyped[counter2][j] != NULL) {
        			
            			printf("%s ", history->commandsTyped[counter2][j]);
        		}
        		
        		
    		}
	    counter2 = (counter2 + 1) % 20;	
	    currentNumber++;	
	    printf("\n");
		}
}

void save_history(History* history){
	char* home_directory = getenv("HOME");
	
	if(home_directory == NULL){
		perror("Error getting home directory. \n");
		return;
	}
	
	// create path for file
	char* history_file_path = malloc(strlen(home_directory) + strlen("/hist_list") + 1);
	strcpy(history_file_path, home_directory);
	strcat(history_file_path, "/hist_list");
	
	FILE *file = fopen(history_file_path, "w"); //overwrites, if there is an existing file
	
	int count = history->counter;
    	int effectiveSize = (count < 20) ? count : 20;
    	int counter2 = history->f;
	if (file != NULL) {

	for(int i = 0; i < effectiveSize; i++){

		for (int j = 0; j < 50; j++) {	
        		if (history->commandsTyped[counter2][j] != NULL) {
        			
            			fprintf(file, "%s ", history->commandsTyped[counter2][j]);
        		}
        		
        		
    		}
    	    fprintf(file, "\n");
	    counter2 = (counter2 + 1) % 20;	

		}

        	fclose(file);
        	printf("History saved to %s. \n", history_file_path);
    	} else {
    		perror("Error creating save file. \n");
	}
    	
    	free(history_file_path);

}

void load_history(History* history, Aliases* aliases){
	char* home_directory = getenv("HOME");
	
	
	if (home_directory == NULL) {
		perror("Error getting home directory path. \n");
		return;
	}
	
	// create path for file
	char* history_file_path = malloc(strlen(home_directory) + strlen("/hist_list") + 1);
	strcpy(history_file_path, home_directory);
	strcat(history_file_path, "/hist_list");

	

	FILE* file = fopen(history_file_path, "r");
	
	if (file != NULL){

		
		char buffer[MAX_INPUT_LENGTH];


	    	while (fgets(buffer, sizeof(buffer), file) != NULL) {
	    		char* thecommandForHistory[50];
	    		buffer[strcspn(buffer, "\n")] = '\0';
	    		// we use parse input function to parse the data from the file and put 
	    		// everything into the history ( there is an add_history() func invocation there
	    		// which would correctly handle it.
		        parse_Input(buffer, thecommandForHistory, history,aliases,0);
		        	

		    }	

		
		fclose(file);
		printf("History loaded. \n");
	} else {
		// create file if one doesn't exist
		file = fopen(history_file_path, "w");
		if (file != NULL) {
			printf("History file did not exist. Creating a new file.\n");
			fclose(file);
		} else {
			perror("Error loading history file. \n");
		}
	}
	
    	free(history_file_path);
	
}
void load_aliases(History* history, Aliases* aliases){
	char* home_directory = getenv("HOME");
	
	
	if (home_directory == NULL) {
		perror("Error getting home directory path. \n");
		return;
	}
	
	// create path for file
	char* aliases_file_path = malloc(strlen(home_directory) + strlen("/aliases") + 1);
	strcpy(aliases_file_path, home_directory);
	strcat(aliases_file_path, "/aliases");

	

	FILE* file = fopen(aliases_file_path, "r");
	
	if (file != NULL){

		
		char buffer[MAX_INPUT_LENGTH];


	    	while (fgets(buffer, sizeof(buffer), file) != NULL) {
	    		char* thecommandForAliases[50];
	    		buffer[strcspn(buffer, "\n")] = '\0';
	    		// we use parse input function with loadAliases mode set to 1
	    		// it wil parse the input from the file and then add to the aliases data structure
	    		// forth argument (1) will indicate that the input must be put into the aliases and
	    		// do not need to be put into the history 
		        parse_Input(buffer, thecommandForAliases, history,aliases,1);
		        	

		    }	

		
		fclose(file);
		printf("Aliases loaded. \n");
	} else {
		// create file if one doesn't exist
		file = fopen(aliases_file_path, "w");
		if (file != NULL) {
			printf("Aliases file did not exist. Creating a new file.\n");
			fclose(file);
		} else {
			perror("Error loading aliases file. \n");
		}
	}
	
    	free(aliases_file_path);
	
}

void save_aliases(History* history, Aliases* aliases){
	char* home_directory = getenv("HOME");
	
	if(home_directory == NULL){
		perror("Error getting home directory. \n");
		return;
	}
	
	// create path for file
	char* aliases_file_path = malloc(strlen(home_directory) + strlen("/aliases") + 1);
	strcpy(aliases_file_path, home_directory);
	strcat(aliases_file_path, "/aliases");
	
	FILE *file = fopen(aliases_file_path, "w"); //overwrites, if there is an existing file
	

	if (file != NULL) {

	for(int i = 0; i < aliases->counter; i++){

		for (int j = 0; j < 51; j++) {	
        		if (aliases->aliasesStored[i][j] != NULL) {
        			
            			fprintf(file, "%s ", aliases->aliasesStored[i][j]);
        		}
        		
        		
    		}
    	    fprintf(file, "\n");
	    	

		}

        	fclose(file);
        	printf("Aliases saved to %s. \n", aliases_file_path);
    	} else {
    		perror("Error creating save file. \n");
	}
    	
    	free(aliases_file_path);

}


void getpath(char **args) {
	if (args[1] != NULL){
		printf("Error no arguments required: getpath\n");
		return;
	}
    printf("%s\n", getenv("PATH"));
}

void setpath(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        printf("Error enter a single parameter: setpath <path>\n");
        return;
    }
    if (access(args[1], F_OK) == -1) {
        perror("setpath");
        return;
    } 
    
    if (setenv("PATH", args[1], 1) != 0) {
        perror("setpath");
    }
}

void save_original_path() {
    char *path = getenv("PATH");
    strcpy(original_path, path);
}

void restore_original_path() {
    if (setenv("PATH", original_path, 1) != 0) {
        perror("restore_original_path");
    }
}

void set_to_home() {
    char *home = getenv("HOME");
    if (chdir(home) != 0) {
        perror("chdir");
    } else {
        char cwd[MAX_INPUT_LENGTH];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working directory changed to %s\n", cwd);
        } else {
            perror("getcwd");
        }
    }
}


void change_directory(char **args) {
    if (args[1] == NULL) {
        // no parameter, change to the home directory
        set_to_home();
    } else if (args[2] != NULL) {
        printf("Error enter a single parameter: cd <directory>\n");
    } else {
        
        struct stat sb; // lstat will check if there is such a file or directory in the system
        if (lstat(args[1], &sb) == -1) {
            printf("cd: %s: No such file or directory\n", args[1]);
            
        }
	// if it is a file, then discard it and say it is not a directory
        else if (S_ISREG(sb.st_mode)) {
            printf("cd: %s: Not a directory\n", args[1]);
        }
        // if this is a directory then execute following:
        else if(S_ISDIR(sb.st_mode)){
		 if(chdir(args[1]) != 0){
			printf("cd: %s: No such file or directory\n", args[1]);
		}else {
		    char cwd[MAX_INPUT_LENGTH];
		    if (getcwd(cwd, sizeof(cwd)) != NULL) {
		        printf("Current working directory changed to %s\n", cwd);
		    } else {
		        perror("getcwd");
		    }
        	}
        }
    }

}





void remove_alias(Aliases* aliases, char **thecommand, int position) {
	
	
	// free the previous alias ( the position variable points to the exact index of the
	// necessary alias to be removed	
	for(int i= 0; i<51;i++){

		if(aliases->aliasesStored[position][i] != NULL){
			
			free(aliases->aliasesStored[position][i]);
		}			
			
			
		
	}
	

    for (int j = position; j < aliases->counter - 1; j++) {
        for (int k = 0; k < 51; k++) {
            if (aliases->aliasesStored[j + 1][k] != NULL) {
                char* temp = malloc((strlen(aliases->aliasesStored[j + 1][k]) + 1) * sizeof(char));
                if (temp != NULL) {
                    strcpy(temp, aliases->aliasesStored[j + 1][k]);
                    aliases->aliasesStored[j][k] = temp;
                } else {
                    printf("Memory allocation failed while removing alias.\n");
                }
            } else {
                aliases->aliasesStored[j][k] = NULL;
            }
        }
    }
    

    // decrement the number of aliases
    aliases->counter = aliases->counter - 1;
    printf("Alias was removed\n");
    
    
    
	
}



// Function to print all aliases
void print_aliases(Aliases* aliases) {
    if(aliases->counter != 0){

    int currentNumber = 1;
    
	
	for(int i = 0; i < aliases->counter; i++){
		printf("%d. ",currentNumber);
		for (int j = 0; j < 51; j++) {	
        		if (aliases->aliasesStored[i][j] != NULL) {
        			if(j == 0){
        				printf("%s : ",aliases->aliasesStored[i][j]);
        			}
        			else{
            				printf("%s ", aliases->aliasesStored[i][j]);
            			}
        		}
        		
        		
    		}
	
	    currentNumber++;	
	    printf("\n");
		}
	}
    else{
    	printf("The alias list is empty\n");
    }
}

int main() {
    char input[MAX_INPUT_LENGTH];
    save_original_path(); // Save original PATH at the start
    	// setting up history & aliases data structures
	History history = {.r = 0, .f = 0, .counter = 0};
	for(int i = 0; i<20; i++){
		for(int j = 0; j<50; j++){
			history.commandsTyped[i][j] = NULL;
		
		}
	}
	
	Aliases aliases = {.counter = 0};
	
	for(int i = 0; i<10; i++){ 
		for(int j = 0; j<51; j++){
			aliases.aliasesStored[i][j] = NULL;
		
		}
	}
	
	 
    
    load_history(&history, &aliases);
    load_aliases(&history, &aliases);
    
    while (1) {
        display_Prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nExiting the shell.\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        char* thecommand[50];
        
	// flag is used for the invoke_command() (!2, !-5, !! commands for invocation from history) 
	// if the command was !!,!5,!-2, it is used to indicate if it passed checker and if not then it will not go into
	// execution by execute_command() because the checker() will print valid errors and we don't want execute_command to 
	// print its own error message and basically execute this command  
        int flag = parse_Input(input, thecommand, &history, &aliases,0);
        
        // if the flag is 0, then don't execute it, there was an error with the !5 command and it has been already handled
	if(thecommand[0] == NULL ||  !flag){
		continue;
	}
        else if (strcmp(thecommand[0], "exit") == 0) {
            printf("Exiting the shell.\n");
            break;
        } else if (strcmp(thecommand[0], "getpath") == 0) {
            getpath(thecommand);
        } else if (strcmp(thecommand[0], "setpath") == 0) {
            setpath(thecommand);
        } else if (strcmp(thecommand[0], "cd") == 0) {
            
            change_directory(thecommand);
        } else if (strcmp(thecommand[0], "history") == 0 && thecommand[1] == NULL) {
            print_history(&history);
        } else if (strcmp(thecommand[0], "alias") == 0) {
            // if there is nothig, just 'alias' command, then print aliases	
            if(thecommand[1] == NULL){
            	print_aliases(&aliases);
            }
            // but if the name for the alias provided without command to be aliases, 
            // tell user that there are not enough arguments to set the alias
            else if(thecommand[1] != NULL && thecommand[2] == NULL){
            	printf("Not enough arguments to set the alias\n");
            }
            else {
                // check_dublicats checks if the alias to be added has been already added
                // if position == -1 , it means that the alias to be added is new and can be added
                // so if the array is full and the alias to be added is not stored ( can't be overwritten ) 
                // then discard it and print that sorage is full
                int position = -1;
		check_dublicats(&aliases, thecommand, &position);
            	if(aliases.counter == 10 && position == -1){
			printf("Aliases storage is full. No more aliases can be set\n");
		
		}
	    	else{
		    int position = -1;
		    check_dublicats(&aliases, thecommand, &position);
		    // we pass 0 as forth argument to indicate that the add alias is not
		    // in the loadAliases mode 
		    // the difference between modes is that if this is loading of aliases,
		    // then there is no command entry 'alias' because aliases storage format is <alias name> <command> <command>
		    add_alias(&aliases, thecommand, position,0);
            	}
            }
        } else if (strcmp(thecommand[0], "unalias") == 0) {
            if(thecommand[1] == NULL){
		printf("Not enough arguments for unalias command supplied\n");            
            }
            else{   // we use check_dublicats function here to find if the alias to be unaliased is stored
                    // if position == -1 , it means that the alias requested to be unaliased is not in the database
                    // so it can't be unaliased
		    int position = -1;
		    check_dublicats(&aliases, thecommand, &position);
		    if(position == -1){
		    	printf("Alias for unaliasing not found\n");
		    }
		    else{
		    	remove_alias(&aliases,thecommand, position);
		    }
            }
        } else {	

        	execute_command(thecommand);
    
        }
        

    }
    save_history(&history);
    save_aliases(&history,&aliases);
    
    restore_original_path(); // Restore original PATH before exiting
    printf("Restored original PATH: %s\n", original_path);
    return 0;
}
