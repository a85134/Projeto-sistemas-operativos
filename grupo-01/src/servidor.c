
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
    int output = open(fifo_escrita, O_WRONLY);
    if(output < 0){
        perror("Erro abertura pipe escrita");
        return -1;
    }
    
    int stop = 0;

    while (!stop){
        while((bufferSize = read(input, buffer, MAX_BUFFER)) > 0){
            write(1, buffer, bufferSize);
        }
    }
    return 0;
}