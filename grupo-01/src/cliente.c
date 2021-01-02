
#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>  /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_BUFFER 1024 

static const char transform[] = "transform"; // transformar filtros
static const char status[] = "status"; // ver status do servidor


typedef struct Pedido{

    int tipo; // transform ou status
    char f_input[1024]; // ficheiro input  
    char f_output[1024]; // ficheiro output
    char filtros[128][1024]; // filtros
    int n_filtros; // numero de filtros  

}pedido; 


int main(int argc, char *argv[]){

    // criar o pedido e alocar os argumentos no pedido em vez destas variaveis
    int type;
    char *file_input;
    char *file_output;
    char filters[128][1024];
    int n_filters;


    if(argc < 5){ // Método de utilização
        printf("Utilização: ./cliente transform input-filename output-filename filter-id-1 filter-id-2 ... \n");
        return 1;
    }

    if(memcmp(argv[1], status, strlen(argv[1])) == 0){ // Deteta que é status e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        type = 1; // tipo 1 indica que é status
        printf("ENTROU1\n");
    }
    
    if(memcmp(argv[1], transform, strlen(argv[1])) == 0){ // Deteta que é transform e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        type = 0; // tipo 0 indica que é transforma
        file_input = argv[2];
        printf("File_input: %s\n", file_input);
        file_output = argv[3];
        printf("File_output: %s\n", file_output);
        n_filters = argc - 4;
        printf("N_filters: %d\n", n_filters);
        for(int i = 4; i < argc; i++){
            for(int j = 0; j < strlen(argv[i]); j++){
                filters[i-4][j] = argv[i][j];
                
            }
            printf("Filtro: %s\n", filters[i-4]);   
        }
        
        // pedido p1 = {};
        printf("ENTROU\n");
    }


    printf("Cliente a executar...\n");

    char *fifo_leitura = "servidor.pipe"; //fifo para escrita
    char *fifo_escrita = "cliente.pipe"; //fifo para leitura
    
    char buffer[MAX_BUFFER];
    int bufferSize = 0; 

    
    int output = open(fifo_escrita, O_WRONLY);
    if(output < 0){
        perror("Erro abertura pipe escrita");
        return -1;
    }

    int input = open(fifo_leitura, O_RDONLY);
    if(input < 0){
        perror("Erro abertura pipe leitura");
        return -1;    
    }

    int stop = 0;

    //write(output, file_output, strlen(file_output));

    while (!stop){ // ENVIAR A ESTRUTURA COMPLETA PARA O SERVIDOR
        while ((bufferSize = read(0,buffer, MAX_BUFFER)) > 0){   
            write(output, buffer, bufferSize);
        }
    }
    
    return 0;
}