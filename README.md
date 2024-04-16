# Servidor e Cliente TCP

Este projeto contém os arquivos fonte em C para um servidor e um cliente TCP concorrente. Abaixo estão instruções detalhadas sobre como compilar e executar os programas.

## Pré-requisitos

Certifique-se de que você tem o GCC instalado em sua máquina para compilar os arquivos. Se você estiver usando uma distribuição Linux, o GCC geralmente já está instalado ou pode ser instalado através do gerenciador de pacotes da sua distribuição.

## Compilação dos programas

### Servidor

Para compilar o arquivo `Servidor.c` e criar o executável do servidor, use o seguinte comando no terminal:

```bash
gcc -o servidor Servidor.c
```

### Cliente

Para compilar o arquivo `Cliente.c` e criar o executável do cliente, use o seguinte comando no terminal:

```bash
gcc -o cliente Cliente.c
```

## Execução dos programas

### Servidor

Para iniciar o servidor, abra um terminal e execute o seguinte comando:

```bash
./servidor
```

### Cliente

Para iniciar o cliente, abra um novo terminal e execute o seguinte comando:

```bash
./cliente
```


## Conectando o Cliente ao Servidor

Após executar o cliente, será solicitado que você forneça o número de IP do servidor. Informe o número de IP da máquina que está hospedando o servidor.
