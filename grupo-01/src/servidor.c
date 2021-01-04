
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

void siginthandler(int sig){
    unlink("servidor.pipe");
    unlink("cliente.pipe");
    _exit(0);
}

typedef struct Pedido{

    int tipo; // transform ou status
    int n_filtros; // numero de filtros 
    char f_input[1024]; // ficheiro input  
    char f_output[1024]; // ficheiro output
    char filtros[128][1024]; // filtros
  
}pedido; 

int main(int argc, char **argv){

    signal(SIGINT, siginthandler);

    printf("Servidor a executar...\n");

    char *fifo_escrita = "servidor.pipe"; //fifo para escrita
    char *fifo_leitura = "cliente.pipe"; //fifo para leitura
    
    char buffer[MAX_BUFFER];
    int bufferSize = 0;
    
    
    if(mkfifo(fifo_leitura, 0666) < 0){
        perror("Erro FIFO leitura");
        return -1;
    }

    if(mkfifo(fifo_escrita, 0666) < 0){
        perror("Erro FIFO escrita");
        return -1;
    }

    int input = open(fifo_leitura, O_RDONLY);
    if(input < 0){
        perror("Erro abertura pipe leitura");
        return -1;    
    }
    int output = open(fifo_escrita, O_WRONLY | O_NONBLOCK);
    if(output < 0){
        perror("Erro abertura pipe escrita");
        return -1;
    }
    
    int stop = 0;

    pedido p1;

    while(!stop){
        while((bufferSize = read(input, &p1, sizeof(pedido))) > 0){
            printf("Pedido recebido tipo: %d\n", p1.tipo);
            printf("Pedido recebido f_input: %s\n", p1.f_input);
            printf("Pedido recebido f_output: %s\n", p1.f_output);
            for(int i=0;i<p1.n_filtros;i++){
                printf("Pedido recebido f_output: %s\n", p1.filtros[i]);
            }
            printf("Pedido recebido n_filtros: %d\n", p1.n_filtros);
            //write(1, &p1, bufferSize);
        }
        stop = 1;
    }
    kill(getpid(),SIGINT);
    return 0;
}