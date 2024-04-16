/*  Alunos: Pietro Pieri - 257337
            Francisco Guimarães - 
    Disciplina: Lab de redes- MC833
    Projeto 1:
    Desenvolve programas com uso de socket para
    comunicação cliente servidor
    
    Cliente:*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 4444

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	char address[16];

	// Definicao do IP do Servidor
	printf("Digite o IP do servidor com o qual quer se conectar (com pontuacoes): ");
    scanf("%s", &address);

	// Configuracoes basicas do socket do Client
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Erro na conexao\n");
		exit(1);
	}
	printf("[+]Socket do Cliente criado.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(address);

	// Estabelecendo conexao com o servidor, no IP informado
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Erro na conexao\n");
		exit(1);
	}
	printf("[+]Conectado ao servidor.\n");

	printf("Digite:\n1 para inserir uma música\n2 para listar as músicas\n3 para deletar uma musica pelo ID\n4 para listar as musicas de um ano\n5 para listar as musicas em um idioma em um ano\n6 para listar as musicas de um tipo\n7 para listar as musicas por ID\n");

	// Looping que ira manter o client conectado enviando e recebendo informacoes
	int c;
	while((c = getchar()) != '\n' && c != EOF) { }
	while(1){


		printf("Cliente:");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = 0;
		send(clientSocket, buffer, strlen(buffer), 0);
		memset(buffer, 0, sizeof(buffer));

		if(strcmp(buffer, ":exit") == 0){
			close(clientSocket);
			printf("[-]Desconectado do Servidor.\n");
			exit(1);
		}

		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Erro ao receber dados.\n");
		}else{
			printf("Servidor: \t%s\n", buffer);
		}
	}

	return 0;
}