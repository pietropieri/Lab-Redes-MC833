/*  Alunos: Pietro Pieri - 257337
            Francisco Guimarães - 234589
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

// Definição da estrutura de da lista de musicas
typedef struct HashMusicas {
    char* key;
    Musica* musicas;
} HashMusicas;

typedef struct HashMap {
    HashMusicas** item;
    int size;
    int count;
} HashMap;

// Função para inserir uma música na lista ligada
void createMusica(int clientSocket) {
    Musica newMusica;
    char tempBuffer[200];  // Temp buffer for receiving data

    // Ask for and receive each field of Musica
    char *fields[] = {"ID", "Titulo", "Interprete", "Idioma", "Tipo", "Refrao", "Ano de Lancamento"};
    for (int i = 0; i < 7; i++) {
        strcpy(buffer, "Por favor, insira ");
        strcat(buffer, fields[i]);
        strcat(buffer, ": ");
        send(clientSocket, buffer, strlen(buffer), 0);
        
        memset(tempBuffer, 0, sizeof(tempBuffer));  // Clear temp buffer
        recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);
        
        // Convert and store the data in the newMusica struct
        switch (i) {
            case 0: newMusica.id = atoi(tempBuffer); break;
            case 1: strcpy(newMusica.titulo, tempBuffer); break;
            case 2: strcpy(newMusica.interprete, tempBuffer); break;
            case 3: strcpy(newMusica.idioma, tempBuffer); break;
            case 4: strcpy(newMusica.tipo, tempBuffer); break;
            case 5: strcpy(newMusica.refrao, tempBuffer); break;
            case 6: newMusica.ano_lancamento = atoi(tempBuffer); break;
        }
    }

    // Print each parameter of the new Musica
    printf("Nova Musica Criada:\n");
    printf("ID: %d\n", newMusica.id);
    printf("Titulo: %s\n", newMusica.titulo);
    printf("Interprete: %s\n", newMusica.interprete);
    printf("Idioma: %s\n", newMusica.idioma);
    printf("Tipo: %s\n", newMusica.tipo);
    printf("Refrao: %s\n", newMusica.refrao);
    printf("Ano de Lancamento: %d\n", newMusica.ano_lancamento);

    // Insert the new Musica into the HashMap
    // You need to implement this part according to your HashMap structure and insertion logic
    // insertMusica(newMusica);
    
    strcpy(buffer, "Musica inserida com sucesso!");
    send(clientSocket, buffer, strlen(buffer), 0);
}

void solicitarInfoMusica(int clientSocket) {
    //ToDo
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
        printf("[+]ListeningBBBBB....\n");
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

    
    // strcpy(buffer, "Digite 1 para inserir uma música, 2 para listar as músicas: ");
    // send(clientSocket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        // Clear the buffer at the beginning of each iteration
        memset(buffer, 0, sizeof(buffer));

        recv(clientSocket, buffer, 1024, 0);
        printf("Mensagem recebida do cliente: %s\n", buffer);
        sleep(1);

        // Process the command
        if (strcmp(buffer, "1") == 0) {
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "===== CRIANDO MUSICA =====:");
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            createMusica(clientSocket);
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
