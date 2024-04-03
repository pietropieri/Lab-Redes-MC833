/*  Alunos: Pietro Pieri - 257337
           Francisco Guimarães - 
    Disciplina: Lab de redes- MC833
    Projeto 1:
    Desenvolve programas com uso de socket para
    comunicação cliente servidor
    
    SERVIDOR:*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444

char buffer[1024];

// Definição da estrutura para armazenar informações da música
typedef struct {
    int id;
    char titulo[100];
    char interprete[100];
    char idioma[50];
    char tipo[50];
    char refrao[200];
    int ano_lancamento;
} Musica;

// Definição da estrutura de nó da lista ligada
typedef struct Node {
    Musica musica;
    struct Node *next;
} Node;

Node *head = NULL; // Cabeça da lista ligada

// Função para inserir uma música na lista ligada
void insertMusica(Musica musica) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->musica = musica;
    newNode->next = head;
    head = newNode;
}

void printMusica(int clientSocket, Musica musica) {
    sprintf(buffer, "Identificador Único: %d\n", musica.id);
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Título: %s\n", musica.titulo);
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Intérprete: %s\n", musica.interprete);
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Idioma: %s\n", musica.idioma);
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Tipo de música: %s\n", musica.tipo);
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Refrão: %s\n", strlen(musica.refrao) > 0 ? musica.refrao : "Não possui");
    send(clientSocket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Ano de lançamento: %d\n", musica.ano_lancamento);
    send(clientSocket, buffer, strlen(buffer), 0);
}

void solicitarInfoMusica(int clientSocket) {
    Musica newMusica;

    // Solicita as informações da música ao cliente
    strcpy(buffer, "Digite o ID da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    newMusica.id = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "Digite o título da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    strcpy(newMusica.titulo, buffer);
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "Digite o intérprete/banda da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    strcpy(newMusica.interprete, buffer);
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "Digite o idioma da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    strcpy(newMusica.idioma, buffer);
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "Digite o tipo de música (pop, rock, MPB, ...): ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    strcpy(newMusica.tipo, buffer);
    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, "Digite o refrão da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    strcpy(newMusica.refrao, buffer);


    strcpy(buffer, "Digite o ano de lançamento da música: ");
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, buffer, 1024, 0);
    newMusica.ano_lancamento = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));

    // Insere a nova música na lista ligada
    insertMusica(newMusica);
    printMusica(clientSocket, newMusica);
    strcpy(buffer, "Musica cadastrada com sucesso!");
    send(clientSocket, buffer, strlen(buffer), 0);
}


int main() {
    int serverSocket, clientSocket, ret;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", PORT);

    if (listen(serverSocket, 10) == 0) {
        printf("[+]Listening....\n");
    } else {
        printf("[-]Error in listening.\n");
    }

    addr_size = sizeof(newAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&newAddr, &addr_size);
    if (clientSocket < 0) {
        printf("[-]Error in accepting.\n");
        exit(1);
    }
    printf("[+]Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    while (1) {
        strcpy(buffer, "Digite 1 para inserir uma música, 2 para listar as músicas: ");
        send(clientSocket, buffer, strlen(buffer), 0);
        memset(buffer, 0, sizeof(buffer));

        recv(clientSocket, buffer, 1024, 0);
        printf("Mensagem recebida do cliente: %s\n", buffer);

        if (strcmp(buffer, "1") == 0) {
           solicitarInfoMusica(clientSocket);
        } else if (strcmp(buffer, "2") == 0) {
            // Lógica para listar as músicas (não implementada neste exemplo)
            strcpy(buffer, "Listagem de músicas:");
            send(clientSocket, buffer, strlen(buffer), 0);
        } else {
            strcpy(buffer, "Comando inválido!");
            send(clientSocket, buffer, strlen(buffer), 0);
        }
    }

    return 0;
}
