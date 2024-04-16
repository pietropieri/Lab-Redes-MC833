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
#include <pthread.h>
#include <signal.h>

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

// Definição da estrutura da lista de musicas
typedef struct HashMusica {
    char* key;
    Musica* musica;
    struct HashMusica* next;
} HashMusica;

// Definicao do HashMap usado
typedef struct HashMap {
    HashMusica** item;
    int size;
    int count;
} HashMap;

// Inicializacao do HashMap de tamanho maximo "size"
HashMap* initHashMap(int size) {
    HashMap* hashMap = (HashMap*)malloc(sizeof(HashMap));
    hashMap->size = size;
    hashMap->count = 0;
    hashMap->item = (HashMusica**)calloc(size, sizeof(HashMusica*)); 
    return hashMap;
}

// Funcao auxiliar para enviar mensagens para os Clientes.
void sendMessage(int clientSocket, char* message) {
    char buffer[2048];
    strcpy(buffer, message);
    send(clientSocket, buffer, strlen(buffer), 0);
}

// Funcao Hash usada para calcular o index do HashMap
int hashFunction(int key, int size) {
    return key % size;
}

// Funcao que salva o HashMap (contendo informacoes das musicas) em um arquivo musicas.json
void saveHashMapToJson(HashMap* hashMap, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Erro ao abrir arquivo");
        return;
    }

    fprintf(fp, "{\n");
    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            fprintf(fp, "  \"%s\": {\n", entry->key);
            Musica* musica = entry->musica;
            fprintf(fp, "    \"id\": %d,\n", musica->id);
            fprintf(fp, "    \"titulo\": \"%s\",\n", musica->titulo);
            fprintf(fp, "    \"interprete\": \"%s\",\n", musica->interprete);
            fprintf(fp, "    \"idioma\": \"%s\",\n", musica->idioma);
            fprintf(fp, "    \"tipo\": \"%s\",\n", musica->tipo);
            fprintf(fp, "    \"refrao\": \"%s\",\n", musica->refrao);
            fprintf(fp, "    \"ano_lancamento\": \"%s\"\n", musica->ano_lancamento);
            fprintf(fp, "  },%s\n", entry->next != NULL ? "," : "");
            entry = entry->next;
        }
    }
    fprintf(fp, "}\n");

    fclose(fp);
}


void insertMusica(HashMap* hashMap, Musica newMusica, int fromJson) {
    int index = hashFunction(newMusica.id, hashMap->size);

    HashMusica* newEntry = (HashMusica*)malloc(sizeof(HashMusica));
    newEntry->key = (char*)malloc(20);
    sprintf(newEntry->key, "%d", newMusica.id);
    newEntry->musica = (Musica*)malloc(sizeof(Musica));
    memcpy(newEntry->musica, &newMusica, sizeof(Musica));
    newEntry->next = NULL;

    if (hashMap->item[index] == NULL) {
        hashMap->item[index] = newEntry;
    } else {
        HashMusica* current = hashMap->item[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newEntry;
    }
    hashMap->count++;
    
    if (fromJson == 0) {
        saveHashMapToJson(hashMap, "musicas.json");
    }
    
}

void loadHashMapFromJson(HashMap* hashMap, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Erro ao abrir arquivo");
        return;
    }

    char line[512];  // Buffer para armazenar cada linha do arquivo
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Ignorar linhas que não contêm dados relevantes
        if (strstr(line, "\"id\":")) {
            Musica newMusica;
            int id;
            char key[20];

            // Parse ID
            sscanf(strstr(line, "\"id\":") + 5, "%d", &id);
            newMusica.id = id;
            sprintf(key, "%d", id);

            // Parse título
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"titulo\": \"%[^\"]\"", newMusica.titulo);

            // Parse intérprete
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"interprete\": \"%[^\"]\"", newMusica.interprete);

            // Parse idioma
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"idioma\": \"%[^\"]\"", newMusica.idioma);

            // Parse tipo
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"tipo\": \"%[^\"]\"", newMusica.tipo);

            // Parse refrão
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"refrao\": \"%[^\"]\"", newMusica.refrao);

            // Parse ano de lançamento
            fgets(line, sizeof(line), fp);
            sscanf(line, " \"ano_lancamento\": \"%[^\"]\"", newMusica.ano_lancamento);

            // Insere a música no HashMap
            insertMusica(hashMap, newMusica, 1);
        }
    }

    fclose(fp);
}


// Função para inserir uma música na lista ligada
void createMusica(int clientSocket, HashMap* hashMap) {
    Musica newMusica;
    char tempBuffer[200];

    char *fields[] = {"ID", "Titulo", "Interprete", "Idioma", "Tipo", "Refrao", "Ano de Lancamento"};
    for (int i = 0; i < 7; i++) {
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "Por favor, insira ");
        strcat(buffer, fields[i]);
        strcat(buffer, ": ");
        sendMessage(clientSocket, buffer);
        
        memset(tempBuffer, 0, sizeof(tempBuffer)); 
        recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);
        
        // Caso o cliente nao informe o refrao, este campo sera substituido por "Não tem refrão"
        if (i == 5 && strlen(tempBuffer) == 0) {
            strcpy(tempBuffer, "Não tem refrão");
        }

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
    
    insertMusica(hashMap, newMusica, 0);
    sleep(1);

    memset(buffer, 0, sizeof(buffer));
    sendMessage(clientSocket, "Musica inserida com sucesso!");
}

// Funcao que envia para o cliente as informacoes de todas as musicas
void printHashMap(int clientSocket, HashMap* hashMap) {
    hashMap = initHashMap(8);
    loadHashMapFromJson(hashMap, "musicas.json");
    char message[4096] = "";


    if (hashMap->count == 0) {
        memset(buffer, 0, sizeof(buffer));
        sendMessage(clientSocket, "Listagem de músicas: Nenhuma musica registrada");
        return; 
    }   
    
    strcat(message, "Listagem de músicas:\n");

    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            Musica* musica = entry->musica;

            char musicaInfo[512];
            snprintf(musicaInfo, sizeof(musicaInfo),
                     "ID: %d\n\tTitulo: %s\n\tInterprete: %s\n\tIdioma: %s\n\tTipo: %s\n\tRefrao: %s\n\tAno de Lancamento: %s\n\n",
                     musica->id, musica->titulo, musica->interprete, musica->idioma, musica->tipo, musica->refrao, musica->ano_lancamento);

            strcat(message, musicaInfo);

            entry = entry->next;
        }
    }

    if (strlen(message) > 0) {
        sendMessage(clientSocket, message);
    }
    
    printf("----------------------------\n");
}

// Funcao que lista as musicas dado um Ano de Lancamento
void listarMusicasPorAno(int clientSocket, HashMap* hashMap) {
    hashMap = initHashMap(8);
    loadHashMapFromJson(hashMap, "musicas.json");
    char tempBuffer[200];

    memset(tempBuffer, 0, sizeof(tempBuffer)); 
    recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);
    printf("%s", tempBuffer);

    char message[4096] = "Músicas lançadas em ";
    strcat(message, tempBuffer);
    strcat(message, ":\n");

    int found = 0;

    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            if (strcmp(entry->musica->ano_lancamento, tempBuffer) == 0) {
                char musicaInfo[256];
                Musica* musica = entry->musica;
                snprintf(musicaInfo, sizeof(musicaInfo),
                     "ID: %d\n\tTitulo: %s\n\tInterprete: %s\n\tIdioma: %s\n\tTipo: %s\n\tRefrao: %s\n\tAno de Lancamento: %s\n\n",
                     musica->id, musica->titulo, musica->interprete, musica->idioma, musica->tipo, musica->refrao, musica->ano_lancamento);

                strcat(message, musicaInfo);
                found = 1;
            }
            entry = entry->next;
        }
    }

    if (!found) {
        strcat(message, "Nenhuma música encontrada.");
    }

    sendMessage(clientSocket, message);
}

// Funcao que lista as musicas dado um Idioma e Ano de Lancamento
void listarMusicasPorIdiomaEAno(int clientSocket, HashMap* hashMap, char* idioma, char* ano) {
    hashMap = initHashMap(8);
    loadHashMapFromJson(hashMap, "musicas.json");
    char message[4096] = "Músicas em ";
    strcat(message, idioma);
    strcat(message, " lançadas em ");
    strcat(message, ano);
    strcat(message, ":\n");

    int found = 0;

    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            if (strcmp(entry->musica->idioma, idioma) == 0 && strcmp(entry->musica->ano_lancamento, ano) == 0) {
                char musicaInfo[256];
                Musica* musica = entry->musica;
                snprintf(musicaInfo, sizeof(musicaInfo),
                     "ID: %d\n\tTitulo: %s\n\tInterprete: %s\n\tIdioma: %s\n\tTipo: %s\n\tRefrao: %s\n\tAno de Lancamento: %s\n\n",
                     musica->id, musica->titulo, musica->interprete, musica->idioma, musica->tipo, musica->refrao, musica->ano_lancamento);

                strcat(message, musicaInfo);
                found = 1;
            }
            entry = entry->next;
        }
    }

    if (!found) {
        strcat(message, "Nenhuma música encontrada.");
    }

    sendMessage(clientSocket, message);
}

// Funcao que lista as musicas dado um Tipo
void listarMusicasPorTipo(int clientSocket, HashMap* hashMap) {
    hashMap = initHashMap(8);
    loadHashMapFromJson(hashMap, "musicas.json");
    char tempBuffer[200];

    memset(tempBuffer, 0, sizeof(tempBuffer)); 
    recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);
    printf("%s", tempBuffer);

    char message[4096] = "Músicas lançadas do tipo ";
    strcat(message, tempBuffer);
    strcat(message, ":\n");

    int found = 0;

    for (int i = 0; i < hashMap->size; i++) {
        HashMusica* entry = hashMap->item[i];
        while (entry != NULL) {
            if (strcmp(entry->musica->tipo, tempBuffer) == 0) {
                char musicaInfo[256];
                Musica* musica = entry->musica;
                snprintf(musicaInfo, sizeof(musicaInfo),
                     "ID: %d\n\tTitulo: %s\n\tInterprete: %s\n\tIdioma: %s\n\tTipo: %s\n\tRefrao: %s\n\tAno de Lancamento: %s\n\n",
                     musica->id, musica->titulo, musica->interprete, musica->idioma, musica->tipo, musica->refrao, musica->ano_lancamento);

                strcat(message, musicaInfo);
                found = 1;
            }
            entry = entry->next;
        }
    }

    if (!found) {
        strcat(message, "Nenhuma música encontrada.");
    }

    sendMessage(clientSocket, message);
}

// Funcao que lista as musicas dado um ID
void listarInformacoesMusicaPorID(int clientSocket, HashMap* hashMap) {
    hashMap = initHashMap(8);
    loadHashMapFromJson(hashMap, "musicas.json");
    char idBuffer[200];  // Buffer para receber o ID como string

    sendMessage(clientSocket, "Digite o ID da música:");
    memset(idBuffer, 0, sizeof(idBuffer));
    recv(clientSocket, idBuffer, sizeof(idBuffer), 0);

    int id = atoi(idBuffer);

    char message[1024];
    snprintf(message, sizeof(message), "Informações da música ID %d:\n", id);

    int index = hashFunction(id, hashMap->size);
    HashMusica* entry = hashMap->item[index];

    int found = 0;

    while (entry != NULL) {
        if (entry->musica->id == id) {
            char musicaInfo[512];
            snprintf(musicaInfo, sizeof(musicaInfo), "\tTítulo: %s\n\tIntérprete: %s\n\tIdioma: %s\n\tTipo: %s\n\tRefrão: %s\n\tAno de Lançamento: %s\n",
                     entry->musica->titulo, entry->musica->interprete, entry->musica->idioma, entry->musica->tipo, entry->musica->refrao, entry->musica->ano_lancamento);
            strcat(message, musicaInfo);
            found = 1;
            break; 
        }
        entry = entry->next;
    }

    if (!found) {
        strcat(message, "Música não encontrada.");
    }

    sendMessage(clientSocket, message);
}

// Funcao auxiliar para desligar o servidor com segurança
void signal_handler(int sig) {
    printf("\nDesligando Servidor...\n");
    exit(0);
}


// Funcao que deleta uma musica dado seu ID
void deleteMusica(HashMap* hashMap, int musicaID, int clientSocket) {
    int index = hashFunction(musicaID, hashMap->size);
    
    if (hashMap->item[index] == NULL) {
        printf("Música não encontrada.\n");
        return;
    }
    
    HashMusica* current = hashMap->item[index];
    HashMusica* prev = NULL;
    while (current != NULL) {
        if (atoi(current->key) == musicaID) {
            if (prev == NULL) {
                hashMap->item[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->key);
            free(current->musica);
            free(current);
            printf("Música com ID %d deletada com sucesso.\n", musicaID);
            sleep(1);
            saveHashMapToJson(hashMap, "musicas.json");
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("Música com ID %d não encontrada.\n", musicaID);
}

void* handle_client(void* arg) {
    int clientSocket = *(int*)arg;
    char buffer[1024];
    
    // Inicialize a HashMap

    
    // Lógica para lidar com o cliente
    while (1) {
        HashMap* hashMap = initHashMap(8); // Tamanho da HashMap, ajuste conforme necessário
        loadHashMapFromJson(hashMap, "musicas.json");
        // Limpa o buffer
        memset(buffer, 0, sizeof(buffer));

        // Recebe a mensagem do cliente
        recv(clientSocket, buffer, sizeof(buffer), 0);
        printf("Mensagem recebida do cliente: %s\n", buffer);
        sleep(2);
        
        if (strcmp(buffer, "1") == 0) {
            memset(buffer, 0, sizeof(buffer));
            createMusica(clientSocket, hashMap);
        } else if (strcmp(buffer, "2") == 0) { // Exibe informacoes de musicas registradas
            memset(buffer, 0, sizeof(buffer));
            printHashMap(clientSocket, hashMap);
        } else if (strcmp(buffer, "3") == 0) { // Deleta musica existente
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "===== DELETANDO MUSICA, DIGITE O ID: =====:\n");
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(clientSocket, buffer, 1024, 0);
            int musicaID = atoi(buffer);
            printf("%d", musicaID);
            deleteMusica(hashMap, musicaID, clientSocket);
            send(clientSocket, buffer, strlen(buffer), 0);
        } else if (strcmp(buffer, "4") == 0) {  // Lista musicas por ano
            sendMessage(clientSocket, "Digite o ano:");
            memset(buffer, 0, sizeof(buffer));
            listarMusicasPorAno(clientSocket, hashMap);
        } else if (strcmp(buffer, "5") == 0) {   // Lista musicas por Idioma e Ano
            char idiomaBuffer[200];
            char anoBuffer[200];
            sendMessage(clientSocket, "Digite o idioma:");
            memset(idiomaBuffer, 0, sizeof(idiomaBuffer));
            recv(clientSocket, idiomaBuffer, sizeof(idiomaBuffer), 0);
            sendMessage(clientSocket, "Digite o ano:");
            memset(anoBuffer, 0, sizeof(anoBuffer));
            recv(clientSocket, anoBuffer, sizeof(anoBuffer), 0);
            listarMusicasPorIdiomaEAno(clientSocket, hashMap, idiomaBuffer, anoBuffer);
        } else if (strcmp(buffer, "6") == 0) {  // Lista musicas por Tipo
            sendMessage(clientSocket, "Digite o tipo:");
            memset(buffer, 0, sizeof(buffer));
            listarMusicasPorTipo(clientSocket, hashMap);
        } else if (strcmp(buffer, "7") == 0) {  // Lista musicas por ID
            memset(buffer, 0, sizeof(buffer));
            listarInformacoesMusicaPorID(clientSocket, hashMap);  
        } else {
            memset(buffer, 0, sizeof(buffer));
            sendMessage(clientSocket, "Comando inválido!");
        }
    }

    // Fecha o socket do cliente
    close(clientSocket);
    free(arg);  // Libera a memória alocada para o clientSocket

    return NULL;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    int opt = 1;

    // Configuração de sinal SIGINT (CTRL+C)
    signal(SIGINT, signal_handler);

    // Inicialize o socket do servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Erro na criação do socket");
        exit(EXIT_FAILURE);
    }
    printf("[+]Socket do servidor criado.\n");

    // Configurações do socket
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erro ao definir as opções do socket");
        exit(EXIT_FAILURE);
    }

    // Configurações do endereço do servidor
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Associa o socket ao endereço e à porta
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Erro ao associar o socket");
        exit(EXIT_FAILURE);
    }
    printf("[+]Socket associado à porta %d\n", PORT);

    // Ouve até 10 conexões pendentes
    if (listen(serverSocket, 10) == 0) {
        printf("[+]Servidor ouvindo por conexões...\n");
    } else {
        perror("Erro ao iniciar a escuta de conexões");
        exit(EXIT_FAILURE);
    }

    // Aceita conexões dos clientes
    while (1) {
        addr_size = sizeof(newAddr);
        int* clientSocket = malloc(sizeof(int));  // Aloca memória para o socket do cliente
        *clientSocket = accept(serverSocket, (struct sockaddr*)&newAddr, &addr_size);
        if (*clientSocket < 0) {
            perror("Erro ao aceitar a conexão");
            free(clientSocket);  // Libera a memória alocada
            continue;
        }
        printf("[+]Conexão aceita de %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        // Crie uma nova thread para lidar com o cliente
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, clientSocket);
        // A nova thread lidará com o cliente
    }

    // Fecha o socket do servidor (em caso de saída do loop)
    close(serverSocket);

    return 0;
}