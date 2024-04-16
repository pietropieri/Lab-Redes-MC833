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

void sendMessage(int clientSocket, char* message) {
    char buffer[2048]; // Aumente o tamanho do buffer se necessário
    strcpy(buffer, message);
    send(clientSocket, buffer, strlen(buffer), 0);
}

int hashFunction(int key, int size) {
    return key % size;
}

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
    
    if (fromJson == 0) {
        saveHashMapToJson(hashMap, "musicas.json");
    }
    
}


// Função para inserir uma música na lista ligada
void createMusica(int clientSocket, HashMap* hashMap) {
    Musica newMusica;
    char tempBuffer[200];  // Temp buffer for receiving data

    // Ask for and receive each field of Musica
    char *fields[] = {"ID", "Titulo", "Interprete", "Idioma", "Tipo", "Refrao", "Ano de Lancamento"};
    for (int i = 0; i < 7; i++) {
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "Por favor, insira ");
        strcat(buffer, fields[i]);
        strcat(buffer, ": ");
        sendMessage(clientSocket, buffer);
        
        memset(tempBuffer, 0, sizeof(tempBuffer));  // Clear temp buffer
        recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);
        
        if (i == 5 && strlen(tempBuffer) == 0) {
            strcpy(tempBuffer, "Não tem refrão");
        }

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
    
    insertMusica(hashMap, newMusica, 0);

    // strcpy(buffer, "Musica inserida com sucesso!");
    memset(buffer, 0, sizeof(buffer));
    sendMessage(clientSocket, "Musica inserida com sucesso!");
}

void solicitarInfoMusica(int clientSocket) {
    //ToDo
}

void printHashMap(int clientSocket, HashMap* hashMap) {
    char message[4096] = "";


    if (hashMap->count == 0) {
        // char* message = "Nenhuma musica registrada";
        memset(buffer, 0, sizeof(buffer));
        sendMessage(clientSocket, "Listagem de músicas: Nenhuma musica registrada");
        return; // Early return if no music is registered
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

void listarMusicasPorAno(int clientSocket, HashMap* hashMap) {
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

void listarMusicasPorIdiomaEAno(int clientSocket, HashMap* hashMap, char* idioma, char* ano) {
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

void listarMusicasPorTipo(int clientSocket, HashMap* hashMap) {
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

void listarInformacoesMusicaPorID(int clientSocket, HashMap* hashMap) {
    char idBuffer[200];  // Buffer para receber o ID como string

    // Solicita e recebe o ID do usuário
    sendMessage(clientSocket, "Digite o ID da música:");
    memset(idBuffer, 0, sizeof(idBuffer));
    recv(clientSocket, idBuffer, sizeof(idBuffer), 0);

    // Converte o ID de string para inteiro
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
            break;  // Encerra o loop assim que encontrar a música
        }
        entry = entry->next;
    }

    if (!found) {
        strcat(message, "Música não encontrada.");
    }

    sendMessage(clientSocket, message);
}


void signal_handler(int sig) {
    printf("\nShutting down server...\n");
    exit(0); // Exit program
}

void deleteMusica(HashMap* hashMap, int musicaID, int clientSocket) {
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
            printHashMap(clientSocket, hashMap);
            printf("Música com ID %d deletada com sucesso.\n", musicaID);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    // Se o ID da música não for encontrado na lista ligada correspondente ao índice
    printf("Música com ID %d não encontrada.\n", musicaID);
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

int main() {
    int serverSocket, clientSocket, ret;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    int opt = 1;

    signal(SIGINT, signal_handler);

    // Initialize HashMap
    HashMap* hashMap = initHashMap(8); // Example size, adjust as needed

    loadHashMapFromJson(hashMap, "musicas.json");

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

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
            createMusica(clientSocket, hashMap);
        } else if (strcmp(buffer, "2") == 0) {
            memset(buffer, 0, sizeof(buffer));
            printHashMap(clientSocket, hashMap);
        } else if (strcmp(buffer, "3") == 0) {
            // Lógica para listar as músicas
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "===== DELETANDO MUSICA, DIGITE O ID: =====:\n");
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(clientSocket, buffer, 1024, 0);
            int musicaID = atoi(buffer);
            printf("%d", musicaID);
            deleteMusica(hashMap, musicaID, clientSocket);
            send(clientSocket, buffer, strlen(buffer), 0);
        } else if (strcmp(buffer, "4") == 0) {  // Listar músicas por ano
            sendMessage(clientSocket, "Digite o ano:");
            memset(buffer, 0, sizeof(buffer));
            listarMusicasPorAno(clientSocket, hashMap);
        } else if (strcmp(buffer, "5") == 0) {  // Listar músicas por idioma e ano
            char idiomaBuffer[200];
            char anoBuffer[200];
            sendMessage(clientSocket, "Digite o idioma:");
            memset(idiomaBuffer, 0, sizeof(idiomaBuffer));
            recv(clientSocket, idiomaBuffer, sizeof(idiomaBuffer), 0);
            sendMessage(clientSocket, "Digite o ano:");
            memset(anoBuffer, 0, sizeof(anoBuffer));
            recv(clientSocket, anoBuffer, sizeof(anoBuffer), 0);
            listarMusicasPorIdiomaEAno(clientSocket, hashMap, idiomaBuffer, anoBuffer);
        } else if (strcmp(buffer, "6") == 0) {  // Listar músicas por tipo
            sendMessage(clientSocket, "Digite o tipo:");
            memset(buffer, 0, sizeof(buffer));
            listarMusicasPorTipo(clientSocket, hashMap);
        } else if (strcmp(buffer, "7") == 0) {  // Listar músicas por tipo
            // sendMessage(clientSocket, "Digite o ID:");
            memset(buffer, 0, sizeof(buffer));
            listarInformacoesMusicaPorID(clientSocket, hashMap);  
        } else {
            memset(buffer, 0, sizeof(buffer));
            sendMessage(clientSocket, "Comando inválido!");
        }
    }

    close(serverSocket);

    return 0;
}
