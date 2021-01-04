
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
    int n_filtros; // numero de filtros 
    char f_input[1024]; // ficheiro input  
    char f_output[1024]; // ficheiro output
    char filtros[128][1024]; // filtros
  
}pedido; 


int main(int argc, char *argv[]){

    char filters[128][1024];
    
    if(argc < 5){ // Método de utilização
        printf("Utilização: ./cliente transform input-filename output-filename filter-id-1 filter-id-2 ... \n");
        return 1;
    }

    pedido p1; //cliente entrou para efetuar pedido

    if(memcmp(argv[1], status, strlen(argv[1])) == 0){ // Deteta que é status e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        p1.tipo = 1; // tipo 1 indica que é status
        printf("ENTROU1\n");
    }
    
    if(memcmp(argv[1], transform, strlen(argv[1])) == 0){ // Deteta que é transform e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        p1.tipo = 1;
        strcpy(p1.f_input, argv[2]);
        strcpy(p1.f_output, argv[3]);
        p1.n_filtros = argc - 4;
        int increment = 0;
        for(int i = 4; i < argc; i++){
            for(int j = 0; j < strlen(argv[i]); j++){
                filters[i-4][j] = argv[i][j];
                
            }
            strcpy(p1.filtros[increment], filters[i-4]);
            printf("PEDIDO Filtro: %s\n", p1.filtros[increment]);
            increment++;
        }
        printf("Pedido tipo: %d\n", p1.tipo);
        printf("Pedido file_input: %s\n", p1.f_input);
        printf("Pedido file_output: %s\n", p1.f_output);
        printf("Pedido n_filters: %d\n", p1.n_filtros);
    }


    printf("Cliente a executar...\n");

    char *fifo_leitura = "servidor.pipe"; //fifo para escrita
    char *fifo_escrita = "cliente.pipe"; //fifo para leitura
    
    char buffer[MAX_BUFFER];
    int bufferSize = 0; 

    
    int output = open(fifo_escrita, O_WRONLY | O_NONBLOCK);
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
    
    write(output, &p1, sizeof(pedido));
    
    return 0;
}