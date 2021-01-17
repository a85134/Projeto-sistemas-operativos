
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

int count_pedidos = 1;

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

typedef struct PedidoTranform
{

    char tipo[20];           // transform ou status
    int n_filtros;           // numero de filtros
    char f_input[1024];      // ficheiro input
    char f_output[1024];     // ficheiro output
    char filtros[128][1024]; // filtros

} pedidoTransform;

typedef struct Config
{

    char filter_type[1024]; // tipo de filtro (alto, baixo, etc)
    char filter_name[1024]; // nome do filtro
    int n_instancia;        // n_instancias

} config;

typedef struct Check
{

    char filter_type[1024]; // tipo de filtro (alto, baixo, etc)
    char filter_name[1024]; // nome do filtro
    int n_instancia;        // n_instancias

} check;

typedef struct Process
{
    int pid;     // pid correspondente ao processo
    char numero_filtros[5]; // alto(0), baixo(1), eco(2), rapido(3), lento(4)
    char f_input[1024];      // ficheiro input
    char f_output[1024];     // ficheiro output

} process;

process pr[20] = {-1};
config file_configuration[32]; // Array de estrutura para os filtros do servidor
check check_filters[32];
int number_filters = 0;

void sigchldhandler(int sig) // sinal que envia o pid do pedido terminado para atualizar o ficheiro de configuração
{
    int status;
    int pid = wait(&status);
    printf("PID Handler 1: %d\n", pid); 
    int k;     
    for(k = 0; k < 20; k++){
        if(pr[k].pid == pid){ // alto(0), baixo(1), eco(2), rapido(3), lento(4)
        printf("TOU\n");
            if(pr[k].numero_filtros[0] > 0){
                printf("pr[k].numero_filtros[0] : %d", pr[k].numero_filtros[0]);
                for(int j = 0; j < 5; j++){
                    if(memcmp(check_filters[j].filter_name, "alto", strlen("alto")) == 0){
                        check_filters[j].n_instancia = check_filters[j].n_instancia + pr[k].numero_filtros[0];
                    }
                }
            }
            if(pr[k].numero_filtros[1] > 0){
                for(int j = 0; j < 5; j++){
                    if(memcmp(check_filters[j].filter_name, "baixo", strlen("baixo")) == 0){
                        check_filters[j].n_instancia = check_filters[j].n_instancia + pr[k].numero_filtros[1];
                    }
                }
            }
            if(pr[k].numero_filtros[2] > 0){
                for(int j = 0; j < 5; j++){
                    if(memcmp(check_filters[j].filter_name, "eco", strlen("eco")) == 0){
                        check_filters[j].n_instancia = check_filters[j].n_instancia + pr[k].numero_filtros[2];
                    }
                }
            }
            if(pr[k].numero_filtros[3] > 0){
                for(int j = 0; j < 5; j++){
                    if(memcmp(check_filters[j].filter_name, "rapido", strlen("rapido")) == 0){
                        check_filters[j].n_instancia = check_filters[j].n_instancia + pr[k].numero_filtros[3];
                    }
                }
            }
            if(pr[k].numero_filtros[4] > 0){
                for(int j = 0; j < 5; j++){
                    if(memcmp(check_filters[j].filter_name, "lento", strlen("lento")) == 0){
                        check_filters[j].n_instancia = check_filters[j].n_instancia + pr[k].numero_filtros[4];
                    }
                }
            }
            pr[k].pid = -1;
            memset(&pr[k].numero_filtros, 0, sizeof(pr[k].numero_filtros));
            memset(&pr[k].f_input, 0, sizeof(pr[k].f_input));
            memset(&pr[k].f_output, 0, sizeof(pr[k].f_output));
            break;
        }

    }
                          
    
}

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
                    memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), sizeof(f[j].filter_type));
                    //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                    printf("file filter: %s\n", file_filter);
                    printf("file input: %s\n", p.f_input);
                    printf("file output: %s\n", p.f_output);
                    //printf("hello\n");
                    filho[i] = fork();
                    if (filho[i] == 0)
                    { // Processo filho que trata do processamento
                        signal(SIGCHLD, 0);
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
                    memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), sizeof(f[j].filter_type));
                    //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                    printf("file filter: %s\n", file_filter);
                    printf("file input: %s\n", p.f_input);
                    printf("file output: %s\n", p.f_output);
                    //printf("hello\n");
                    filho[i] = fork();
                    if (filho[i] == 0)
                    { // Processo filho que trata do processamento
                        signal(SIGCHLD, 0);
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
                    memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), sizeof(f[j].filter_type));
                    //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                    printf("file filter: %s\n", file_filter);
                    printf("file input: %s\n", p.f_input);
                    printf("file output: %s\n", p.f_output);
                    //printf("hello\n");
                    filho[i] = fork();
                    if (filho[i] == 0)
                    { // Processo filho que trata do processamento
                        signal(SIGCHLD, 0);
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
                    memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), sizeof(f[j].filter_type));
                    //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                    printf("file filter: %s\n", file_filter);
                    printf("file input: %s\n", p.f_input);
                    printf("file output: %s\n", p.f_output);
                    //printf("hello\n");
                    filho[i] = fork();
                    if (filho[i] == 0)
                    { // Processo filho que trata do processamento
                        signal(SIGCHLD, 0);
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
                                //         //return 1;
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
                    memcpy(&file_filter[sizeof(filters_folder)], &(f[j].filter_type), sizeof(f[j].filter_type));
                    //printf("file filter type size: %ld\n", strlen(f[j].filter_type));
                    printf("file filter: %s\n", file_filter);
                    printf("file input: %s\n", p.f_input);
                    printf("file output: %s\n", p.f_output);
                    //printf("hello\n");
                    filho[i] = fork();
                    if (filho[i] == 0)
                    { // Processo filho que trata do processamento
                        signal(SIGCHLD, 0);
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
                        // Envia sinal para incrementar o ficheiro de configuração
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
                // ESCREVER PARA O CLIENTE 
                // write(output, buffer, sizeof(buffer)); // servidor envia o resultado do pedido do cliente
                // printf("A repetir...\n");
            }
            printf("OUTPUT FEITO !\n");
            close(outputPipe[p.n_filtros][0]);
            close(fd_out);
        }
    }
    for(int i = 0; i < p.n_filtros; i++){
        int status;
        wait(&status);
    }
    _exit(0);
}

void ExecuteStatus(int output)
{ 
    char task_answer[1024];
    char str[2048];
    char pid[50];
    // ESCREVER PARA O CLIENTE 
    for(int i = 0; i < 20; i++){
        if(pid, pr[i].pid != -1){
            sprintf(pid,"%d", pr[i].pid);
            strcat(task_answer, "task with pid ");
            strcat(task_answer, pid);
            strcat(task_answer, ": ");
            strcat(task_answer, "transform");
            strcat(task_answer, " ");
            strcat(task_answer, pr[i].f_input);
            strcat(task_answer, " ");
            strcat(task_answer, pr[i].f_output);
            strcat(task_answer, " ");
            for(int j = 0; j < pr[i].numero_filtros[0]; j++){ // alto
                strcat(task_answer, "alto ");
            }
            for(int j = 0; j < pr[i].numero_filtros[1]; j++){ // alto
                strcat(task_answer, "baixo ");
            }
            for(int j = 0; j < pr[i].numero_filtros[2]; j++){ // alto
                strcat(task_answer, "eco ");
            }
            for(int j = 0; j < pr[i].numero_filtros[3]; j++){ // alto
                strcat(task_answer, "rapido ");
            }
            for(int j = 0; j < pr[i].numero_filtros[4]; j++){ // alto
                strcat(task_answer, "lento ");
            }
            strcat(task_answer, "\n");
        }
        
    }
    // printf("TASK_ANSWER: %s\n",task_answer);
    
    

    
    sprintf(str, "%sfilter alto: %d/%d (in use/total)\nfilter baixo: %d/%d (in use/total)\nfilter eco: %d/%d (in use/total)\nfilter rapido: %d/%d (in use/total)\nfilter lento: %d/%d (in use/total)\n", task_answer, check_filters[0].n_instancia, file_configuration[0].n_instancia, check_filters[1].n_instancia, file_configuration[1].n_instancia, check_filters[2].n_instancia, file_configuration[2].n_instancia, check_filters[3].n_instancia, file_configuration[3].n_instancia, check_filters[4].n_instancia, file_configuration[4].n_instancia);
    
    printf("str: %s", str);
    int x = strlen(str);
    printf("SIZOFFF X : %d\n", x);
    write(output, str, strlen(str)); // Escreve os filtros em uso e totais 
    memset(str, 0 ,sizeof(str));
}

int main(int argc, char *argv[])
{

    signal(SIGINT, siginthandler);
    signal(SIGCHLD, sigchldhandler);

    for(int k = 0; k < 20; k++){
        memset(&pr[k].numero_filtros, 0, sizeof(pr[k].numero_filtros));
        pr[k].pid = -1;
        memset(&pr[k].f_input, 0, sizeof(pr[k].f_input));
        memset(&pr[k].f_output, 0, sizeof(pr[k].f_output));
    }
    
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
        strcpy(check_filters[i].filter_type, file_configuration[i].filter_type);
        strcpy(check_filters[i].filter_name, file_configuration[i].filter_name); // Guarda na estrutura dos filtros em uso
        check_filters[i].n_instancia = file_configuration[i].n_instancia;
    
        //check_filters[i].filter_type = file_configuration[i].filter_type;
        printf("config.tipo: %s \n", file_configuration[i].filter_type);
        printf("config.nome: %s \n", file_configuration[i].filter_name);
        printf("config.instancias: %d \n", file_configuration[i].n_instancia);

    }

    // Aqui recebe os argumentos do cliente, após isso é fazer o processamento dos dados recebidos e executar.

    int stop = 0;

    pedido p1;
    bool execute = true;
    

    while (!stop)
    {
        while ((bufferSize = read(input, &p1, sizeof(pedido))) > 0)
        {
            execute = true;
            count_pedidos++;
            // printf("Pedido recebido tipo: %s\n", p1.tipo);
            // printf("Pedido recebido f_input: %s\n", p1.f_input);
            // printf("Pedido recebido f_output: %s\n", p1.f_output);
            //number_filters = p1.n_filtros;
            for (int i = 0; i < p1.n_filtros; i++)
            {
                //printf("Pedido recebido filtros: %s\n", p1.filtros[i]);
            }
            pedidoTransform task;
            //printf("Pedido recebido n_filtros: %d\n", p1.n_filtros);
            if (memcmp(p1.tipo, transform, strlen(transform)) == 0)
            { // se for transform a receber, processa os dados
                
                
                strcpy(task.f_input, p1.f_input);
                strcpy(task.f_output, p1.f_output);
                task.n_filtros = p1.n_filtros;
                for (int i = 0; i < p1.n_filtros; i++)
                {
                    strcpy(task.filtros[i], p1.filtros[i]);
                }
                printf("AQUIII FDP: %s\n", task.f_input);
                printf("AQUIII FDP: %s\n", task.f_output);
                printf("AQUIII FDP: %d\n", task.n_filtros);

                // Recebe pedido e cria filho para tratar do pedido
                // Dentro da função,
                int counter_alto = 0;
                int counter_baixo = 0;
                int counter_eco = 0;
                int counter_rapido = 0;
                int counter_lento = 0;
                
                for (int i = 0; i < p1.n_filtros; i++)
                {
                    if (memcmp(p1.filtros[i], "alto", strlen("alto")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        { // counter = 5
                            if (memcmp(file_configuration[j].filter_name, "alto", strlen("alto")) == 0)
                            {
                                counter_alto++;
                                printf("ALTO\n");
                                printf("Instancias deste tipo inicial: %d\n", check_filters[j].n_instancia); // TEM DE DEVOLVER 1
                                check_filters[j].n_instancia = check_filters[j].n_instancia - 1;
                                if (check_filters[j].n_instancia < 0){
                                    execute = false;
                                    break;
                                }
                                printf("Instancias deste tipo decrementado: %d\n", check_filters[j].n_instancia);
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "baixo", strlen("baixo")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "baixo", strlen("baixo")) == 0)
                            {
                                counter_baixo++;
                                printf("BAIXO\n");
                                printf("Instancias deste tipo inicial: %d\n", check_filters[j].n_instancia); // TEM DE DEVOLVER 1
                                check_filters[j].n_instancia = check_filters[j].n_instancia - 1;
                                if (check_filters[j].n_instancia < 0){
                                    execute = false;
                                    break;
                                }
                                printf("Instancias deste tipo decrementado: %d\n", check_filters[j].n_instancia);
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "eco", strlen("eco")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "eco", strlen("eco")) == 0)
                            {
                                counter_eco++;
                                printf("ECO\n");
                                printf("Instancias deste tipo inicial: %d\n", check_filters[j].n_instancia); // TEM DE DEVOLVER 1
                                check_filters[j].n_instancia = check_filters[j].n_instancia - 1;
                                if (check_filters[j].n_instancia < 0){
                                    execute = false;
                                    break;
                                }
                                printf("Instancias deste tipo decrementado: %d\n", check_filters[j].n_instancia);
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "rapido", strlen("rapido")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "rapido", strlen("rapido")) == 0)
                            {
                                counter_rapido++;
                                printf("RAPIDO\n");
                                printf("Instancias deste tipo inicial: %d\n", check_filters[j].n_instancia); // TEM DE DEVOLVER 1
                                check_filters[j].n_instancia = check_filters[j].n_instancia - 1;
                                if (check_filters[j].n_instancia < 0){
                                    execute = false;
                                    break;
                                }
                                printf("Instancias deste tipo decrementado: %d\n", check_filters[j].n_instancia);
                            }
                        }
                    }
                    if (memcmp(p1.filtros[i], "lento", strlen("lento")) == 0)
                    {
                        for (int j = 0; j < counter; j++)
                        {
                            if (memcmp(file_configuration[j].filter_name, "lento", strlen("lento")) == 0)
                            {
                                counter_lento++;
                                printf("LENTO\n");
                                printf("Instancias deste tipo inicial: %d\n", check_filters[j].n_instancia); // TEM DE DEVOLVER 1
                                check_filters[j].n_instancia = check_filters[j].n_instancia - 1;
                                if (check_filters[j].n_instancia < 0){
                                    execute = false;
                                    break;
                                }
                                printf("Instancias deste tipo decrementado: %d\n", check_filters[j].n_instancia);
                                //printf("Instancias deste tipo: %d\n ", file_configuration[j].n_instancia);
                            }
                        }
                    }
                }
                if (execute == false)
                {
                    printf("Não pode ser processado o pedido!\n");
                    for(int j = 0; j < 5; j++){
                        if(memcmp(check_filters[j].filter_name, "alto", strlen("alto")) == 0)
                            check_filters[j].n_instancia = check_filters[j].n_instancia + counter_alto;
                        if(memcmp(check_filters[j].filter_name, "baixo", strlen("brapido")) == 0)
                            check_filters[j].n_instancia = check_filters[j].n_instancia + counter_baixo;
                        if(memcmp(check_filters[j].filter_name, "eco", strlen("eco")) == 0)
                            check_filters[j].n_instancia = check_filters[j].n_instancia + counter_eco;
                        if(memcmp(check_filters[j].filter_name, "rapido", strlen("rapido")) == 0)
                            check_filters[j].n_instancia = check_filters[j].n_instancia + counter_rapido;
                        if(memcmp(check_filters[j].filter_name, "lento", strlen("lento")) == 0)
                            check_filters[j].n_instancia = check_filters[j].n_instancia + counter_lento;
                        printf("check_filters[j].n_instancia: %d\n", check_filters[j].n_instancia);
                    }
                            
                }

                if (execute == true)
                {
                    int k;
                    for(k = 0; k < 20; k++){
                        printf("pr[k].pid: %d\n", pr[k].pid);
                        if(pr[k].pid == -1){
                            pr[k].numero_filtros[0] = counter_alto;
                            pr[k].numero_filtros[1] = counter_baixo; // alto(0), baixo(1), eco(2), rapido(3), lento(4)
                            pr[k].numero_filtros[2] = counter_eco;
                            pr[k].numero_filtros[3] = counter_rapido;
                            pr[k].numero_filtros[4] = counter_lento;
                            strcpy(pr[k].f_input , p1.f_input);
                            strcpy(pr[k].f_output , p1.f_output);
                            pr[k].pid = -1;
                            break;
                        }
                    }

                    pr[k].pid = fork();

                    
        
                    printf("prk pid: %d\n", pr[k].pid);

                    if (pr[k].pid == 0)
                    {
                        signal(SIGCHLD, 0);
                        printf("PID do filho: %d\n", getpid());
                        //_exit(0);
                        ExecuteTransform(p1, file_configuration, counter, output);
                        //_exit(0);
                    }
                    
                    
                    
                    // sinal é chamado
                }

                // chama função que processa os dados e retorna o resultado
                // faz write desse resultado
            }
            // printf("heelo\n");
            // // process pr;
            // printf("helllllo\n");
            // if(pr.type == "rapido"){
            //     printf("PID correspondente: %d\n", pr.pid);

            // }
            if (memcmp(p1.tipo, status, strlen(status)) == 0)
            { // se for status a receber, envia o status do servidor
                printf("ENTROU STATUS \n");
                if(!fork()){
                    ExecuteStatus(output);
                    _exit(0);
                }
                // chama função que faz o status e retorna o resultado
                // faz write desse resultado
            }
        }
    }

    kill(getpid(), SIGINT);
    return 0;
}