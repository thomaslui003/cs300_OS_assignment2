//project 2 cshell
//github name: thomaslui003


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

typedef struct
{

    char *name;
    char *value;
} EnvVar;

typedef struct
{
    char *name;
    struct tm time;
    int value;
} TheCommand;

void non_builtIn(char *command, TheCommand *commandArray, int *numCommand);
void built_in(char **arg, EnvVar *envArray, int *numofVar, TheCommand *commandArray, int *numCommand);
void readingInput(char command[], char *restOfInput[]);
void scriptmode(FILE *fp); 
char *getSubstring(const char *startpattern, const char *endpattern, char *inputline);


int main(int argc, char *argv[])
{

    void addingEnvVar(EnvVar * envArray, EnvVar a, int *numberofItem);
    void addingCommandVar(TheCommand * commandArray, TheCommand a, int *numbCommand);
    FILE *fp;
    char line[256];
    int mode;                        // mode 1 is interactive and mode 2 is script
    EnvVar environmentVarArray[100]; // the struct environment array
    int envVarNum = 0;
    TheCommand commandArray[100];
    int commandNum = 0;

    if (argc == 2) //if there are 2 argument passed in then it should be script mode else if only one then it's interactive mode
    {

        mode = 2; // mode is scriptmode
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {

            printf("Unable to read script file: %s\n", argv[1]);
            exit(0);
        }
        else
        {
            scriptmode(fp);
            fclose(fp);
            // printf("exiting from the file script mode\n");
            printf("\033[0m");
            exit(0);
        }
    }
    else if (argc == 1)
    {
        mode = 1; // mode is interactive
    }
    else
    {
        printf("error, user entered too many argument for starting the program\n"); /////// dont know if neeed for this
        exit(0);
    }

    while (1)
    {

        printf("cshell$ ");
        // fgets(line, sizeof line, stdin);
        // printf("%s\n",line);

        char command[100];
        char *restOfInput[20];

        readingInput(command, restOfInput); // restOfInput[0] is still the command therefore it's entire input in array

        // printf("second input is: %s\n",restOfInput[1]);

        // printf("the command is: %c\n", command[1]);

        if (strcmp(command, "exit") == 0)
        {
            printf("Bye!\n");
            printf("\033[0m");
            // break;
            exit(0);
        }

        // char *arg_pwd[] = {"pwd", NULL};
        // char *arg_whoami[] = {"whoami", NULL};

        //check if input is print or log or theme and if so, execute the builtin function 
        if ((strcmp(command, "print") == 0) || (strcmp(command, "log") == 0) || (strcmp(command, "theme") == 0))
        {
            // added the env array here for printing var value
            built_in(restOfInput, environmentVarArray, &envVarNum, commandArray, &commandNum); // restOfInput is an array of the user input and it worked with print function you wrote.
            continue;
        }
        else if (((strcmp(command, "pwd") == 0) || (strcmp(command, "ls") == 0) || (strcmp(command, "whoami") == 0)) && restOfInput[1] == NULL)
        {
            //check if input is either pwd, ls, whoami and use pipe from child to parent send TheCommand struct and add it to the array
            int childtoParent[2];
            pipe(childtoParent);
            int fc = fork();

            if (fc < 0)
            {
                printf("fork function failed\n");
                // exit(1);
            }
            else if (fc == 0)
            {
                // in child process
                //  executing the non-built-in commands (ls, pwd, whoami)
                char *tempV = strdup(command);
                //printf("the command is this: %s\n", tempV);
                time_t rawtime;
                time(&rawtime);
                

                if (strcmp(command, "pwd") == 0)
                {
                    TheCommand newExecutedVar = {"pwd", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else if (strcmp(command, "ls") == 0)
                {
                    TheCommand newExecutedVar = {"ls", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else if (strcmp(command, "whoami") == 0)
                {
                    TheCommand newExecutedVar = {"whoami", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else
                {
                }

                // close(childtoParent[0]);
                // write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                // close(childtoParent[1]);
                //printf("it is prob gonna execute nonbuiltin\n");
                non_builtIn(command, commandArray, &commandNum);
            }
            else
            {
                // in parent process
                wait(NULL);
                // addingCommandVar(commandArray, newExecutedVar, numCommand);
                close(childtoParent[1]);
                TheCommand newExecutedVar;
                read(childtoParent[0], &newExecutedVar, sizeof(TheCommand));
                // printf("the post piped result is: %s\n",asctime(&newExecutedVar.time));
                //printf("the post piped result is: <%d>\n", newExecutedVar.value);
                addingCommandVar(commandArray, newExecutedVar, &commandNum);
                close(childtoParent[0]);
            }
        }
        else if (command[0] == '$') //if the input start with $ and see if it's updating variable or setting new variable
        {
            int tempVarSucc;

            if (restOfInput[1] == NULL)
            {

                char *theVar = getSubstring("$", "=", command);
                char *newvalue = strstr(command, "=") + 1;
                
                EnvVar newVar = {strdup(theVar), strdup(newvalue)}; // the user wanted new environment variable and created with the extracted string of the format ($Var=value)
                
                // check if the variable exist in the array, if existed then just assign the new value to it
                //printf("the envVarNum is: %d\n", envVarNum);
                if (envVarNum > 0) //check if the variable already exist and possibly update or set new
                {
                    int foundedSameVar = 0;
                    for (int x = 0; x < envVarNum; x++)
                    {
                        if (strcmp(environmentVarArray[x].name, theVar) == 0) //found existing variable
                        {
                            environmentVarArray[x].value = strdup(newvalue);
                            foundedSameVar = 1;
                            tempVarSucc = 0;
                        }
                    }
                    if (foundedSameVar == 0)
                    {
                        addingEnvVar(environmentVarArray, newVar, &envVarNum); //adding new env variable to array when not found existing
                        tempVarSucc = 0;
                    }
                }
                else
                {
                    addingEnvVar(environmentVarArray, newVar, &envVarNum); //adding new env variable to array when array is empty
                    tempVarSucc = 0;
                }

                //printf("the command is: %c\n", command[0]);
                //printf("the variable name is: %s\n", theVar);
                //printf("the variable value is: %s\n", newvalue);
            }
            else
            {

                // when the user input wrong format for $<var>=<value>
                tempVarSucc = -1;
                printf("Variable value expected\n");
            }
            // dont know if needed for log
            //************************************************ leave out for now
            //  time_t rawtime;
            //  time(&rawtime);
            //  TheCommand newExecutedVar = {strdup(command), *localtime(&rawtime), tempVarSucc};
            //  //printf("the newexecutedvar is: %s\n",newExecutedVar.name);
            //  addingCommandVar(commandArray, newExecutedVar, &commandNum);
        }
        else
        {
            // overall when user input invalid keyword or command
            printf("Missing keyword or command, or permission problem\n");
        }

        
    }

    return 0;
}

void scriptmode(FILE *fp) // void scriptmode(FILE *fp , EnvVar *envArray, int *numofVar)
{
    void addingEnvVar(EnvVar * envArray, EnvVar a, int *numberofItem);
    void addingCommandVar(TheCommand * commandArray, TheCommand a, int *numbCommand);
    char line[1024];                  // maybe memory problem here?
    EnvVar environmentVarArrayS[100]; // the struct environment array
    int envVarNumS = 0;
    TheCommand commandArrayS[100];
    int commandNumS = 0;

    while (fgets(line, sizeof line, fp) != NULL)
    {

        char command[100];
        char *restOfInput[20];

        int i = 0;
        char *array[100];
        char *temp;
        // char line[1024];

        temp = strtok(line, " \n");

        // printf("read this from user: %s\n", temp); // testing output

        if (temp == NULL)
        {
            return;
        }

        while (temp != NULL)
        {
            array[i++] = strdup(temp);
            temp = strtok(NULL, " \n");
        }

        strcpy(command, array[0]);

        for (int x = 0; x < i; x++)
        {
            restOfInput[x] = array[x];
            restOfInput[i] = NULL;
        }

        // after tokenizing the current line

        if (strcmp(command, "exit") == 0)
        {
            printf("Bye!\n");
            printf("\033[0m");
            // break;
            exit(0);
        }

        if ((strcmp(command, "print") == 0) || (strcmp(command, "log") == 0) || (strcmp(command, "theme") == 0))
        {
            built_in(restOfInput, environmentVarArrayS, &envVarNumS, commandArrayS, &commandNumS); // restOfInput is an array of the user input and it worked with print function you wrote.
            // need to add the environment array and the envnumber
            continue;
        }
        else if (((strcmp(command, "pwd") == 0) || (strcmp(command, "ls") == 0) || (strcmp(command, "whoami") == 0)) && restOfInput[1] == NULL)
        {
        
            //check if input is either pwd, ls, whoami and use pipe from child to parent send TheCommand struct and add it to the array
            int childtoParent[2];
            pipe(childtoParent);
            int fc = fork();

            if (fc < 0)
            {
                printf("fork function failed\n");
                // exit(1);
            }
            else if (fc == 0)
            {
                // in child process
                //  executing the non-built-in commands (ls, pwd, whoami)
                char *tempV = strdup(command);
                //printf("the command is this: %s\n", tempV);
                time_t rawtime;
                time(&rawtime);
                

                if (strcmp(command, "pwd") == 0)
                {
                    TheCommand newExecutedVar = {"pwd", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else if (strcmp(command, "ls") == 0)
                {
                    TheCommand newExecutedVar = {"ls", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else if (strcmp(command, "whoami") == 0)
                {
                    TheCommand newExecutedVar = {"whoami", *localtime(&rawtime), 0};
                    close(childtoParent[0]);
                    write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                    close(childtoParent[1]);
                }
                else
                {
                }

                // close(childtoParent[0]);
                // write(childtoParent[1], &newExecutedVar, sizeof(TheCommand));
                // close(childtoParent[1]);
                //printf("it is prob gonna execute nonbuiltin\n");
                non_builtIn(command, commandArrayS, &commandNumS);
            }
            else
            {
                // in parent process
                wait(NULL);
                // addingCommandVar(commandArray, newExecutedVar, numCommand);
                close(childtoParent[1]);
                TheCommand newExecutedVar;
                read(childtoParent[0], &newExecutedVar, sizeof(TheCommand));
                // printf("the post piped result is: %s\n",asctime(&newExecutedVar.time));
                //printf("the post piped result is: <%d>\n", newExecutedVar.value);
                addingCommandVar(commandArrayS, newExecutedVar, &commandNumS);
                close(childtoParent[0]);
            }



        }
        else if (command[0] == '$')
        {
            int tempVarSucc;

            if (restOfInput[1] == NULL)
            {

                char *theVar = getSubstring("$", "=", command);
                char *newvalue = strstr(command, "=") + 1;
                // printf();
                EnvVar newVar = {strdup(theVar), strdup(newvalue)}; // the user wanted new environment variable and created with the extracted string of the format ($Var=value)
                // addingEnvVar(environmentVarArray,newVar,&envVarNum);

                // check if the variable exist in the array, if existed then just assign the new value to it
                // printf("the envVarNum is: %d\n", envVarNumS);
                if (envVarNumS > 0)
                {
                    int foundedSameVar = 0;
                    for (int x = 0; x < envVarNumS; x++)
                    {
                        if (strcmp(environmentVarArrayS[x].name, theVar) == 0)
                        {
                            environmentVarArrayS[x].value = strdup(newvalue);
                            foundedSameVar = 1;
                            tempVarSucc = 0;
                        }
                    }
                    if (foundedSameVar == 0)
                    {
                        addingEnvVar(environmentVarArrayS, newVar, &envVarNumS);
                        tempVarSucc = 0;
                    }
                }
                else
                {
                    addingEnvVar(environmentVarArrayS, newVar, &envVarNumS);
                    tempVarSucc = 0;
                }

                // printf("the command is: %c\n", command[0]);
                // printf("the variable name is: %s\n", theVar);
                // printf("the variable value is: %s\n", newvalue);
            }
            else
            {

                // when the user input wrong format for $<var>=<value>
                tempVarSucc = -1;
                printf("Variable value expected\n");
            }
            //******************leave out now if variable setting need print log
            // time_t rawtime;
            // time(&rawtime);
            // TheCommand newExecutedVar = {strdup(command), *localtime(&rawtime), tempVarSucc};
            // //printf("settingcompleted\n");
            // addingCommandVar(commandArrayS, newExecutedVar, &commandNumS);
        }
        else
        {
            printf("Missing keyword or command, or permission problem\n");
        }
    }
    printf("Bye!\n");
    return;
}

void non_builtIn(char *command, TheCommand *commandArray, int *numCommand)
{
    void addingCommandVar(TheCommand * commandArray, TheCommand a, int *numbCommand);

    char *arg_pwd[] = {"pwd", NULL};
    char *arg_whoami[] = {"whoami", NULL};
    char *arg_ls[] = {"ls", "-1", NULL};

    if (strcmp(command, "pwd") == 0)
    {

        execvp(command, arg_pwd);
    }
    else if (strcmp(command, "whoami") == 0)
    {

        execvp(command, arg_whoami);
    }
    else if (strcmp(command, "ls") == 0)
    {
        // time_t rawtime;
        // time(&rawtime);
        // TheCommand newExecutedVar = {strdup("ls"), *localtime(&rawtime), 0};
        // addingCommandVar(commandArray, newExecutedVar, numCommand);

        execvp(command, arg_ls);
    }
    else
    {
        return;
    }
}
// function that read in the user input and tokenize it into the first command and the input in array form
void readingInput(char command[], char *restOfInput[])
{

    int i = 0;
    char *array[100];
    char *temp;
    char line[1024];

    fgets(line, sizeof line, stdin);

    temp = strtok(line, " \n");

    // printf("read this from user: %s\n", temp); // testing output

    if (temp == NULL)
    {
        return;
    }

    while (temp != NULL) // while the tokenize process is not empty
    {
        array[i++] = strdup(temp);
        temp = strtok(NULL, " \n");
    }

    strcpy(command, array[0]);

    for (int x = 0; x < i; x++) // make assign array values to restOfInput array
    {
        restOfInput[x] = array[x];
        restOfInput[i] = NULL;
    }
}

// here is the function I worte to read input and prase the input
void read_line(char *str)
{
    int n = 64;

    fgets(str, n, stdin);

    return;
}

char **parse_line(char *str)
{
    int n = 64, i = 0;
    char **commands = malloc(n * sizeof(char *));
    char *com;

    com = strtok(str, " ");
    while (com != NULL)
    {
        commands[i] = com;
        i++;
        com = strtok(NULL, " ");
    }
    commands[i] = NULL;
    return commands;
}

void built_in(char **arg, EnvVar *envArray, int *numofVar, TheCommand *commandArray, int *numCommand)
{
    void addingCommandVar(TheCommand * commandArray, TheCommand a, int *numbCommand);
    char *BIcommands[4];
    BIcommands[0] = "exit";
    BIcommands[1] = "print";
    BIcommands[2] = "theme";
    BIcommands[3] = "log";

    char *colors[3];
    colors[0] = "red";
    colors[1] = "green";
    colors[2] = "blue";

    if (strcmp(arg[0], BIcommands[1]) == 0) // if command is print
    {
        int i = 1;
        int valueSucc;

        while (arg[i] != NULL) // looping through all of the argument and print out each string or values of stored environment variable
        {

            char *printVar = strdup(arg[i]);
            // printf("the print var with $ is: %s\n", printVar);

            if (printVar[0] == '$')
            {

                char *theVarName = strdup(strstr(printVar, "$") + 1);
                // printf("the varName var within print is: %s\n", theVarName);

                // printf("the *numofVar within print is: %d\n", *numofVar);

                if (*numofVar > 0)
                {
                    // printf("the number of variable in array is: %d\n",*numofVar);
                    int founded = 0; // indicate not founded
                    for (int x = 0; x < *numofVar; x++)
                    {
                        char *tempt = strdup(envArray[x].name);
                        if (strcmp(tempt, theVarName) == 0)
                        {
                            // printing the found variable value
                            // printf("after compared for variable thomas: %s\n",envArray[x].name);
                            // printf("after compared for variable thomas: %s\n",theVarName);

                            // printf("the result is this for variable thomas: %s ",envArray[x].value);
                            printf("%s ", envArray[x].value);

                            // printing the value here
                            // printf("the x is: %d\n",x);
                            founded = 1;
                        }
                    }
                    if (founded == 0)
                    {
                        printf("Error: No Environment Variable $%s found\n", theVarName);
                        time_t rawtime;
                        struct tm *info;
                        time(&rawtime);
                        info = localtime(&rawtime);
                        valueSucc = -1;
                        TheCommand newExecutedVar = {strdup("print"), *info, valueSucc};
                        addingCommandVar(commandArray, newExecutedVar, numCommand);
                        return;
                    }
                }
                else
                {
                    // if the env variable is not found in the array
                    printf("Error: No Environment Variable $%s found\n", theVarName);
                    time_t rawtime;
                    struct tm *info;
                    time(&rawtime);
                    info = localtime(&rawtime);
                    valueSucc = -1;
                    TheCommand newExecutedVar = {strdup("print"), *info, valueSucc};
                    addingCommandVar(commandArray, newExecutedVar, numCommand);
                    return;
                }
            }
            else
            {

                // printing statement with this format: print var and result is var where is not
                printf("%s ", arg[i]); // change to the i
                // return;
            }

            i++;

            // if variable not found, printf("Error: No Environment Variable $%s found",)
        }
        printf("\n");
        time_t rawtime;
        struct tm *info;
        time(&rawtime);
        info = localtime(&rawtime);
        valueSucc = 0;
        // printf("the time is: %s\n",asctime(info));

        TheCommand newExecutedVar = {strdup("print"), *info, valueSucc};
        addingCommandVar(commandArray, newExecutedVar, numCommand);
        // printf("the value are %s %d\n",commandArray[0].name,commandArray[0].value);
        // printf("the time are %s\n", asctime(&commandArray[0].time));

       
    }
    else if (strcmp(arg[0], BIcommands[2]) == 0)
    {
        int suc;

        if (arg[1] != NULL)
        {
            if (strcmp(arg[1], colors[0]) == 0)
            {
                printf("\033[0;31m");
                suc = 0;
            }
            else if (strcmp(arg[1], colors[1]) == 0)
            {
                printf("\033[0;32m");
                suc = 0;
            }
            else if (strcmp(arg[1], colors[2]) == 0)
            {
                printf("\033[0;34m");
                suc = 0;
            }
            else
            {
                printf("unsupported theme\n");
                suc = -1;
            }
        }
        else
        {
            suc = -1;
            printf("unsupported theme\n");
        }
        time_t rawtime;
        time(&rawtime);
        TheCommand newExecutedVar = {strdup("theme"), *localtime(&rawtime), suc};
        addingCommandVar(commandArray, newExecutedVar, numCommand);
    }
    else if (strcmp(arg[0], BIcommands[3]) == 0)
    { // if command is log

        int tempSucc;

        if (arg[1] == NULL)
        {
            for (int k = 0; k < *numCommand; k++)
            {
                printf("%s", asctime(&commandArray[k].time));
                printf(" %s %d\n", commandArray[k].name, commandArray[k].value);
            }
            tempSucc = 0;
        }
        else
        {
            tempSucc = -1;
        }
        time_t rawtime;
        time(&rawtime);
        TheCommand newExecutedVar = {strdup("log"), *localtime(&rawtime), tempSucc};
        addingCommandVar(commandArray, newExecutedVar, numCommand);
    }

    else
    {

        if ((strcmp(arg[0], "pwd") != 0) && (strcmp(arg[0], "whoami") != 0) && (strcmp(arg[0], "ls") != 0))
        {
            printf("Missing keyword or command, or permission problem\n");
        }
    }

    return;
}

char *getSubstring(const char *startpattern, const char *endpattern, char *inputline)
{

    const char *s = inputline;
    char *result = NULL;
    const char *matchPatternFront = startpattern;
    const char *matchPatternBack = endpattern;
    char *trackStart;
    char *trackEnd;

    if ((trackStart = strstr(s, matchPatternFront)))
    {
        trackStart += strlen(matchPatternFront);
        if ((trackEnd = strstr(trackStart, matchPatternBack)))
        {
            result = (char *)malloc(trackEnd - trackStart + 1);
            memcpy(result, trackStart, trackEnd - trackStart);
            result[trackEnd - trackStart] = '\0';
        }
    }

    if (result)
        return result;

    free(result);

    return 0;
}

void addingEnvVar(EnvVar *envArray, EnvVar a, int *numberofVar)
{

    if (*numberofVar < 100)
    {
        envArray[*numberofVar] = a;
        *numberofVar += 1;
    }
    // printf("the env is: %s added\n", envArray[*numberofVar - 1].name);
    // printf("the env value is: %s added\n", envArray[*numberofVar - 1].value);
}
void addingCommandVar(TheCommand *commandArray, TheCommand a, int *numbCommand)
{

    if (*numbCommand < 100)
    {
        commandArray[*numbCommand] = a;
        *numbCommand += 1;
    }
    // printf("the env is: %s added\n", commandArray[*numbCommand - 1].name);
    // printf("the env value is: %d added\n", commandArray[*numbCommand - 1].value);
}
