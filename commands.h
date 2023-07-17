#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include "GPS.h"
long id_cliente;
//Estrutura utilizada para armazenar informações relacionadas a um socket de servidor.
struct nodo
{
    int server_socket;
};
//Essa declaração cria um array chamado nodo com tamanho 1,
//onde cada elemento do array é do tipo struct nodo.
//Usado para armazenar o socket do servidor
struct nodo nodo[1];
pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;
int tempFechar=0,fechada=0, tempFlag =0,opacidade=0, valorOpac=0;

pthread_mutex_t respostaMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger o acesso à mensagem de resposta


void estadoJanela()
{
    if(writer!=-1)
    {
        pthread_mutex_lock(&globalMutex);

        char tempResposta[256]; // Variável temporária para armazenar a mensagem de resposta

        if ((temperatura <= tempFechar && tempFechar!=0) || fechada==1 )
        {
            strcpy(tempResposta, "\nEstado janela: fechada!\n");

        }
        else
        {
            strcpy(tempResposta, "\nEstado janela: aberta! \n");

            if (tempFechar == 0)
            {
                strcat(tempResposta, "\nDeseja determinar uma temperatura para FECHAR a janela? Digite -- fechar janela em <temperatura desejada>--\n\n");
            }
        }

        pthread_mutex_lock(&respostaMutex); // Bloqueia o acesso de outras threads à variável mensagemResposta
        if(tempFlag==1 || fechada==1 || fechada==0)
        {
            tempFlag=0;
            strcat(mensagemResposta, tempResposta); // Concatena a mensagem temporária à mensagem de resposta global
        }

        else
        {
            snprintf(mensagemResposta,sizeof(mensagemResposta),"%s",tempResposta);
        }

        pthread_mutex_unlock(&respostaMutex); // Libera o acesso para outras threads

        pthread_mutex_unlock(&globalMutex);
        pthread_cond_signal(&condVar);
    }
}

void estadoOpacidadeJanela()
{
    char tempResposta[256]; // Variável temporária para armazenar a mensagem de resposta

    pthread_mutex_lock(&globalMutex);
    if(valorOpac==0)
    {

        strcpy(tempResposta, "Estado janela: Transparente!\n");
        strcat(mensagemResposta,tempResposta);
    }
    else if(valorOpac==25)
    {

        strcpy(tempResposta, "Estado janela: 25% de Opacidade!\n");
        strcat(mensagemResposta,tempResposta);
    }
    else if(valorOpac==50)
    {

        strcpy(tempResposta, "Estado janela: 50% de Opacidade!\n");
        strcat(mensagemResposta,tempResposta);
    }
    else if(valorOpac==75)
    {

        strcpy(tempResposta, "Estado janela: 75% de Opacidade!\n");
        strcat(mensagemResposta,tempResposta);
    }
    else if(valorOpac==100)
    {

        strcpy(tempResposta, "Estado janela: 100% de Opacidade!\n");
        strcat(mensagemResposta,tempResposta);
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);

}


void *temperaturaPeriodica (void *arg)
{
    struct periodic_info info;
    srand( (unsigned)time(NULL) );

    make_periodic (15000000, &info);
    while (1)
    {
        temp++;

        temperatura= rand() % 51 - 10;  // Gera um valor aleatório entre 0 e 50, depois subtrai 10
        wait_period (&info);
    }
    return NULL;
}
void temperaturaAtual()
{
    srand( (unsigned)time(NULL) );
    tempFlag=1;
    double myDouble = nan("-nan");


    pthread_mutex_lock(&globalMutex);
    if(isnan(temperatura))
    {
        writer = -1;
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite outro comando! \n");
    }
    else
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta) - 1,"\nA temperatura atual é: %.2f °C\n", temperatura);
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);

    estadoJanela();
}
void *umidadePeriodica(void *arg)
{
    struct periodic_info info1;
    srand( (unsigned)time(NULL) );

    make_periodic (15000000, &info1);
    while (1)
    {
        umid++;

        umidade= rand() % 101;
        wait_period (&info1);
    }
    return NULL;
}
void umidadeAtual()
{

    pthread_mutex_lock(&globalMutex);
    if (umidade <= 0)
    {
        writer = -1;
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite outro comando! \n");

    }
    else
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta) - 1,"\nA umidade atual é: %.2f%%\n\n", umidade);
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);

}


void fecharJanelaEm (int temp)
{
    tempFechar=temp;
    pthread_mutex_lock(&globalMutex);
    if(tempFechar==0)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"\nTemperatura não definida! Defina usando  -- fechar janela em <temperatura desejada> --\n");

    }
    else
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"\nTemperatura definida para fechar janela em: %d °C!\n",tempFechar);
        fechada=1;
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);
}

void fecharJanela()
{
    fechada = 1;
    int contador=0;
    pthread_mutex_lock(&globalMutex);

    for(contador=0; contador<=10; contador++)
    {

        snprintf(mensagemResposta,sizeof(mensagemResposta)-1,"Fechando janela...%d\n",contador);
        write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR);
        sleep(1);
    }
    pthread_cond_signal(&condVar);
    pthread_mutex_unlock(&globalMutex);

    estadoJanela();
}


void abrirJanela()
{
    fechada=0;
    int contador=0;
    pthread_mutex_lock(&globalMutex);
    for(contador=0; contador<=10; contador++)
    {

        snprintf(mensagemResposta,sizeof(mensagemResposta)-1,"Abrindo janela...%d\n",contador);
        write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR);
        sleep(1);
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);

    estadoJanela();
}

void maisOpaca()
{
    int contador=0;
    if(valorOpac>=0 && valorOpac<100)
    {
        pthread_mutex_lock(&globalMutex);
        valorOpac= valorOpac + 25;

        for(contador=0; contador<=10; contador++)
        {
            snprintf(mensagemResposta,sizeof(mensagemResposta)-1,"Escurecendo janela...%d\n",contador);
            write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR);
            sleep(1);
        }
        pthread_mutex_unlock(&globalMutex);
        pthread_cond_signal(&condVar);


    }
    else if(valorOpac>=100)
    {
        pthread_mutex_lock(&globalMutex);
        snprintf(mensagemResposta,sizeof(mensagemResposta) - 1,"\nLimite de solicitação de aumentar a opacidade foi atingido. Janela está na sua opacidade máxima!!\n");
        pthread_mutex_unlock(&globalMutex);
        pthread_cond_signal(&condVar);

    }

    estadoOpacidadeJanela();
}
void menosOpaca()
{
    int contador=0;

    if(valorOpac>0)
    {
        pthread_mutex_lock(&globalMutex);
        valorOpac= valorOpac -25;

        for(contador=0; contador<=10; contador++)
        {
            snprintf(mensagemResposta,sizeof(mensagemResposta)-1,"Clareando janela...%d\n",contador);
            write(nodo[id_cliente].server_socket, mensagemResposta, MSG_DO_SERVIDOR);
            sleep(1);
        }
        pthread_mutex_unlock(&globalMutex);
        pthread_cond_signal(&condVar);
    }
    else if(valorOpac==0)
    {
        pthread_mutex_lock(&globalMutex);
        snprintf(mensagemResposta,sizeof(mensagemResposta) - 1,"\nLimite de solicitação de diminuir a opacidade foi atingido. Janela está na sua transparência máxima!!\n");
        pthread_mutex_unlock(&globalMutex);
        pthread_cond_signal(&condVar);
    }

    estadoOpacidadeJanela();
}



void horario()
{
    pthread_mutex_lock(&globalMutex);

    if (gpsHoras!= 0 && gpsMins != 0  && gpsSegs!= 0)
    {
        snprintf(mensagemResposta, sizeof(mensagemResposta)-2, "\nA hora atual é %02d:%02d:%02d\n\n", gpsHoras,gpsMins,gpsSegs);
    }
    else
    {
        writer = -1;
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite outro comando! \n");
    }

    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);


}

void data()
{
    pthread_mutex_lock(&globalMutex);

    if(gpsDia!=0 && gpsMes!=0 && gpsAno!=0 )
    {
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nO dia de hoje é %02d/%02d/%02d\n\n", gpsDia,gpsMes,gpsAno);
    }
    else
    {
        writer =-1;
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite outro comando!\n ");
    }

    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);
}


void coordenadasGPS()
{
    pthread_mutex_lock(&globalMutex);

    if(t_lat!=0.0 && t_long !=0.0)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta) - 1, "\nAs coordenadas sao: %.6f de latitude e %.6f de longitude\n", t_lat,t_long);
    }
    else
    {
        writer =-1;
        snprintf(mensagemResposta, sizeof(mensagemResposta), "\nErro ao retornar resposta solicitada. Digite outro comando!\n ");
    }

    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);

}

void estadoAtualJanela()
{
    pthread_mutex_lock(&globalMutex);
    if(fechada==1 && valorOpac==0)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela fechada e transparente!\n");
    }
    else if(fechada==1 && valorOpac==25)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela fechada e 25%% de opacidade!\n");
    }
    else if(fechada==1 && valorOpac==50)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela fechada e 50%% de opacidade!\n");
    }
    else if(fechada==1 && valorOpac==75)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela fechada e 75%% de opacidade!\n");
    }
    else if(fechada==1 && valorOpac==100)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela fechada e 100%% de opacidade!\n");
    }
    else if(fechada==0 && valorOpac==0)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela aberta e transparente!\n");
    }
    else if(fechada==0 && valorOpac==25)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela aberta e 25%% de opacidade!\n");
    }
    else if(fechada==0 && valorOpac==50)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela aberta e 50%% de opacidade!\n");
    }
    else if(fechada==0 && valorOpac==75)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela aberta e 75%% de opacidade!\n");
    }
    else if(fechada==0 && valorOpac==100)
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Janela aberta e 100%% de opacidade!\n");
    }
    else
    {
        snprintf(mensagemResposta,sizeof(mensagemResposta),"Erro ao retornar mensagem!\n");
    }
    pthread_mutex_unlock(&globalMutex);
    pthread_cond_signal(&condVar);
}

