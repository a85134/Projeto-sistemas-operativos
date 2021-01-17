
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
#include <stdbool.h> 

#define MAX_BUFFER 1024 

static const char transform[] = "transform"; // transformar filtros
static const char status[] = "status"; // ver status do servidor
static const char file_input_confirmation[] = "../samples";

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

typedef struct Pedido{

    char tipo[20]; // transform ou status
    int n_filtros; // numero de filtros 
    char f_input[1024]; // ficheiro input  
    char f_output[1024]; // ficheiro output
    char filtros[128][1024]; // filtros
  
}pedido;

typedef struct Process
{
    int pid;     // pid correspondente ao processo
    char numero_filtros[5]; // alto(0), baixo(1), eco(2), rapido(3), lento(4)

} process;


int main(int argc, char *argv[]){

    char filters[128][1024];
    process pr[20] = {-1};
    
    if(argc < 2){ // Método de utilização
        printf("Utilização: ./cliente transform input-filename output-filename filter-id-1 filter-id-2 ... \n");
        printf("OU \n");
        printf("Utilização: ./cliente status \n");
        return 1;
    }

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

    pedido p1; //cliente entrou para efetuar pedido

    if(memcmp(argv[1], status, strlen(status)) == 0 && argc == 2){ // Deteta que é status e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        printf("Cliente a executar...\n");
        strcpy(p1.tipo, argv[1]);
        printf("ENTROU1\n");
        int stop = 0;
        write(output, &p1, sizeof(pedido));
        int output_instancia[5][2] = {};
        char str[2048];
        int n = 0;
        n = read(input, str, 2048);
            printf("SIZEOFFF N: %d\n", n);
            write(1, str, n);
        close(input);
        memset(str, 0 ,sizeof(str));
        return 0;
    }
    
    if(memcmp(argv[1], transform, strlen(transform)) == 0 && argc >= 5){ // Deteta que é transform e procede ao seu pedido
        // pegar os argumentos da linha de comando e adicionar à estrutura para posteriormente enviar ao servidor
        if(startsWith(file_input_confirmation, argv[2]) == true){
            printf("Cliente a executar...\n");
            printf("ENTROU2\n");
            strcpy(p1.tipo, argv[1]);
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
            printf("Pedido tipo: %s\n", p1.tipo);
            printf("Pedido file_input: %s\n", p1.f_input);
            printf("Pedido file_output: %s\n", p1.f_output);
            printf("Pedido n_filters: %d\n", p1.n_filtros);
            int stop = 0; // implementar ciclo ou não
            write(output, &p1, sizeof(pedido));
            int bufferSize = 0;
            char buffer_output[1024];
            // while((bufferSize = read(input, buffer_output, sizeof(buffer_output))) > 0) { // fica à espera de receber a resposta do servidor
            //     printf("HELLO\n");
            // }
            // printf("Recebi, obrigado!\n");
            return 0;
        }
        else{
            printf("Exemplo: ./cliente transform ../samples/sample-1.m4a output.m4a alto alto alto \n");
            return 1;
        }
        
        
    }
    
}