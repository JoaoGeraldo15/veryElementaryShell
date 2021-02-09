#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

//Consts
#define READ_END 0
#define WRITE_END 1
#define TRUE 1
#define FALSE 0
#define MAX_COMMAND_LINE_SIZE 256
#define MAX_STRING_SIZE 64

//TAD STRING LIST
typedef struct StringListNode{
    char string[MAX_STRING_SIZE];
    struct StringListNode * next;
}StringListNode;

typedef struct StringList{
    StringListNode * first;
    StringListNode * last;
}StringList;

StringList * createStringList(){
    StringList * stringList = (StringList *)malloc(sizeof(StringList));
    if(stringList == NULL){
        printf("Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    stringList->first = NULL;
    stringList->last = NULL;
    return stringList;
}

void insertStringList(StringList * stringList, char *string){
    StringListNode * stringListNode = (StringListNode *)malloc(sizeof(StringListNode));
    if(stringListNode == NULL){
        printf("Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    strcpy(stringListNode->string, string);
    stringListNode->next = NULL;
    if(stringList->first == NULL){
        stringList->first = stringListNode;
        stringList->last = stringListNode;
    }else{
        stringList->last->next = stringListNode;
        stringList->last = stringListNode;
    }
}


void makeStringListEmpty(StringList * stringList){
    StringListNode * aux;
    StringListNode * aux2;
    aux = stringList->first;
    aux2 = stringList->first->next;
    while (aux2 != NULL){
        free(aux);
        aux = aux2;
        aux2 = aux->next;
    }
    stringList->first = NULL;
    stringList->last = NULL;
}

//TAD lista de Argumentos
typedef struct CommandType{
    char command[MAX_STRING_SIZE];
    StringList * arguments;
    int argc;
    int direcionamento;
}CommandType;

typedef struct CommandTypeNode{
    CommandType command;
    struct CommandTypeNode * next;
}CommandTypeNode;

typedef struct CommandList{
    CommandTypeNode *first;
    CommandTypeNode *last;
}CommandList;

CommandList * createCommandList(){
    CommandList * commandList = (CommandList *)malloc(sizeof(CommandList));
    if(commandList == NULL){
        printf("Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    commandList->first = NULL;
    commandList->last = NULL;
    return commandList;
}

void insertCommandList(CommandList * commandList, CommandType command){
    CommandTypeNode * commandTypeNode = (CommandTypeNode *)malloc(sizeof(CommandTypeNode));
    if(commandTypeNode == NULL){
        printf("Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    commandTypeNode->command = command;
    commandTypeNode->next = NULL;
    if(commandList->first == NULL){
        commandList->first = commandTypeNode;
        commandList->last = commandTypeNode;
    }else{
        commandList->last->next = commandTypeNode;
        commandList->last = commandTypeNode;
    }
}


//Functions
CommandList * getListOfCommands(char * inputString){
    int newCommand = 0;
    CommandList * commandList = createCommandList();
    CommandType command;
    command.argc = 0;
    command.arguments = createStringList();
    int count = 0;
    char *token = strtok(inputString, " ");

    // Retirando a quebra de linha da string
    while (count < strlen(inputString)) {
        if (inputString[count] == '\n') { // Encontrou a quebra de linha
            inputString[count] = '\0';
            // printf("Tem quebra de linha\n");
        }
        count ++;
    }
    

    while (token != NULL){  
        token[strlen(token)] = '\0';  

        if(strcmp(token, "&&") == 0 || strcmp(token, "|") == 0 || strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
            newCommand = 0;
            insertCommandList(commandList, command);
            command.arguments = createStringList();

        }else if(strcmp(token, "sair") == 0){
            exit(EXIT_SUCCESS);

        }else if(newCommand == 0){
            strcpy(command.command,token);
            insertStringList(command.arguments,token);
            command.direcionamento = 0;
            command.argc = 1;
            newCommand = 1;

        }else{
            insertStringList(command.arguments,token);
            command.argc += 1;
            newCommand = 1;
        }

        token = strtok (NULL," \n");
    }
    if(newCommand == 1){
        insertCommandList(commandList, command);
    }
    return commandList;
}

int main(){
    char *inputString = malloc(MAX_COMMAND_LINE_SIZE * sizeof(char));
    int flagSair = FALSE;
    CommandList * commandList;
    CommandTypeNode * aux;
    StringListNode * aux2;

    while (flagSair == FALSE){
        printf("<VESH>: ");
        fgets(inputString, MAX_COMMAND_LINE_SIZE, stdin);

        //RETORNA UMA LISTA COM OS COMANDOS
        commandList = getListOfCommands(inputString);
        
        //APENAS PARA DEBUGAR
        aux = commandList->first;
        while (aux != NULL)
        {
            printf("Comando: %s\nargc: %d\ndirecionamento %d\nArgumentos: ", aux->command.command, aux->command.argc, aux->command.direcionamento);
            aux2 = aux->command.arguments->first;
            while(aux2 != NULL){
                printf("%s ",aux2->string);
                aux2 = aux2->next;
            }
            printf("\n\n");
            aux = aux->next;
        }
        
    }

    return EXIT_SUCCESS;
}