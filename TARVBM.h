#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define t 3 // ORDEM (t) da árvore
#define MAX_CHAVES (2 * t - 1) // 5 chaves
#define MAX_FILHOS (2 * t)     // 6 filhos
#define MAX_PREMIOS 20 // Quantidade máxima de prêmios para Estandarte de Ouro

typedef struct EstandarteOuro{
    char categoria_estandarte[50]; 
    char vencedor_estandarte[100];
    int qtd_estandartes;
} Premio;

typedef struct registro {
    
    // Arquivo Campeãs do Canarval Carioca

    int ano; // Ano em que a escola foi campeã
    char escola_campea[100]; // Nome da escola campeã
    int qtd_vitorias; // Número de vezes que a escola foi campeã
    char enredo_campea[200]; // Tema da aprensentação
    char carnavalesco[100]; // Nome da pessoa que organizou o desfile dessa escola
    char vice_campea[100]; // Nome da escola vice campea

    // Arquivo Estandartes de Ouro
    Premio estandartes[MAX_PREMIOS]; // Guardar dados do Estandarte de Ouro
    int qtd_estandartes;

} RegistroDados;

// Estrutura para o arquivo de ÍNDICE (Nós Internos apenas)
typedef struct arvbmIndice{
    int nchaves; // Número de chaves (máx.: 2*t - 1)
    int folha; // Indica se o Nó é folha
    int chaves[MAX_CHAVES]; // As chaves(índices)
    
    // Se esse nó aponta para outro nó interno, 'filhos' contém o OFFSET no indice.bin
    // Se esse nó aponta para uma folha, 'filhos' contém o ID DO ARQUIVO DE DADOS.

    long filhos[MAX_FILHOS]; 
    
    int nivel; // Ajuda a saber se os filhos são outros nós internos ou arquivos de folha
} NoIndice;


// Estrutura para os arquivos de DADOS (Cada folha é um arquivo)
typedef struct arvbmFolha{
    int nchaves; // Número de chaves (máx.:2*t - 1)
    
    // Chaves e Dados
    RegistroDados dados[MAX_CHAVES];
    
    int id_prox_folha; 
} NoFolha;


typedef struct EstruturaDeControle{ // Ficam no ínicio do arquivo indice.bin
    long offset_raiz; // Localização da raiz 
    int contador_folhas; // Para saber qual será a próxima folha a ser criada (Começa em 0)
} Superbloco;

// Protótipos das Funções 

void inicializar_banco();
void buscar(int ano_busca);
void imprimir_registro(RegistroDados reg);
