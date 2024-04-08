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
    char ano_lancamento[100];
} Musica;

// Definição da estrutura de da lista de musicas
typedef struct HashMusica {
    char* key;
    Musica* musica;
    struct HashMusica* next;
} HashMusica;

typedef struct HashMap {
    HashMusica** item;
    int size;
    int count;
} HashMap;

HashMap* initHashMap(int size) {
    HashMap* hashMap = (HashMap*)malloc(sizeof(HashMap));
    hashMap->size = size;
    hashMap->count = 0;
    hashMap->item = (HashMusica**)calloc(size, sizeof(HashMusica*));  // Allocate and initialize with NULL
    return hashMap;
}

int hashFunction(int key, int size) {
    return key % size;
}

void insertMusica(HashMap* hashMap, Musica newMusica) {
    printf("NEW MUSICA");
    int index = hashFunction(newMusica.id, hashMap->size);

    HashMusica* newEntry = (HashMusica*)malloc(sizeof(HashMusica));
    newEntry->key = (char*)malloc(20);  // Assuming an int can be converted to a string of max length 20
    sprintf(newEntry->key, "%d", newMusica.id);
    newEntry->musica = (Musica*)malloc(sizeof(Musica));
    memcpy(newEntry->musica, &newMusica, sizeof(Musica));
    newEntry->next = NULL;

    // Handle collision
    if (hashMap->item[index] == NULL) {
        hashMap->item[index] = newEntry;
    } else {
        // Append to the end of the list at this index
        HashMusica* current = hashMap->item[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newEntry;
    }
    hashMap->count++;
}


// Função para inserir uma música na lista ligada
void createMusica(int clientSocket, HashMap* hashMap) {
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
            case 6: strcpy(newMusica.ano_lancamento, tempBuffer); break;
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
    printf("Ano de Lancamento: %s\n", newMusica.ano_lancamento);

    // Insert the new Musica into the HashMap
    // You need to implement this part according to your HashMap structure and insertion logic
    // insertMusica(newMusica);
    
    insertMusica(hashMap, newMusica);

    strcpy(buffer, "Musica inserida com sucesso!");
    send(clientSocket, buffer, strlen(buffer), 0);
}

void solicitarInfoMusica(int clientSocket) {
    //ToDo
}

void printHashMap(HashMap* hashMap) {
    printf("------ HashMap Data ------\n");
    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            Musica* musica = entry->musica;
            printf("Bucket %d -> Key: %s\n", i, entry->key);
            printf("\tID: %d\n", musica->id);
            printf("\tTitulo: %s\n", musica->titulo);
            printf("\tInterprete: %s\n", musica->interprete);
            printf("\tIdioma: %s\n", musica->idioma);
            printf("\tTipo: %s\n", musica->tipo);
            printf("\tRefrao: %s\n", musica->refrao);
            printf("\tAno de Lancamento: %s\n", musica->ano_lancamento);
            entry = entry->next;
        }
    }
    printf("----------------------------\n");
}


void deleteMusica(HashMap* hashMap, int musicaID) {
    // Calcula o índice na tabela hash para o ID da música
    int index = hashFunction(musicaID, hashMap->size);
    
    // Verifica se existe uma entrada no índice calculado
    if (hashMap->item[index] == NULL) {
        printf("Música não encontrada.\n");
        return;
    }
    
    // Caso haja uma ou mais entradas no índice calculado, percorre-se a lista ligada
    // para encontrar a entrada correspondente ao ID da música
    HashMusica* current = hashMap->item[index];
    HashMusica* prev = NULL;
    while (current != NULL) {
        // Se o ID da música for encontrado na entrada atual
        if (atoi(current->key) == musicaID) {
            // Remove a entrada
            if (prev == NULL) {
                // Se a entrada a ser removida for a primeira na lista ligada
                hashMap->item[index] = current->next;
            } else {
                // Se a entrada a ser removida estiver no meio ou no final da lista ligada
                prev->next = current->next;
            }
            // Libera a memória alocada para a entrada
            free(current->key);
            free(current->musica);
            free(current);
            printHashMap(hashMap);
            printf("Música com ID %d deletada com sucesso.\n", musicaID);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    // Se o ID da música não for encontrado na lista ligada correspondente ao índice
    printf("Música com ID %d não encontrada.\n", musicaID);
}


int main() {
    int serverSocket, clientSocket, ret;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;

    // Initialize HashMap
    HashMap* hashMap = initHashMap(8); // Example size, adjust as needed

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
            strcpy(buffer, "===== CRIANDO MUSICA =====:\n");
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            createMusica(clientSocket, hashMap);
        } else if (strcmp(buffer, "2") == 0) {
            // Lógica para listar as músicas
            strcpy(buffer, "Listagem de músicas:");
            send(clientSocket, buffer, strlen(buffer), 0);
            printHashMap(hashMap);
        } else if (strcmp(buffer, "3") == 0) {
            // Lógica para listar as músicas
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "===== DELETANDO MUSICA, DIGITE O ID: =====:\n");
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(clientSocket, buffer, 1024, 0);
            int musicaID = atoi(buffer);
            printf("%d", musicaID);
            deleteMusica(hashMap, musicaID);
            send(clientSocket, buffer, strlen(buffer), 0);
        } else {
            strcpy(buffer, "Comando inválido!");
            send(clientSocket, buffer, strlen(buffer), 0);
        }
    }


    return 0;
}
