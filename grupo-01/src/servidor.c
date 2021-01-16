
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

static const char config_filename[] = "../etc/aurrasd.conf";
static const char filters_folder[] = "../bin/aurrasd-filters";
static const char transform[] = "transform"; // transformar filtros
static const char status[] = "status";       // ver status do servidor

void siginthandler(int sig)
{
    unlink("servidor.pipe");
    unlink("cliente.pipe");
    _exit(0);
}

typedef struct Pedido
{

    char tipo[20];           // transform ou status
    int n_filtros;           // numero de filtros
    char f_input[1024];      // ficheiro input
    char f_output[1024];     // ficheiro output
    char filtros[128][1024]; // filtros

} pedido;

typedef struct Config
{

    char filter_type[1024]; // tipo de filtro (alto, baixo, etc)
    char filter_name[1024]; // nome do filtro
    int n_instancia;        // n_instancias

} config;

void ExecuteTransform(pedido p, config f[], int counter, int output)
{ // Função transform
    // Criar o ficheiro output
    // int fd_output = open(f_output, O_CREAT | O_RDWR, 0666);
    //printf("n_filtros: %d \n", p.n_filtros);
    int instancia = 1;
    int beforePipe = 0;
    //int afterPipe[2];
    //int outputPipe[2];
    int outputPipe[p.n_filtros][2];
    int filho[p.n_filtros];
    int readyOutput = 0;
    for (int i = 0; i < p.n_filtros; i++)
    {
        pipe(outputPipe[i + 1]);
        outputPipe[0][0] = open(p.f_input, O_RDONLY);
        if (memcmp(p.filtros[i], "alto", strlen("alto")) == 0)
        { // Faz o processamento para o filtro alto
            for (int j = 0; j < counter; j++)
            { // counter = 5
                if (memcmp(f[j].filter_name, "alto", strlen("alto")) == 0)
                {
                    //int instancia = atoi(p.filtros[i]);
                    //printf("Instancias tipo alto: %d\n", f[j].n_instancia);// TEM DE DEVOLVER 2
                    //printf("Instancias: %d\n", instancia);
                        readyOutput++;
                        f[j].n_instancia = f[j].n_instancia - 1;
                        char file_filter[1024]; // filtro correspondente para fazer open (primeira parte)
                        memcpy(&file_filter, &filters_folder, sizeof(filters_folder));
                        //printf("filtrs folder size: %ld\n", sizeof(filters_folder));
                        file_filter[sizeof(filters_folder) - 1] = '/';
                        memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), strlen(f[j].filter_type));
                        //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                        printf("file filter: %s\n", file_filter);
                        printf("file input: %s\n", p.f_input);
                        printf("file output: %s\n", p.f_output);
                        //printf("hello\n");
                        filho[i] = fork();
                        if (filho[i] == 0)
                        { // Processo filho que trata do processamento
                            close(outputPipe[i][1]);
                            dup2(outputPipe[i][0], 0);
                            close(outputPipe[i][0]);
                            close(outputPipe[i + 1][0]);
                            dup2(outputPipe[i + 1][1], 1);
                            close(outputPipe[i + 1][1]);
                            // bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a
                            execlp(file_filter, file_filter, NULL);
                            _exit(1);
                        }
                        else
                        { // Processo pai que controla o seu filho
                            if (i != 0)
                            {
                                close(outputPipe[i][0]);
                                close(outputPipe[i][1]);
                            }
                            int status;
                            if (filho[i] == wait(&status))
                            {
                                if (WEXITSTATUS(status))
                                {
                                    close(outputPipe[i + 1][0]);
                                    close(outputPipe[i + 1][1]);
                                    //return 1;
                                }
                            }
                            printf("Sou o pai %d e o meu filho %d terminou com código de saída %d \n", getppid(), filho[i], status);
                        }
                  
                }
            }
        }
        if (memcmp(p.filtros[i], "baixo", strlen("baixo")) == 0)
        { // Faz o processamento para o filtro baixo
            for (int j = 0; j < counter; j++)
            { // counter = 5
                if (memcmp(f[j].filter_name, "baixo", strlen("baixo")) == 0)
                {
                    //printf("Instancias deste tipo: %d\n", f[i].n_instancia);// TEM DE DEVOLVER 2
                    //printf("Instancias: %d\n", instancia);
                        readyOutput++;
                        f[j].n_instancia = f[j].n_instancia - 1;
                        char file_filter[1024]; // filtro correspondente para fazer open (primeira parte)
                        memcpy(&file_filter, &filters_folder, sizeof(filters_folder));
                        //printf("filtrs folder size: %ld\n", sizeof(filters_folder));
                        file_filter[sizeof(filters_folder) - 1] = '/';
                        memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), strlen(f[j].filter_type));
                        //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                        printf("file filter: %s\n", file_filter);
                        printf("file input: %s\n", p.f_input);
                        printf("file output: %s\n", p.f_output);
                        //printf("hello\n");
                        filho[i] = fork();
                        if (filho[i] == 0)
                        { // Processo filho que trata do processamento
                            close(outputPipe[i][1]);
                            dup2(outputPipe[i][0], 0);
                            close(outputPipe[i][0]);
                            close(outputPipe[i + 1][0]);
                            dup2(outputPipe[i + 1][1], 1);
                            close(outputPipe[i + 1][1]);
                            // bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a
                            execlp(file_filter, file_filter, NULL);
                            _exit(1);
                        }
                        else
                        { // Processo pai que controla o seu filho
                            if (i != 0)
                            {
                                close(outputPipe[i][0]);
                                close(outputPipe[i][1]);
                            }
                            int status;
                            if (filho[i] == wait(&status))
                            {
                                if (WEXITSTATUS(status))
                                {
                                    close(outputPipe[i + 1][0]);
                                    close(outputPipe[i + 1][1]);
                                    //return 1;
                                }
                            }
                            printf("Sou o pai %d e o meu filho %d terminou com código de saída %d \n", getppid(), filho[i], status);
                        }
                    
                }
            }
        }
        if (memcmp(p.filtros[i], "eco", strlen("eco")) == 0)
        { // Faz o processamento para o filtro eco
            for (int j = 0; j < counter; j++)
            { // counter = 5
                if (memcmp(f[j].filter_name, "eco", strlen("eco")) == 0)
                {
                    //printf("Instancias deste tipo: %d\n", f[j].n_instancia);// TEM DE DEVOLVER 1
                    //printf("Instancias: %d\n", instancia);
            
                        readyOutput++;
                        f[j].n_instancia = f[j].n_instancia - 1;
                        char file_filter[1024]; // filtro correspondente para fazer open (primeira parte)
                        memcpy(&file_filter, &filters_folder, sizeof(filters_folder));
                        //printf("filtrs folder size: %ld\n", sizeof(filters_folder));
                        file_filter[sizeof(filters_folder) - 1] = '/';
                        memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), strlen(f[j].filter_type));
                        //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                        printf("file filter: %s\n", file_filter);
                        printf("file input: %s\n", p.f_input);
                        printf("file output: %s\n", p.f_output);
                        //printf("hello\n");
                        filho[i] = fork();
                        if (filho[i] == 0)
                        { // Processo filho que trata do processamento
                            close(outputPipe[i][1]);
                            dup2(outputPipe[i][0], 0);
                            close(outputPipe[i][0]);
                            close(outputPipe[i + 1][0]);
                            dup2(outputPipe[i + 1][1], 1);
                            close(outputPipe[i + 1][1]);
                            // bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a
                            execlp(file_filter, file_filter, NULL);
                            _exit(1);
                        }
                        else
                        { // Processo pai que controla o seu filho
                            if (i != 0)
                            {
                                close(outputPipe[i][0]);
                                close(outputPipe[i][1]);
                            }
                            int status;
                            if (filho[i] == wait(&status))
                            {
                                if (WEXITSTATUS(status))
                                {
                                    close(outputPipe[i + 1][0]);
                                    close(outputPipe[i + 1][1]);
                                    //return 1;
                                }
                            }
                            printf("Sou o pai %d e o meu filho %d terminou com código de saída %d \n", getppid(), filho[i], status);
                        }
                   
                }
            }
        }
        if (memcmp(p.filtros[i], "rapido", strlen("rapido")) == 0)
        { // Faz o processamento para o filtro rapido
            for (int j = 0; j < counter; j++)
            { // counter = 5
                if (memcmp(f[j].filter_name, "rapido", strlen("rapido")) == 0)
                {
                    //printf("Instancias deste tipo: %d\n ", f[j].n_instancia);// TEM DE DEVOLVER 2
                    //printf("Instancias: %d\n", instancia);
                    
                        readyOutput++;
                        f[j].n_instancia = f[j].n_instancia - 1;
                        char file_filter[1024]; // filtro correspondente para fazer open (primeira parte)
                        memcpy(&file_filter, &filters_folder, sizeof(filters_folder));
                        //printf("filtrs folder size: %ld\n", sizeof(filters_folder));
                        file_filter[sizeof(filters_folder) - 1] = '/';
                        memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), strlen(f[j].filter_type));
                        //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                        printf("file filter: %s\n", file_filter);
                        printf("file input: %s\n", p.f_input);
                        printf("file output: %s\n", p.f_output);
                        //printf("hello\n");
                        filho[i] = fork();
                        if (filho[i] == 0)
                        { // Processo filho que trata do processamento
                            close(outputPipe[i][1]);
                            dup2(outputPipe[i][0], 0);
                            close(outputPipe[i][0]);
                            close(outputPipe[i + 1][0]);
                            dup2(outputPipe[i + 1][1], 1);
                            close(outputPipe[i + 1][1]);
                            // bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a
                            execlp(file_filter, file_filter, NULL);
                            _exit(1);
                        }
                        else
                        { // Processo pai que controla o seu filho
                            if (i != 0)
                            {
                                close(outputPipe[i][0]);
                                close(outputPipe[i][1]);
                            }
                            int status;
                            if (filho[i] == wait(&status))
                            {
                                if (WEXITSTATUS(status))
                                {
                                    close(outputPipe[i + 1][0]);
                                    close(outputPipe[i + 1][1]);
                                    //return 1;
                                }
                            }
                            printf("Sou o pai %d e o meu filho %d terminou com código de saída %d \n", getppid(), filho[i], status);
                        }
                }
            }
        }
        if (memcmp(p.filtros[i], "lento", strlen("lento")) == 0)
        { // Faz o processamento para o filtro lento
            for (int j = 0; j < counter; j++)
            { // counter = 5
                if (memcmp(f[j].filter_name, "lento", strlen("lento")) == 0)
                {
                    //printf("Instancias deste tipo: %d\n ", f[j].n_instancia);// TEM DE DEVOLVER 1
                    //printf("Instancias: %d\n", instancia);
                        readyOutput++;
                        f[j].n_instancia = f[j].n_instancia - 1;
                        char file_filter[1024]; // filtro correspondente para fazer open (primeira parte)
                        memcpy(&file_filter, &filters_folder, sizeof(filters_folder));
                        //printf("filtrs folder size: %ld\n", sizeof(filters_folder));
                        file_filter[sizeof(filters_folder) - 1] = '/';
                        memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), strlen(f[j].filter_type));
                        //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                        printf("file filter: %s\n", file_filter);
                        printf("file input: %s\n", p.f_input);
                        printf("file output: %s\n", p.f_output);
                        //printf("hello\n");
                        filho[i] = fork();
                        if (filho[i] == 0)
                        { // Processo filho que trata do processamento
                            close(outputPipe[i][1]);
                            dup2(outputPipe[i][0], 0);
                            close(outputPipe[i][0]);
                            close(outputPipe[i + 1][0]);
                            dup2(outputPipe[i + 1][1], 1);
                            close(outputPipe[i + 1][1]);
                            // bin/aurrasd-filters/aurrasd-echo < samples/samples-1.m4a
                            execlp(file_filter, file_filter, NULL);
                            _exit(1);
                        }
                        else
                        { // Processo pai que controla o seu filho
                            if (i != 0)
                            {
                                close(outputPipe[i][0]);
                                close(outputPipe[i][1]);
                            }
                            int status;
                            if (filho[i] == wait(&status))
                            {
                                if (WEXITSTATUS(status))
                                {
                                    close(outputPipe[i + 1][0]);
                                    close(outputPipe[i + 1][1]);
                                    //return 1;
                                }
                            }
                            printf("Sou o pai %d e o meu filho %d terminou com código de saída %d \n", getppid(), filho[i], status);
                        }
                    
                }
            }
        }
        if (readyOutput == p.n_filtros)
        {
            close(outputPipe[p.n_filtros][1]);
            int fd_out = open(p.f_output, O_CREAT | O_WRONLY, 0666);
            int n = 0;
            char buffer[1024];
            while ((n = read(outputPipe[p.n_filtros][0], buffer, sizeof(buffer))) > 0)
            {
                write(fd_out, buffer, sizeof(buffer));
                write(output, buffer, sizeof(buffer)); // servidor envia o resultado do pedido do cliente
                printf("A repetir...\n");
            }
            printf("OUTPUT FEITO !\n");
            close(outputPipe[p.n_filtros][0]);
            close(fd_out);
        }
    }
}

void ExecuteStatus()
{ // Função transform
}

int main(int argc, char *argv[])
{

    signal(SIGINT, siginthandler);

    if (argc < 3)
    { // Método de utilização
        printf("Utilização: ./servidor config-filename filters-folder \n");
        return 1;
    }

    if (memcmp(argv[1], config_filename, strlen(config_filename)) == 0)
    { // Deteta se o argumento file-name foi bem inserido
        printf("ENTROU 1\n");
    }
    else
    {
        printf("Utilização: ./servidor ../etc/aurrasd.conf ../bin/aurras-filters \n");
        return 1;
    }

    if (memcmp(argv[2], filters_folder, strlen(filters_folder)) == 0)
    { // Deteta se o argumento filters-folder foi bem inseridos
        printf("ENTROU 2\n");
    }
    else
    {
        printf("Utilização: ./servidor etc/aurrasd.conf bin/aurras-filters \n");
        return 1;
    }

    printf("Servidor a executar...\n");

    char *fifo_escrita = "servidor.pipe"; //fifo para escrita
    char *fifo_leitura = "cliente.pipe";  //fifo para leitura

    char buffer[MAX_BUFFER];
    int bufferSize = 0;

    if (mkfifo(fifo_leitura, 0666) < 0)
    {
        perror("Erro FIFO leitura");
        return -1;
    }

    if (mkfifo(fifo_escrita, 0666) < 0)
    {
        perror("Erro FIFO escrita");
        return -1;
    }

    int input = open(fifo_leitura, O_RDONLY);
    if (input < 0)
    {
        perror("Erro abertura pipe leitura");
        return -1;
    }
    int output = open(fifo_escrita, O_WRONLY | O_NONBLOCK);
    if (output < 0)
    {
        perror("Erro abertura pipe escrita");
        return -1;
    }

    int file_conf = open(argv[1], O_RDONLY); // Abrir o ficheiro de configuração
    if (file_conf < 0)
    { // Erro a abrir o ficheiro
        perror("Erro abrir ficheiro configuração");
        return -1;
    }

    int n = 0;
    char file_buffer[1024] = {0};
    char c;
    int increment = 0;
    int counter = 0;
    int size = 0;

    config file_configuration[32]; // Array de estrutura para os filtros do servidor

    while ((n = read(file_conf, &c, 1)) > 0)
    { // Lê o conteúdo do ficheiro e aloca na estrutura o filtro do servidor
        file_buffer[size++] = c;
        if (file_buffer[strlen(file_buffer) - 1] == '\n')
        {
            file_buffer[strlen(file_buffer) - 1] = '\0';
            increment++;
            if (increment == 1)
            {
                strcpy(file_configuration[counter].filter_name, file_buffer);
                memset(file_buffer, 0, sizeof(file_buffer));
                size = 0;
            }
            if (increment == 2)
            {
                strcpy(file_configuration[counter].filter_type, file_buffer);
                memset(file_buffer, 0, sizeof(file_buffer));
                size = 0;
            }
            if (increment == 3)
            {
                file_configuration[counter].n_instancia = atoi(file_buffer);
                counter++;
                increment = 0;
                memset(file_buffer, 0, sizeof(file_buffer));
                size = 0;
            }
        }
    }

    int total_instancias = 0;

    for (int i = 0; i < counter; i++) // counter = 5
    {
        printf("config.tipo: %s \n", file_configuration[i].filter_type);
        printf("config.nome: %s \n", file_configuration[i].filter_name);
        printf("config.instancias: %d \n", file_configuration[i].n_instancia);
        total_instancias = total_instancias + file_configuration[i].n_instancia;
    }

    printf("N_INSTANCIAS: %d\n", total_instancias); //devolve 8

    // Aqui recebe os argumentos do cliente, após isso é fazer o processamento dos dados recebidos e executar.

    int stop = 0;

    pedido p1;
    bool execute = false;

    while (!stop)
    {
        while ((bufferSize = read(input, &p1, sizeof(pedido))) > 0)
        {
            printf("Pedido recebido tipo: %s\n", p1.tipo);
            printf("Pedido recebido f_input: %s\n", p1.f_input);
            printf("Pedido recebido f_output: %s\n", p1.f_output);
            for (int i = 0; i < p1.n_filtros; i++)
            {
                printf("Pedido recebido filtros: %s\n", p1.filtros[i]);
            }
            printf("Pedido recebido n_filtros: %d\n", p1.n_filtros);
            if (memcmp(p1.tipo, transform, strlen(transform)) == 0)
            { // se for transform a receber, processa os dados
                // printf("ENTROU TRANSFORM \n");
                // Recebe pedido e cria filho para tratar do pedido
                // Dentro da função,
                for (int i = 0; i < p1.n_filtros; i++)
                {
                    if (memcmp(p1.filtros[i], "alto", strlen("alto")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        { // counter = 5
                            if (memcmp(file_configuration[j].filter_name, "alto", strlen("alto")) == 0)
                            {
                                printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia); // TEM DE DEVOLVER 1
                                file_configuration[j].n_instancia = file_configuration[j].n_instancia - 1;
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "baixo", strlen("baixo")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "baixo", strlen("baixo")) == 0)
                            {
                                printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia); // TEM DE DEVOLVER 1
                                file_configuration[j].n_instancia = file_configuration[j].n_instancia - 1;
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "eco", strlen("eco")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "eco", strlen("eco")) == 0)
                            {
                                printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia); // TEM DE DEVOLVER 1
                                file_configuration[j].n_instancia = file_configuration[j].n_instancia - 1;
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "rapido", strlen("rapido")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "rapido", strlen("rapido")) == 0)
                            {
                                printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia); // TEM DE DEVOLVER 1
                                file_configuration[j].n_instancia = file_configuration[j].n_instancia - 1;
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "lento", strlen("lento")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "lento", strlen("lento")) == 0)
                            {
                                printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia); // TEM DE DEVOLVER 1
                                file_configuration[j].n_instancia = file_configuration[j].n_instancia - 1;
                                //printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia);
                            }
                        }
                    }
                }
                for (int i = 0; i < p1.n_filtros; i++)
                {
                    for (int j = 0; j < counter; j++)
                    {
                        if (file_configuration[j].n_instancia < 0)
                        {
                            
                            execute = true;
                            
                        }
                    }
                }
                if(execute == true){ 
                    printf("Não pode ser processado o pedido!\n");
                }
                if(execute == false){
                    if(!fork()){
                        ExecuteTransform(p1, file_configuration, counter, output);
                    }
                }
                

                // chama função que processa os dados e retorna o resultado
                // faz write desse resultado
            }
            if (memcmp(p1.tipo, status, strlen(status)) == 0)
            { // se for status a receber, envia o status do servidor
                printf("ENTROU STATUS \n");
                ExecuteStatus();
                // chama função que faz o status e retorna o resultado
                // faz write desse resultado
            }
        }
    }

    kill(getpid(), SIGINT);
    return 0;
}