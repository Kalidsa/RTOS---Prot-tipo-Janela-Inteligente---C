//servidor

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#include "commands.h"


//sincronizar threads no escopo do servidor.c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int processandoComando = 0;


void *cliente(void *arg)
{
    struct periodic_info information;
    id_cliente= (long)arg;
    int idClientes, codRet;
    char buffer[MSG_DO_CLIENTE],
         buffer_msg_cliente[MSG_DO_CLIENTE];


//criação do tokenizer que permitira mais de um comando concatenados gerando um unico


    while (1)
    {

        char* token;
        char *saveptr;
        token=NULL;
        bzero(buffer, sizeof(buffer)); //A função está sendo utilizada para zerar o conteúdo do buffer
        codRet = read(nodo[id_cliente].server_socket, buffer, MSG_DO_CLIENTE);


        if (codRet == -1)
        {
            fprintf(stderr, "Erro lendo mensagem do cliente\n");
            close(nodo[id_cliente].server_socket);
            nodo[id_cliente].server_socket = -1;
            pthread_exit(NULL);

        }
        else if (codRet == 0)
        {
            printf("Cliente %ld desconectou\n",id_cliente);
            close(nodo[id_cliente].server_socket);
            nodo[id_cliente].server_socket= -1;
            pthread_exit(NULL);
        }

        sscanf(buffer, "%[^\n]", buffer_msg_cliente);  // Extrai a string até encontrar uma quebra de linha
        printf("Recebeu do cliente: %s - tamanho do buffer: %lu\n \n", buffer_msg_cliente, strlen(buffer_msg_cliente));

        char buffer_copy[MSG_DO_CLIENTE];
        strcpy(buffer_copy, buffer_msg_cliente);

        token = strtok(buffer_copy, " ");

        if(processandoComando==1)
        {
            // MUTEX LOCK - CONDVAR
            pthread_mutex_lock(&mutex);

            while (processandoComando == 1)
            {
                snprintf(mensagemResposta, sizeof(mensagemResposta), "Servidor ocupado com"
                         "outro comando no momento. Adicionado à lista de espera da variavel de condicao\n");
                pthread_cond_wait(&condVar, &mutex);
                printf("Retomando execução do comando efetuado pelo cliente\n");
            }
            processandoComando=0;
            pthread_mutex_unlock(&mutex);
        }
        // MUTEX UNLOCK - CONDVAR

        /*
         * strcasecmp é usado para fazer a comparação sem considerar
         * case (maiúsculo ou minúsculo).
         *
         */
        if (strcasecmp(buffer_msg_cliente, "ajuda") == 0)
        {
            strcpy(mensagemResposta, "\nComandos: horas ou horario, data, temperatura ou tempo, coordenadas,"
                   "definir fechar janela, definir abrir janela, mais opacidade, menos opacidade,  estado janela, opacidade, close the window, open the window, sair\n");
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1)
            {
                printf("Erro ao retornar mensagem de ajuda\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta) -1, "Erro ao retornar resposta solicitada. Digite outro comando!\n ");
            }
            //Horário
        }
        else if (strcasecmp(buffer_msg_cliente, "horas" ) == 0|| strcasecmp(buffer_msg_cliente, "horario") == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            horario();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                printf("Erro ao retornar mensagem de horario\n");
                writer =0;
            }
        } //data
        else if (strcasecmp(buffer_msg_cliente, "data") == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            data();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                printf("Erro ao retornar mensagem da data\n");
                writer =0;
            }

        }
        // Temperatura
        else if (strcasecmp(buffer_msg_cliente, "temperatura") == 0 || strcasecmp(buffer_msg_cliente, "tempo" ) == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            temperaturaAtual();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 )
            {
                printf("Erro ao retornar mensagem do temperatura atual\n");
            }
        }
        //Umidade
        else if (strcasecmp(buffer_msg_cliente, "umidade") == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            umidadeAtual();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                printf("Erro ao retornar mensagem da umidade atual\n");
                writer =0;
            }
        }
        //Coordenadas
        else if (strcasecmp(buffer_msg_cliente, "coordenadas") == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            coordenadasGPS();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                printf("Erro ao retornar mensagem de coordenadas\n");
                writer =0;
            }
        }
        // Verifica o token para determinar a ação a ser executada
        else if (token != NULL && strcasecmp(token, "fechar") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "janela") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token

                if (token != NULL && strcasecmp(token, "em") == 0)
                {
                    token = strtok(NULL, " "); // Obtém o próximo token

                    if (token != NULL)
                    {
                        pthread_mutex_lock(&mutex);
                        processandoComando=1;
                        temperatura_requerida = atoi(token);
                        fecharJanelaEm(temperatura_requerida);
                        pthread_cond_signal(&condVar);
                        pthread_mutex_unlock(&mutex);
                    }
                    else
                    {
                        fprintf(stderr, "Erro: TOKEN de digítido para temperatura não encontrado!\n");
                        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- fechar janela em <temperatura> --\n");
                    }
                }
                else
                {
                    fprintf(stderr, "Erro: TOKEN em não encontrado!\n");
                    snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- fechar janela em <temperatura> --\n");
                }
            }
            else
            {
                fprintf(stderr, "Erro: TOKEN janela não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- fechar janela em <temperatura> --\n");
            }
            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }

        }
        else if (token != NULL && strcasecmp(token, "close") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "the") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token

                if (token != NULL && strcasecmp(token, "window") == 0)
                {
                    token = strtok(NULL, " "); // Obtém o próximo token
                    pthread_mutex_lock(&mutex);
                    processandoComando=1;
                    fecharJanela();
                    pthread_cond_signal(&condVar);
                    pthread_mutex_unlock(&mutex);
                }
                else
                {
                    fprintf(stderr, "Erro: TOKEN window não encontrado!\n");
                    snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- close the window--\n");
                }
            }
            else
            {
                fprintf(stderr, "Erro: token the não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- close the window --\n");

            }

            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }

        }
        else if (token != NULL && strcasecmp(token, "open") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "the") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token

                if (token != NULL && strcasecmp(token, "window") == 0)
                {
                    token = strtok(NULL, " "); // Obtém o próximo token
                    pthread_mutex_lock(&mutex);
                    processandoComando=1;
                    abrirJanela();

                    pthread_cond_signal(&condVar);
                    pthread_mutex_unlock(&mutex);


                }
                else
                {
                    fprintf(stderr, "Erro: TOKEN window não encontrado!\n");
                    snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- open the window--\n");

                }
            }
            else
            {
                fprintf(stderr, "Erro: token the não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- open the window--\n");

            }

            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }
        }
        else if (token != NULL && strcasecmp(token, "mais") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "opacidade") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token
                pthread_mutex_lock(&mutex);
                processandoComando=1;
                maisOpaca();

                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&condVar);

            }
            else
            {
                fprintf(stderr, "Erro: TOKEN opacidade não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- mais opacidade--\n");

            }

            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }
        }
        else if (token != NULL && strcasecmp(token, "menos") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "opacidade") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token
                pthread_mutex_lock(&mutex);
                processandoComando=1;
                menosOpaca();
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&condVar);

            }
            else
            {
                fprintf(stderr, "Erro: TOKEN opacidade não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- menos opacidade--\n");

            }

            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }
        }
        else if (token != NULL && strcasecmp(token, "estado") == 0)
        {
            token = strtok(NULL, " "); // Obtém o próximo token

            if (token != NULL && strcasecmp(token, "janela") == 0)
            {
                token = strtok(NULL, " "); // Obtém o próximo token
                pthread_mutex_lock(&mutex);
                processandoComando=1;
                estadoAtualJanela();
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&condVar);

            }
            else
            {
                fprintf(stderr, "Erro: TOKEN janela não encontrado!\n");
                snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite -- estado janela--\n");

            }

            if (write(nodo[id_cliente].server_socket, mensagemResposta, strlen(mensagemResposta)) == -1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de resposta\n");

            }
        }
        else if (strcasecmp(buffer_msg_cliente, "opacidade") == 0)
        {
            pthread_mutex_lock(&mutex);
            processandoComando=1;
            estadoOpacidadeJanela();
            pthread_cond_signal(&condVar);
            pthread_mutex_unlock(&mutex);
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                printf("Erro ao retornar mensagem da opacidade atual\n");
                writer =0;
            }
        }

        else
        {
            printf("Erro - comando desconhecido\n");
            snprintf(mensagemResposta, sizeof(mensagemResposta) -1, "O servidor não reconheceu a sua mensagem como um comando,"
                     "digite \"ajuda\" para ver quais comandos são válidos\n");
            if (write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR) == -1 || writer==-1)
            {
                fprintf(stderr, "Erro ao enviar a mensagem de aviso\n");
            }
        }

        processandoComando = 0;
        pthread_cond_signal(&condVar);
        pthread_mutex_unlock(&mutex);
        // MUTEX UNLOCK - ENCERRANDO PROCESSAMENTO

    }





}

int main(int argc, char *argv[])
{

    //tarefas periódicas =  temperatura e umidade
    pthread_t temperatura;
    pthread_t umidade;
    sigset_t alarm_sig; //variável é usada para armazenar um conjunto de sinais.
    int i;
    sigemptyset (&alarm_sig); //inicializa o conjunto de sinais alarm_sig, removendo todos os sinais do conjunto.
    for (i = SIGRTMIN; i <= SIGRTMAX; i++)
    {
        sigaddset (&alarm_sig, i);//adiciona cada sinal ao conjunto alarm_sig
    }
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
    //bloqueia todos os sinais contidos no conjunto alarm_sig.
    //Isso significa que os sinais adicionados ao conjunto alarm_sig não serão entregues ao processo enquanto estiverem bloqueados.

    //criacao de threads periodicas
    pthread_create (&temperatura, NULL, temperaturaPeriodica, NULL) ;
    pthread_create (&umidade, NULL, umidadePeriodica, NULL) ;

    //criação da thread que irá coletar as informações do GPS
    pthread_t t_gps;
    int codRetGps;


    struct sockaddr_in server_addr, client_addr;
    socklen_t cliCompr; //indica o tamanho em bytes da estrutura de endereço do cliente passada como argumento para a função

    int serverSocket, portNum, codRet, flagErroDuranteLockGeral = 0;
    pthread_t thread;
    long idCliente =0;



    if (argc != 2)
    {
        printf("Erro: Porta não definida. Uso: %%s porta\n");
    }
    else
    {
        portNum=atoi(argv[1]); //numero da porta utilizada na comunicação do servidor e cliente
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
        {
            printf("Erro abrindo o socket\n");
            return -1;
        }
        else
        {
            bzero((char *) &server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET; // Define a família de endereços como IPv4
            server_addr.sin_addr.s_addr = INADDR_ANY; //indica que o servidor aceita conexoes de maquinas com qualquer endereco de ip
            server_addr.sin_port = htons(portNum);  // Define o número da porta como o valor passado na linha de comando

            // verificação para lidar com possíveis erros durante a associação do socket
            if (bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)  //associar um endereço IP e número de porta a um socket.
            {
                printf("Essa porta esta sendo usada, escolha outra!\n");
                return -1;
            }
            else
            {
                //listen() serve para colocar o socket em um estado de escuta, permitindo que ele aceite conexões de clientes.
                if (listen(serverSocket, TOTAL_CLIENTES) == -1)
                {
                    printf("Erro executando listen\n");
                    return -1;
                }
            }
        }
    }
    codRetGps=pthread_create (&t_gps, NULL,gps, NULL) ;
    //retorna se a criação da thread foi bem sucedida ou não, se for diferente de zero é porque ocorreu erro
    if (codRetGps)
    {
        fprintf(stderr, "Erro ao criar a thread para o GPS\n");
        pthread_mutex_unlock(&mutex);
        return -1;

    }
    else
    {
        variavelGPS=1; //serve para ativar o GPS

    }

    printf("Esperando por conexões\n");
    // Inicializa o descritor de socket de cada um dos clientes

    for (idCliente = 0; idCliente < TOTAL_CLIENTES; idCliente++)
    {
        nodo[idCliente].server_socket = -1;
    }
    while (1)
    {
        /*
         * Este loop garante que os IDs sejam reutilizados para novos
         * clientes que se conectarem ao servidor.
         *
        */
        for (idCliente = 0; idCliente < TOTAL_CLIENTES; idCliente++)
        {
            if (nodo[idCliente].server_socket== -1)  //que percorre os elementos do vetor nodo para encontrar um elemento com server_socket igual a -1.
            {
                //Isso significa que esse elemento está disponível para ser associado a uma nova conexão de cliente.
                break;
            }
        }
        nodo[idCliente].server_socket = accept(serverSocket, (struct sockaddr *) &client_addr, &cliCompr); // para aceitar uma conexão de cliente em um socket de servidor.
        //uma vez que um elemento com server_socket = -1 foi encontrado, ele entra no accept para realizar a conexão com o socket
        //e o mutex é bloqueado para garantir exclusão mútua durante o acesso compartilhado aos recursos do servidor

        // MUTEX LOCK - GERAL
        pthread_mutex_lock(&mutex);
        if (nodo[idCliente].server_socket == -1)
        {
            fprintf(stderr, "Erro no accept com o cliente\n");
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        else
        {
            printf("Cliente %ld conectado e número de descritor de socket %d\n",id_cliente, nodo[idCliente].server_socket);
        }
        //Se o número de clientes conectados for menor que 1, uma nova thread é criada para lidar com esse cliente. Caso contrário, uma mensagem de conexão recusada é enviada para o cliente e o socket é fechado.
        //ou seja, se idCliente for menor que 1, então significa que há posição disponível para um novo cliente no vetor nodo, que recebe o valor do total de clientes que podem acessar o servidor
        if (idCliente < TOTAL_CLIENTES)
        {
            codRet = pthread_create(&thread, NULL, cliente, (void *)idCliente);

            if (codRet)
            {
                fprintf(stderr, "Erro ao criar a thread: %d\n", codRet);
                return -1;

            }
        }
        else
        {
            printf("Socket %d terá que ser desfeito pois já há o máximo de conexões ativas no momento\n", nodo[idCliente].server_socket);
            pthread_mutex_unlock(&mutex);
            char mensagemConexaoRecusada[MSG_DO_SERVIDOR] = "Conexao recusada, apenas UM cliente por vez!\n";

            if (write(nodo[idCliente].server_socket, mensagemConexaoRecusada, MSG_DO_SERVIDOR) == -1)  //é usada para escrever dados em um descritor de socket
            {
                printf("Erro ao enviar a mensagem de aviso de que a conexão foi recusada\n");
                return -1;
            }
            if (close(nodo[idCliente].server_socket) == -1)  // a conexão com esse cliente é encerrada e o descritor de socket é liberado
            {
                perror("Houve erro ao fechar o socket\n");
                return -1;
            }
            else
            {
                printf("Socket %d foi desfeito com sucesso\n", nodo[idCliente].server_socket);
                break;
            }
            break;
        }
        // MUTEX UNLOCK - GERAL
        pthread_mutex_unlock(&mutex);
    }



    return 0;
}
