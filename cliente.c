//Cliente
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MSG_DO_CLIENTE 20
#define MSG_DO_SERVIDOR 450


int serverSocket;

void *leitura(void *arg)
{
    char buffer[256];
    int codRet;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        codRet = recv(serverSocket, buffer, MSG_DO_SERVIDOR, 0);
        if (codRet == -1)
        {
            perror("Erro lendo do socket");
            exit(1);
        }
        if (codRet == 0)
        {
            printf("\nO socket foi encerrado pelo servidor\n");
            exit(0);
        }
        if(strlen(buffer) ==0 )
        {
            printf(" ");
        }
        else
        {
            printf("Mensagem do servidor: %s", buffer);
        }
    }
}

int main(int argc, char *argv[])
{
    int portNum;
    struct sockaddr_in server_addr;

    pthread_t thread;

    char buffer[MSG_DO_CLIENTE];
    if (argc != 3)
        printf("Uso: %s nomehost porta\n", argv[0]);
    else
    {
        portNum = atoi(argv[2]);
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
        {
            printf("Erro criando socket");
            return -1;
        }
        else
        {
            bzero((char *) &server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            inet_aton(argv[1], &server_addr.sin_addr);
            server_addr.sin_port = htons(portNum);
            if (connect( serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
                printf("Erro conectando");
            else
            {
                pthread_create(&thread, NULL, leitura, NULL);
                printf("Digite a mensagem (ou sair): ");
                do
                {
                    bzero(buffer, sizeof(buffer));
                    fgets(buffer, MSG_DO_CLIENTE, stdin);
                    if (send(serverSocket, buffer, strlen(buffer), 0) == -1)
                    {
                        printf("Erro escrevendo no socket, reconecte e tente novamente");
                        break;
                    }
                }
                while (strcmp(buffer, "sair\n") != 0);

                if (close(serverSocket) == -1)
                    printf("Erro fechando descritorSocket");
                return -1;
            }
        }
    }


    return 0;
}




