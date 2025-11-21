#include "TARVBM.h"

void inicializar_banco() {

    // Verificando se o arquivo já existe
    FILE *arq_ind = fopen("indice.bin", "rb");
    if (arq_ind){ // Se entrar, então o arquivo já existe
        printf("O banco de dados ja foi inicializado!\n");
        fclose(arq_ind);
        return;
    }

    arq_ind = fopen("indice.bin", "wb+");
    
    Superbloco sb; // sb = super bloco
    sb.offset_raiz = sizeof(Superbloco); // Calcula o tamanho em bytes da estrutura e informa a posição inicial da raiz
    sb.contador_folhas = 1;  // A folha 0 será criada nessa função, então já incrementa 1 nesse contador
    
    // Criar a Raiz (que é um Nó Índice)
    NoIndice raiz;
    raiz.nchaves = 0; // Por enquanto nenhuma chave foi inserida
    raiz.folha = 1; // Inicialmente é uma folha, pois não possui filhos
    
    // Inicializa filhos com -1
    for(int i=0; i<MAX_FILHOS; i++){ // t = 3, então são 6 filhos
        raiz.filhos[i] = -1;
    }

    // A raiz aponta para o arquivo "folha_0.bin"
    raiz.filhos[0] = 0; // ID do arquivo da primeira folha
    
    // Gravar o Superbloco e Raiz no indice.bin
    fwrite(&sb, sizeof(Superbloco), 1, arq_ind); // Grava no início do arquivo
    fwrite(&raiz, sizeof(NoIndice), 1, arq_ind); // Será gravado após o Supwrbloco
    fclose(arq_ind); 

    FILE *arq_folha = fopen("dados/folha_0.bin", "rb");
    if (!arq_folha){ // Se entrar, então possivelmente a pasta 'dados' naõ existe
        printf("ERRO: Crie a pasta 'dados' no diretorio do projeto!\n");
        exit(1);
    }

    // Criando o arquivo físico "folha_0.bin" dentro de uma pasta "dados"
    arq_folha = fopen("dados/folha_0.bin", "wb");
    
    NoFolha folha_inicial; // 
    folha_inicial.nchaves = 0; // Por enqunato nenhuma chave foi inserida
    folha_inicial.id_prox_folha = -1; // Não tem irmão à direita ainda
    
    // Grava a folha 0 dentro do arquivo folha_0.bin
    fwrite(&folha_inicial, sizeof(NoFolha), 1, arq_folha); // folha_inicial == "folha_0.bin"
    fclose(arq_folha);

    // Testando pra ver se o cod rodou, === DEPOIS APAGA ISSO ===
    printf("Banco inicializado: indice.bin + folha_0.bin criados.\n");
}

void imprimir_registro(RegistroDados reg) {
    printf("\n========================================\n");
    printf("DADOS DO ANO: %d\n", reg.ano);
    printf("========================================\n");
    printf("[CAMPEAS]\n");
    printf("Escola Campea: %s\n", reg.escola_campea);
    printf("Enredo: %s\n", reg.enredo_campea);
    printf("Carnavalesco: %s\n", reg.carnavalesco);
    printf("Vice-Campea: %s\n", reg.vice_campea);
    
    printf("\n[ESTANDARTES DE OURO]\n");
    if (reg.qtd_estandartes == 0) {
        printf("Nenhum registro de estandarte para este ano.\n");
    } 
    else {
        for (int i = 0; i < reg.qtd_estandartes; i++) {
            printf("- %s: %s\n", reg.estandartes[i].categoria_estandarte, reg.estandartes[i].vencedor_estandarte);
        }
    }
    printf("========================================\n");
}

void buscar(int ano_busca) {
    // Abre o arquivo de índice
    FILE *arq_ind = fopen("indice.bin", "rb");
    if (!arq_ind) {
        printf("Erro: Nao foi possivel abrir o indice. Execute a inicializacao primeiro.\n");
        return;
    }

    // Ler o Superbloco para achar a Raiz
    Superbloco sp;
    fread(&sp, sizeof(Superbloco), 1, arq_ind);
    
    // Pula para a raiz
    fseek(arq_ind, sp.offset_raiz, SEEK_SET);

    NoIndice no_atual; // Para a leitura de fread
    fread(&no_atual, sizeof(NoIndice), 1, arq_ind);

    // Navegação nos Nós Internos (Enquanto não apontar para folha externa)
    // só vai rodar quando a árvore tiver mais de 1 nível de altura.
    while (no_atual.folha == 0) { // Enquanto o nó atual NÃO for o último nível do índice (que aponta p/ arquivos)
        int i = 0;
        // Procura qual filho descer
        // Se i > nchaves, então está fora do escopo do vetor
        while (i < no_atual.nchaves && ano_busca >= no_atual.chaves[i]) {
            i++;
        }
        
        // 'i' agora é o índice do ponteiro que devemos seguir
        long proximo_offset = no_atual.filhos[i]; // Enquanto no_autal.folha == 0, os filhos dele serão offsets (número de bytes exato para localizar outro Nó)
        
        // Lê o próximo nó interno do disco
        fseek(arq_ind, proximo_offset, SEEK_SET); // Faz o deslocamento a partir do início do arquivo
        fread(&no_atual, sizeof(NoIndice), 1, arq_ind); // Faz a leitura de outro Nó
    }

    // AQUI CHEGAMOS NO NÓ QUE APONTA PARA ARQUIVOS
    // 'no_atual' agora tem folha == 1.
    // Seus 'filhos' não são offsets, são IDs de ARQUIVO (0, 1, 2...).
    
    int i = 0;
    // Se i > nchaves, então está fora do escopo do vetor
    // Se ano_busca > chaves[i], então o ano buscado não existe 
    while (i < no_atual.nchaves && ano_busca > no_atual.chaves[i]) {
        i++;
    }
    
    // Pegamos o ID do arquivo de dados
    int id_arquivo_dados = (int)no_atual.filhos[i]; // Os filhos de no_atual são folhas, então o vetor irá passar o ID dessas folhas
    fclose(arq_ind); // Não precisamos mais desse arquivo

    // Abrir o Arquivo de Folha Específico
    char nome_arquivo[100];
    sprintf(nome_arquivo, "dados/folha_%d.bin", id_arquivo_dados);
    

    // Verificando se o arquivo existe
    FILE *arq_folha = fopen(nome_arquivo, "rb");
    if (!arq_folha) {
        printf("O arquivo nao existe!\n");
        return;
    }

    NoFolha folha;
    fread(&folha, sizeof(NoFolha), 1, arq_folha);
    fclose(arq_folha);

    // dfghdgfhfdghhDSFSDFSDF Busca Sequencial na Folha (Memória RAM) fghfdhgfdhfdgSDASDFDSFSD
    // Agora temos os dados na mão. Vamos procurar o ano.
    int encontrado = 0;
    for (int j = 0; j < folha.nchaves; j++) {
        if (folha.dados[j].ano == ano_busca) { // Estrutura: NoFolha; vetor: dados[i]; int: ano
            imprimir_registro(folha.dados[j]);
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Registro do ano %d nao encontrado no banco de dados.\n", ano_busca);
    }
}
