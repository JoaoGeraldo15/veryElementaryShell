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
// #define MAX_STRING_SIZE 64
#define MAX_STRING_SIZE 2048
#define READ_END 0
#define WRITE_END 1


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
        fprintf(stderr, "Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    stringList->first = NULL;
    stringList->last = NULL;
    return stringList;
}

void insertStringList(StringList * stringList, char *string){
    StringListNode * stringListNode = (StringListNode *)malloc(sizeof(StringListNode));
    if(stringListNode == NULL){
        fprintf(stderr, "Erro de Alocação de memória\n");
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
    char direcionamentoType[3]; // Guarda qual foi o redirecionamento
}CommandType;

typedef struct CommandTypeNode{
    CommandType command;
    struct CommandTypeNode * next;
}CommandTypeNode;

typedef struct CommandList{
    CommandTypeNode *first;
    CommandTypeNode *last;
    int quantidadeComandos; // Armazena a quantidade de comandos terá no input
}CommandList;

CommandList * createCommandList(){
    CommandList * commandList = (CommandList *)malloc(sizeof(CommandList));
    if(commandList == NULL){
        fprintf(stderr, "Erro de Alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    commandList->first = NULL;
    commandList->last = NULL;
    commandList->quantidadeComandos = 1;
    return commandList;
}

void insertCommandList(CommandList * commandList, CommandType command){
    CommandTypeNode * commandTypeNode = (CommandTypeNode *)malloc(sizeof(CommandTypeNode));
    if(commandTypeNode == NULL){
        fprintf(stderr, "Erro de Alocação de memória\n");
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
        if (inputString[count] == '\n') {                  // Encontrou a quebra de linha
            inputString[count] = '\0';
        }
        count ++;
    }
    
    while (token != NULL){  
        token[strlen(token)] = '\0';  

        if(strcmp(token, "&&") == 0 || strcmp(token, "|") == 0 || strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
            newCommand = 0;
            command.direcionamento = 1;                    // Informando que teve direcionamento
            strcpy(command.direcionamentoType, token);     // Guardando qual foi o redirecionamento
            commandList->quantidadeComandos++;             // Incrimentando a quantidade de comandos que existem na lista
            insertCommandList(commandList, command);
            command.arguments = createStringList();
        }else if(strcmp(token, "sair") == 0){
            exit(EXIT_SUCCESS);

        }else if(newCommand == 0){
            strcpy(command.command, token);
            insertStringList(command.arguments, token);
            command.direcionamento = 0;
            command.argc = 1;
            newCommand = 1;

        }else{
            insertStringList(command.arguments, token);
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


/* 
    param: comando é um nó que contem o comando a ser executado junto dos argumentos necessários na execução.
    objetivo: Gerar o vetor de argumento que é passado no execvp, e executar e o nosso programa de acordo com o comando passado.
*/

void executarProcesso(CommandTypeNode *comando ) {
    StringListNode *parametros;

    char *argumentos[comando->command.argc+1];      // +1 para adicionar o NULL no final
    int cont = 0;
    
    parametros = comando->command.arguments->first; // Pega o primeiro argumento da lista

    /* Gerando vetor de argumentos */
    while(parametros != NULL){
        argumentos[cont] = parametros->string;
        parametros = parametros->next;
        cont ++;
    }

    argumentos[cont] = (char *)0;                   // Finalizando a ultima posição com NULL para sinalizar no execvp
    execvp(comando->command.command, argumentos);   // Executando comando
}


/* 
    param 1:  Ponteiro que informa o arquivo a ser aberto para escrita [char *name]
    param 2:  Ponteiro que contem a mensagem a ser escrita no arquivo [char* msg]
    objetivo: Pega o conteúdo da mensagem passada como parâmetro e exporta para um arquivo cujo nome também é informado.
              Utilizar essa função no auxilio do redirecionamento '>'.

*/

void writeFile(char *name, char msg[MAX_STRING_SIZE]) {
    FILE *archive;
    archive = fopen(name, "w");
    
    msg[strlen(msg)] = '\0';
    
    if (!archive) {
        fprintf(stderr, "Falha no redirecionamento > \n");
        exit(EXIT_FAILURE);
    }
    
    fprintf(archive, "%s", msg);
    fclose(archive);
}   


/* 
    param: Ponteiro que informa o arquivo a ser aberto
    objetivo: Percorre o arquivo, informado por parâmetro, e mostra as informações na tela. Essa função deve ser utilizada
              como auxiliar quando tem um redirecionamento do tipo '<'.

*/
void readFile(char *nameArchive) {
    FILE *archive;
    char linha[MAX_STRING_SIZE];

    archive = fopen(nameArchive, "r");
    
    if (!archive) {
        fprintf(stderr, "Falha na leitura do arquivo %s \n", nameArchive);
        exit(EXIT_FAILURE);
    }

    
    /* Percorre o arquivo até o fim */
    while (1) {
        if (fgets(linha, MAX_STRING_SIZE, archive) != NULL) {
            printf("%s", linha);
            
        } else {
            break;
        }
        
    }
    
    close(archive);
}

int main(){
    char *inputString = malloc(MAX_COMMAND_LINE_SIZE * sizeof(char));
    int flagSair = FALSE;
    CommandList * commandList;
    CommandTypeNode * aux;
    StringListNode * aux2;
    pid_t resultadoFork;
    pid_t resultadoFork2;


    while (flagSair == FALSE){
        printf("<VESH>: ");
        fgets(inputString, MAX_COMMAND_LINE_SIZE, stdin);

        //RETORNA UMA LISTA COM OS COMANDOS
        commandList = getListOfCommands(inputString);
        
        aux = commandList->first; // Pegando a lista de comandos
        
        if(strcmp(aux->command.command, "sair") == 0){
            exit(EXIT_SUCCESS);

        } else if (commandList->quantidadeComandos == 1) { // Não será necessário pipe [Usar um processo filho]
            resultadoFork = fork();                        // Criando processo filho
            
            if (resultadoFork < 0) {                       // Erro ao criar fork
                fprintf(stderr, "Falha na realizacao do fork()\n");
                exit(EXIT_FAILURE);
            
            }
             
            if(resultadoFork == 0) {                       // Bloco de códigos que será executado pelo processo filho
            
                executarProcesso(aux);
            } 
            // Processor criador(a) não irá executar 
            wait(NULL);

        } else if (commandList->quantidadeComandos == 2) { // Precisará de 1 pipe ou comandos de redirecionamento <, >[Usar 2 processos filhos]

            /* Para todos os casos será necessário criar um meio de comunicação, no caso o pipe */
            /* PROCESSO CRIADOR(A) CRIANDO O PIPE */
            int des_p[2];
            
            if (pipe(des_p) == -1) { // Erro ao criar pipe
                fprintf(stderr, "Falha na criação do pipe\n");
                exit(EXIT_FAILURE);
            }
            
            if (strcmp(aux->command.direcionamentoType, "|") == 0) {
                
                /* PROCESSO FILHO 1 */
                resultadoFork = fork();

                if (resultadoFork < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }
                if (resultadoFork == 0) {                                   // bloco de execução do processo filho
                    close(STDOUT_FILENO);  	                                // fecha a saída padrão (monitor), que não mais utilizarei no filho 1
                    dup(des_p[WRITE_END]);                                  // substitui a saída padrão pela minha (filho 1) ponta de escrita do pipe
                    close(des_p[WRITE_END]);                                // libera a ponta de escrita do pipe, já amarrada na saída padrão pelo dup()
                    close(des_p[READ_END]);	                                // fecha a minha (filho 1) ponta de leitura do pipe,

                    executarProcesso(aux);

                    // Só executará daqui para baixo no filho 1 caso dê erro no execvp
                    fprintf(stderr, "Falha na substituição (execvp) no filho 1");
                    exit(EXIT_FAILURE);        //se o exec der errado, fecha o processo filho A pois não faz sentido continuar                
                } /* FIM EXECUÇÃO DO PROCESSO FILHO 1 */
                
            

                /* PROCESSO FILHO 2 */
                resultadoFork2 = fork();

                if (resultadoFork2 < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }
                if (resultadoFork2 == 0) {                                  // bloco de execução do processo filho
                    close(STDIN_FILENO);   	                                // fecha a entrada padrão (teclado), que não mais utilizarei no filho 2
                    dup(des_p[READ_END]);                                   // substitui a entrada padrão pela minha (filho 2) ponta de leitura do pipe
                    close(des_p[READ_END]);                                 // libera a ponta de leitura do pipe, já amarrada na entrada padrão pelo dup()
                    close(des_p[WRITE_END]);                                // fecha a minha (filho 2) ponta de escrita do pipe, pois não utilizarei-a
                    aux = aux->next;                                        // Pegando o proximo bloco de comandos que deve ser executado

                    executarProcesso(aux);

                    // Só executará daqui para baixo no filho 1 caso dê erro no execvp
                    fprintf(stderr, "Falha na substituição (execvp) no filho 2");
                    exit(EXIT_FAILURE); //se o exec der errado, fecha o processo filho A pois não faz sentido continuar                
                } /* EXECUÇÃO DO PROCESSO FILHO 2 */

            } else if(strcmp(aux->command.direcionamentoType, ">") == 0) {
                
                /* PROCESSO FILHO 1 [IRÁ ESCREVER] > */
                resultadoFork = fork();
                
                if (resultadoFork < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }

                if (resultadoFork == 0) {                                   // bloco de execução do processo filho
                    close(STDOUT_FILENO);  	                                // fecha a saída padrão (monitor), que não mais utilizarei no filho 1
                    dup(des_p[WRITE_END]);                                  // substitui a saída padrão pela minha (filho 1) ponta de escrita do pipe
                    close(des_p[WRITE_END]);                                // libera a ponta de escrita do pipe, já amarrada na saída padrão pelo dup()
                    close(des_p[READ_END]);	                                // fecha a minha (filho 1) ponta de leitura do pipe

                    executarProcesso(aux);

                    // Só executará daqui para baixo no filho 1 caso dê erro no execvp
                    fprintf(stderr, "Falha na substituição (execvp) no filho 1");
                    exit(EXIT_FAILURE);                                      //se o exec der errado, fecha o processo filho A pois não faz sentido continuar                
                } /* FIM EXECUÇÃO DO PROCESSO FILHO 1 */                

                /* PROCESSO FILHO 2  > [IRÁ LER] */
                resultadoFork2 = fork();
                
                if (resultadoFork2 < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }

                if (resultadoFork2 == 0) {                                  // bloco de execução do processo filho
                    
                    close(STDIN_FILENO);   	                                // fecha a entrada padrão (teclado), que não mais utilizarei no filho 2
                    char read_msg[MAX_STRING_SIZE];                         // buffer para ler a mensagem passada pelo processo irmao anterior
                    // printf("conteudo pipe: %s", des_p[READ_END]);
                    read(des_p[READ_END], read_msg, (MAX_STRING_SIZE));   // Lendo a mensagem na ponta de leitura
                    // printf("conteudo read_msg: %s", read_msg);
                    aux = aux->next;                                        // Pegando o proximo bloco de comandos que deve ser executado
                    writeFile(aux->command.command, read_msg);              // Função para exportar informações em arquivo
                    close(des_p[READ_END]);                                 // libera a ponta de leitura do pipe, já amarrada na entrada padrão pelo dup()
                    close(des_p[WRITE_END]);                                // fecha a minha (filho 2) ponta de escrita do pipe, pois não utilizarei-a
                    
                    exit(EXIT_SUCCESS);
                } /* EXECUÇÃO DO PROCESSO FILHO 2 */

            } else if( strcmp(aux->command.direcionamentoType, "<") == 0) {
                
                /* PROCESSO FILHO 1 [IRÁ LER] < */
                resultadoFork = fork();
                
                if (resultadoFork < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }

                if (resultadoFork == 0) {                                   // bloco de execução do processo filho
                    close(STDIN_FILENO);                                    // Fechando entrada de dados via teclado
                    dup(des_p[READ_END]);                                   // Tornando o pipe como entrada padrão de leitura
                    close(des_p[READ_END]);	                                // fecha a minha (filho 1) ponta de leitura do pipe,
                    close(des_p[WRITE_END]);                                // libera a ponta de escrita do pipe, já amarrada na saída padrão pelo dup()
                    executarProcesso(aux);                                  // Função para executar o comando lido antes do sinal <
                    // Só executará daqui para baixo no filho 1 caso dê erro no execvp
                    fprintf(stderr, "Falha na substituição (execvp) no filho 1");
                    exit(EXIT_FAILURE);
                } /* FIM EXECUÇÃO DO PROCESSO FILHO 1 */
                

                /* PROCESSO FILHO 2  > [IRÁ ESCREVER] */
                resultadoFork2 = fork();
                
                if (resultadoFork2 < 0) { // Erro ao criar fork
                    fprintf(stderr, "Falha na realizacao do fork()\n");
                    exit(EXIT_FAILURE);
                }

                if (resultadoFork2 == 0) {                                  // bloco de execução do processo filho
                    
                    aux = aux->next;                                        // Pegando o proximo bloco de comandos que deve ser executado
                    close(STDOUT_FILENO);                                   // fecha a saída padrão (monitor), que não será utilizada
                    dup(des_p[WRITE_END]);                                  // Saida padrão de escrita será colocado no pipe
                    readFile(aux->command.command);                         // Função que irá ler o conteudo do arquivo e povoar a variavel msg
                    close(des_p[WRITE_END]);                                // libera a ponta de escrita do pipe, já amarrada na saída padrão pelo dup()
                    close(des_p[READ_END]);                                 // fecha a minha (filho 1) ponta de leitura do pipe
                    exit(EXIT_SUCCESS);

                } /* FIM EXECUÇÃO DO PROCESSO FILHO 2 */

            }                        

            close(des_p[WRITE_END]);
		    close(des_p[READ_END]);

            wait(NULL);                                                     /* Fazendo o processo criador(a) esperar pelos filhos */
            wait(NULL);                                                     /* Fazendo o processo criador(a) esperar pelos filhos */

        } /* else {} pensar nesse caso com mais de 1 pipe

        //APENAS PARA DEBUGAR
        /* aux = commandList->first;
        while (aux != NULL)
        {
            
            printf("Comando: %s\nargc: %d\ndirecionamento %d\nArgumentos: ", aux->command.command, aux->command.argc, aux->command.direcionamento);
            aux2 = aux->command.arguments->first;
            while(aux2 != NULL){
                printf("%s ", aux2->string);
                aux2 = aux2->next;
            }

            if (aux->command.direcionamento == 1) { // Mostra redirecionamento caso exista
                printf("\nRedirecionamento: %s", aux->command.direcionamentoType); 
            }
            
            printf("\n\n");
            aux = aux->next;
        } */
        
    }

    return EXIT_SUCCESS;
}