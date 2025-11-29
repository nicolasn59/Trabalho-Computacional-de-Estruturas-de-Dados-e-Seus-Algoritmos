#include "TARVBM.h"

void inicializar_banco() {

    // Verificando se o arquivo já existe
    FILE *arq_ind = fopen("indice.bin", "rb");
    if (arq_ind){ // Se entrar, então o arquivo já existe
        printf("O banco de dados ja foi inicializado!\n");
        fclose(arq_ind);
        return;
    }

    // Cria o arquivo de indice 
    arq_ind = fopen("indice.bin", "wb+");
    
    Superbloco sb; // sb = super bloco
    sb.offset_raiz = sizeof(Superbloco); // Calcula o tamanho em bytes da estrutura e informa a posição inicial da raiz
    sb.contador_folhas = 1;  // A folha 0 será criada nessa função, então já incrementa 1 nesse contador
    
    // Cria a raiz (que é um Nó Índice)
    NoIndice raiz;
    raiz.nchaves = 0; // Por enquanto nenhuma chave foi inserida
    raiz.folha = 1; // Inicialmente é uma folha, pois não possui filhos
    
    // Inicializa filhos com -1
    for(int i=0; i<MAX_FILHOS; i++){ // t = 3, então são 6 filhos
        raiz.filhos[i] = -1;
    }

    // A raiz aponta para o arquivo "folha_0.bin"
    raiz.filhos[0] = 0; // ID do arquivo da primeira folha
    
    // Grava o Superbloco e Raiz no indice.bin
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

void buscar_ano(int ano_busca) {
    // Abre o arquivo de índice
    FILE *arq_ind = fopen("indice.bin", "rb");
    if (!arq_ind) {
        printf("Erro: Nao foi possivel abrir o indice. Execute a inicializacao primeiro.\n");
        return;
    }

    // Lê o Superbloco para achar a raiz
    Superbloco sp;
    fread(&sp, sizeof(Superbloco), 1, arq_ind);
    
    // Pula para a raiz
    fseek(arq_ind, sp.offset_raiz, SEEK_SET);

    NoIndice no_atual; // Para a leitura de fread
    fread(&no_atual, sizeof(NoIndice), 1, arq_ind);

    // Navegação nos Nós Internos (Enquanto não apontar para folha externa)
    // só vai rodar quando a árvore tiver mais de 1 nível de altura.
    while (no_atual.folha == 0) { // Enquanto o nó atual não for o último nível do índice (que aponta p/ arquivos)
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

    // Aqui começa a busca pela folha com o ano que estamos procurando
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

    // Abre o Arquivo de folha específico
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

// Buffer temporário para 200 anos
RegistroDados buffer_carga[200]; 

// Função auxiliar para limpar o lixo de memória antes de começar
void limpar_buffer() {
    for (int i = 0; i < 200; i++) {
        buffer_carga[i].ano = 0; // 0 indica que o ano está vazio, ou seja, não foi escrito nenhum dado (Vai ser útil na função ler_arquivo_campea())
        buffer_carga[i].qtd_estandartes = 0; // 0 indica que está vazio
        // Preenchendo com strings (Para verificar se foi escrito algum dado)
        strcpy(buffer_carga[i].escola_campea, "");
        strcpy(buffer_carga[i].enredo_campea, "");
    }
}

// Converte o ano para índice do vetor
int ano_para_indice(int ano) {
    return ano - 1900; // Ajuste conforme necessário
}

// Função auxiliar para achar um ano (4 dígitos) no meio de sujeira tipo "1972"
int pegar_ano_arquivo(char *linha) {
    int len = strlen(linha); // 'len' recebe a quantidade de caracteres na linha
    for (int i = 0; i < len - 3; i++) {
        // Procura 4 dígitos seguidos
        if (isdigit(linha[i]) && isdigit(linha[i+1]) && isdigit(linha[i+2]) && isdigit(linha[i+3])) {
            char digitos[5]; // Vetor para armazenar os dígitos do ano
            strncpy(digitos, &linha[i], 4); // Copia os 4 dígitos seguidos para esse vetor
            digitos[4] = '\0'; // Indica o fim do vetor 'digitos'
            return atoi(digitos); // Converte string para inteiro
        }
    }
    return 0;
}

void ler_arquivo_campeas() {
    FILE *arq = fopen("Campeas.txt", "r");
    // Verifica se o arquivo foi aberto corretamente
    if (!arq) {
        printf("Erro ao abrir Campeas.txt\n");
        return;
    }

    // Armazena no vetor os caracteres de uma linha do arquivo
    char linha[1024];
    
    // 'fgets' está lendo cada caractere de uma linha do arquivo 
    // Após ler uma linha o cursor move para a próxima linha
    fgets(linha, sizeof(linha), arq); // Aqui é a primeira linah, então vai ser pulada

    // As linhas continuarão sendo lidas até chegar ao fim do arquivo
    while (fgets(linha, sizeof(linha), arq)) {

        // Remove a quebra de linha que foi lida na linha anterior
        linha[strcspn(linha, "\r\n")] = 0;
        
        // Extraindo o Ano de forma segura
        int ano = pegar_ano_arquivo(linha);
        if (ano < 1932 || ano > 2025){
            continue; // Ano inválido ou linha vazia
        }    

        int indice = ano_para_indice(ano);
        if(indice < 0 || indice >= 200){
            continue; // Índice inválido ou linha vazia
        }

        // Se já tiver lido esse ano (ex: empate de 1950), não sobrescreve o principal (escreve apenas o primero que aparecer)
        if (buffer_carga[indice].ano != 0) { // Não está vazio, algum ano já foi gravado nessa posição
            continue; // Para evitar sobrescrever dados duplicados em caso de empate
        }

        // Está vazio (nenhum ano não foi gravado nessa posição), então grava o ano no buffer
        buffer_carga[indice].ano = ano;

        // Tokenização com TABULAÇÃO (\t)
        // O formato do arquivo é: Ano \t Escola \t N.º \t Enredo ...
        
        // 1ª coluna: ano
        char *ptr = strtok(linha, "\t"); // Pega o Ano (já foi coletado, mas precisa avançar o ponteiro)
        
        // 2ª coluna: nome da escola
        char *escola = strtok(NULL, "\t");
        if(escola) { 
            strncpy(buffer_carga[indice].escola_campea, escola, 99); // Garante que que o texto não estoure o limite do vetor. Além disso, o 100º caractere é o '\0'
        }

        // 3ª Coluna: Nº Vitórias (Pula)
        // Aqui a gente pode fazer uma função pra contar a quantidade de vitórias das escolas em cada ano
        // Armazenar a quantidade vitórias de uma escola é um pouco redundante, pois se uma escola X pode vencer mais de uma vez, então seria necessário atualizar a quantidade de vitórias em todos os anos em que ela foi vencedora
        strtok(NULL, "\t"); 

        // 4ª Coluna: Enredo
        char *enredo = strtok(NULL, "\t");

        if(enredo){
            strncpy(buffer_carga[indice].enredo_campea, enredo, 199); // Garante que que o texto não estoure o limite do vetor (O último caractere é o '\0')
        } 

        // 5ª Coluna: Carnavalesco
        char *carnavalesco = strtok(NULL, "\t");

        if(carnavalesco) {
            strncpy(buffer_carga[indice].carnavalesco, carnavalesco, 99); // Garante que que o texto não estoure o limite do vetor (O último caractere é o '\0')

        }
        char *vice = strtok(NULL, "\t"); // 6ª Coluna: Vice
        if(vice) {
            strncpy(buffer_carga[indice].vice_campea, vice, 99); // Garante que que o texto não estoure o limite do vetor (O último caractere é o '\0')
        }
    }
    fclose(arq);
    printf("Arquivo de Campeas carregado no buffer!\n");
}

void ler_arquivo_estandartes() {
    FILE *arq = fopen("EstandartesOuro.txt", "r");
    if (!arq) { 
        printf("Erro ao abrir EstandartesOuro.txt\n"); 
        return; 
    }

    // Armazena no vetor os caracteres de uma linha do arquivo
    char linha[1024];

    int ano_atual = 0;

     // As linhas continuarão sendo lidas até chegar ao fim do arquivo
    while (fgets(linha, sizeof(linha), arq)) {

        // Remove a quebra de linha que foi lida na linha anterior
        linha[strcspn(linha, "\r\n")] = 0;

        // Verifica linhas vazias ou com lixo
        if (strlen(linha) < 2){
            continue;
        }
        
        // Tenta achar um ano na linha
        int ano_temp = pegar_ano_arquivo(linha);
        
        // Se achou um ano e a linha é curta
        if (ano_temp >= 1972 && strlen(linha) < 6) {
            ano_atual = ano_temp;
            continue;
        }

        // Se temos um ano válido selecionado, lemos o prêmio
        if (ano_atual > 0) {
            int indice = ano_para_indice(ano_atual);

            // Verifica se o indice é válido
            if(indice < 0 || indice >= 200) {
                continue;
            }

            // Se o registro não existe (ano sem campeã, mas com estandarte).
            // Por exemplo, em 1952 não teve escola campeã
            if (buffer_carga[indice].ano == 0){ // ano está vazio (nenhum dado foi escrito)
                buffer_carga[indice].ano = ano_atual;
            }

            // Atalho para struct (registro)
            RegistroDados *reg = &buffer_carga[indice];
            
            if (reg->qtd_estandartes < MAX_PREMIOS) { // qtd_estandarte foi defina na função limpar_buffer (inicialmente é 0)
                // Tenta quebrar por TAB
                // col = coluna
                char *col1 = strtok(linha, "\t"); // Categoria
                char *col2 = strtok(NULL, "\t");  // Vencedor (ou Escola)
                char *col3 = strtok(NULL, "\t");  // Detalhe (ou Pessoa)

                if (col1 && col2) {
                    // qtd_estandartes servirá com índice
                    int p = reg->qtd_estandartes;
                    
                    // Copia Categoria
                    strncpy(reg->estandartes[p].categoria_estandarte, col1, 49);

                    // Copia Vencedor.
                    // O arquivo às vezes tem: "Bateria \t Portela \t Mestre Cinco"
                    // Então juntamos Col2 e Col3 se existirem
                    if (col3) {
                        sprintf(reg->estandartes[p].vencedor_estandarte, "%s - %s", col2, col3);
                    } else {
                        strncpy(reg->estandartes[p].vencedor_estandarte, col2, 99);
                    }
                    // Incrementa qtd_estandartes para acessar o próximo índice
                    reg->qtd_estandartes++;
                }
            }
        }
    }
    fclose(arq);
    printf("Arquivo de Estandartes mesclado no buffer!\n");
}

// Função para carregar os arquivos e inserir
void executar_carga_inicial() {

    // Verificando se já existe registros no banca de dados
    FILE *arq_ind = fopen("indice.bin", "rb");
    if (arq_ind) {
        Superbloco sp;
        fread(&sp, sizeof(Superbloco), 1, arq_ind);

        // Ler a raiz para ver se ela tem chaves
        NoIndice raiz;
        fseek(arq_ind, sp.offset_raiz, SEEK_SET);
        fread(&raiz, sizeof(NoIndice), 1, arq_ind);
        
        fclose(arq_ind); // fechar antes de continuar ou retornar

        // Se a raiz tem chaves > 0, significa que a carga já foi feita antes
        if (raiz.nchaves > 0) {
            printf("AVISO: A carga inicial ja foi realizada anteriormente.\n");
            printf("O banco de dados ja contem registros.\n");
            return; // Cancela a função aqui
        }
    }

    // Se chegou aqui, o banco está vazio
    
    // Prepara a memória
    limpar_buffer();

    // Lê e mescla os arquivos
    ler_arquivo_campeas();
    ler_arquivo_estandartes();

    // Estão carregados no buffer_carga

    // Insere na Árvore B+
    printf("Inserindo na Arvore!\n");
    int cont = 0;
    
    for (int i = 0; i < 200; i++) {
        // Se o ano existe (foi preenchido), insere
        if (buffer_carga[i].ano != 0) { // Evite que anos que não possuem registros sejam inseridos (Só são inseridos anos de 1932 a 2025)
            inserir(&buffer_carga[i]); // Inserindo dados da posição 'i'
            cont++;
        }
    }
    printf("Inserção concluida! %d registros inseridos.\n", cont);
}

void inserir(RegistroDados *dado) {
    // Verifica se o arquivo foi aberto corretamente ou se existe
    FILE *arq_ind = fopen("indice.bin", "rb+");
    if (!arq_ind) {
        return;
    }

    // Lê o superbloco
    Superbloco sp;
    fread(&sp, sizeof(Superbloco), 1, arq_ind);

    // Chama a recursão a partir da raiz
    Promocao promo_raiz = inserir_recursivo(arq_ind, sp.offset_raiz, dado);

    // Se a raiz dividiu, será criada uma nova raiz
    if (promo_raiz.tem_promo == 1) {
        NoIndice nova_raiz;
        nova_raiz.nchaves = 1;
        nova_raiz.folha = 0; // Aponta para nós internos (a antiga raiz e a nova irma)
        nova_raiz.chaves[0] = promo_raiz.chave_promovida; // Sobe
        
        nova_raiz.filhos[0] = sp.offset_raiz; // Filho esq: Antiga raiz
        nova_raiz.filhos[1] = promo_raiz.id_novo_filho; // Filho dir: Nova irmã
        
        // Salva nova raiz no final do índice
        fseek(arq_ind, 0, SEEK_END); // Move o cursor para o fim do arquivo
        long novo_offset = ftell(arq_ind); // Pega a posição atual do cursor do arquivo
        fwrite(&nova_raiz, sizeof(NoIndice), 1, arq_ind); // Escreve a nova raiz no final do arquivo indice.bin

        // Atualiza Superbloco
        sp.offset_raiz = novo_offset; // Nova localização da raiz
        fseek(arq_ind, 0, SEEK_SET); // Move o cursor para o ínicio do arquivo
        fwrite(&sp, sizeof(Superbloco), 1, arq_ind); // Grava o Superbloco com novas informações
        
        printf("Nova raiz criada.\n");
    }

    fclose(arq_ind);
}

// Retorna uma Promocao caso o próprio nó interno divida
Promocao inserir_recursivo(FILE *arq_ind, long offset_atual, RegistroDados *dado) {
    // offset_atual == localização da raiz no arquivo (offset_raiz)
    
    NoIndice no;
    fseek(arq_ind, offset_atual, SEEK_SET); // Move o cursor do arquivo para a raiz
    fread(&no, sizeof(NoIndice), 1, arq_ind); // Lê o nó interno (raiz)

    int i = 0;
    while (i < no.nchaves && dado->ano > no.chaves[i]) { // Procura o índice do filho
        i++;
    }
    // 'i' é o índice do filho para onde vamos descer

    Promocao retorno_filho;

    // Caso base: O filho é uma folha (Arquivo externo)
    if (no.folha == 1) {
        int id_folha = (int)no.filhos[i];
        
        // O índice deve ser fechado aqui, pois 'inserir_na_folha' pode chamar 
        // 'criar_nova_folha', que tenta abrir o índice de novo.
        fclose(arq_ind); 
        
        retorno_filho = inserir_na_folha(id_folha, dado);
        
        // Reabre índice para continuar a execução deste nó
        arq_ind = fopen("indice.bin", "rb+");
    } 
    // Caso recursivo: O filho é outro nó interno
    else {
        long offset_filho = no.filhos[i];
        retorno_filho = inserir_recursivo(arq_ind, offset_filho, dado); // Até a folha
    }

    // Tratamento do retorno
    
    // Se o filho não dividiu (tem_promo == 0), não fazemos nada.
    if (retorno_filho.tem_promo == 0) {
        return retorno_filho;
    }

    // Se o filho dividiu, temos que inserir a chave promovida neste nó interno
    
    // Cenário 1: Tem espaço no nó interno?
    if (no.nchaves < MAX_CHAVES) {
        // Empurra chaves e filhos para a direita para abrir espaço
        for (int j = no.nchaves; j > i; j--) {
            // Abrindo espaço
            no.chaves[j] = no.chaves[j-1];
            no.filhos[j+1] = no.filhos[j];
        }

        // Insere a chave e o novo ponteiro (que veio da promoção)
        no.chaves[i] = retorno_filho.chave_promovida;
        no.filhos[i+1] = retorno_filho.id_novo_filho; // ID ou Offset
        no.nchaves++;
        
        // Salva e encerra (split não propaga mais)
        fseek(arq_ind, offset_atual, SEEK_SET);
        fwrite(&no, sizeof(NoIndice), 1, arq_ind);
        
        Promocao p_vazia = {0, 0, 0};
        return p_vazia;
    }
    // Cenário 2: Nó interno cheio -> SPLIT DE NÓ INTERNO
    else {
        // Cria buffers temporários maiores (+1 espaço)
        int chaves_temp[MAX_CHAVES + 1];
        long filhos_temp[MAX_FILHOS + 1];

        // Copia tudo do nó atual para os temps
        int k = 0;
        for(k = 0; k < no.nchaves; k++) {
             chaves_temp[k] = no.chaves[k];
             filhos_temp[k] = no.filhos[k];
        }
        filhos_temp[no.nchaves] = no.filhos[no.nchaves]; // Último filho

        // Insere a nova chave/filho no lugar correto dos temps
        // Empurra para a direita a partir de 'i'
        for (int k = no.nchaves; k > i; k--) {
            chaves_temp[k] = chaves_temp[k-1];
            filhos_temp[k+1] = filhos_temp[k];
        }
        chaves_temp[i] = retorno_filho.chave_promovida;
        filhos_temp[i+1] = retorno_filho.id_novo_filho;

        // Define o ponto de corte (Split)
        // Em nós internos, a chave mediana sobe e não fica em nenhum dos dois nós
        int meio = (MAX_CHAVES + 1) / 2; 
        int chave_que_sobe = chaves_temp[meio];

        // Cria o Novo Nó (Irmão da direita)
        NoIndice novo_no;
        novo_no.folha = no.folha; // Ambos são folhas internas, pois são irmãos
        novo_no.nchaves = 0;

        // Redistribui os dados
        // Nó Atual (Esquerda): Fica com o que está antes do meio
        no.nchaves = meio;
        for(int k=0; k<meio; k++) {
            no.chaves[k] = chaves_temp[k];
            no.filhos[k] = filhos_temp[k];
        }
        no.filhos[meio] = filhos_temp[meio]; // Último ponteiro da esquerda

        // Novo Nó (Direita): Fica com o que está depois do meio
        // Começa do meio + 1 porque a chave do meio subiu!
        int total_elementos = MAX_CHAVES + 1;
        for(int k = meio + 1; k < total_elementos; k++) {
            novo_no.chaves[novo_no.nchaves] = chaves_temp[k];
            novo_no.filhos[novo_no.nchaves] = filhos_temp[k]; // Ponteiro à esquerda da chave
            novo_no.nchaves++;
        }
        novo_no.filhos[novo_no.nchaves] = filhos_temp[total_elementos]; // Último ponteiro da direita

        // Gravação em Disco
        // Grava o novo nó no final do arquivo
        fseek(arq_ind, 0, SEEK_END);
        long offset_novo = ftell(arq_ind);
        fwrite(&novo_no, sizeof(NoIndice), 1, arq_ind);

        // Atualiza o nó atual (velho) no mesmo lugar
        fseek(arq_ind, offset_atual, SEEK_SET);
        fwrite(&no, sizeof(NoIndice), 1, arq_ind);

        // Retorna a Promoção para o pai (ou raiz)
        Promocao p_cima;
        p_cima.chave_promovida = chave_que_sobe;
        p_cima.id_novo_filho = (int)offset_novo; // Casting para int, pois a struct usa int
        p_cima.tem_promo = 1;
        
        return p_cima;
    }
}

// Cria um novo arquivo de folha e retorna o ID dele
int criar_nova_folha_arquivo(NoFolha *folha) {
    // Lê o Superbloco para pegar o ID disponível
    FILE *arq_ind = fopen("indice.bin", "rb+");
    Superbloco sp;
    fread(&sp, sizeof(Superbloco), 1, arq_ind);
    
    int id = sp.contador_folhas; // Pega o ID atual
    
    // Atualiza o Superbloco para o próximo
    sp.contador_folhas++;
    fseek(arq_ind, 0, SEEK_SET);
    fwrite(&sp, sizeof(Superbloco), 1, arq_ind);
    fclose(arq_ind);

    // Cria o arquivo físico
    char nome[50];
    sprintf(nome, "dados/folha_%d.bin", id); // Guardando em uma pasta 'dados' para ficar organizado
    
    FILE *f_folha = fopen(nome, "wb");
    fwrite(folha, sizeof(NoFolha), 1, f_folha); // Grava a folha inicial
    fclose(f_folha);
    
    return id;
}

// Salva uma folha existente (sobrescreve)
void salvar_folha(int id, NoFolha *folha) {
    char nome[50];
    sprintf(nome, "dados/folha_%d.bin", id);
    FILE *arq = fopen(nome, "rb+"); // rb+ para atualizar
    fwrite(folha, sizeof(NoFolha), 1, arq);
    fclose(arq);
}

Promocao inserir_na_folha(int id_folha, RegistroDados *novo_dado) {
    Promocao promo = {0, -1, 0}; // Inicializa sem promoção

    // Carregar a folha do disco
    char nome[50];
    sprintf(nome, "dados/folha_%d.bin", id_folha);
    FILE *arq_f = fopen(nome, "rb");
    NoFolha folha;
    fread(&folha, sizeof(NoFolha), 1, arq_f);
    fclose(arq_f);

    // Verificar se cabe (Simples Inserção)
    if (folha.nchaves < MAX_CHAVES) {
        // Lógica de Insert Sort (Empurra os maiores para frente)
        int i = folha.nchaves - 1;
        while (i >= 0 && folha.dados[i].ano > novo_dado->ano) {
            folha.dados[i+1] = folha.dados[i];
            i--;
        }
        folha.dados[i+1] = *novo_dado;
        folha.nchaves++;
        
        salvar_folha(id_folha, &folha); // Salva e retorna vazio
        return promo;
    }

    // Não cabe: split folha
    
    // Cria um buffer temporário maior para ordenar tudo
    RegistroDados temp[MAX_CHAVES + 1];
    
    // Copia tudo para o temp ordenado
    int i = 0, j = 0;
    while (i < MAX_CHAVES && folha.dados[i].ano < novo_dado->ano) {
        temp[j++] = folha.dados[i++];
    }
    temp[j++] = *novo_dado; // Insere o novo
    while (i < MAX_CHAVES) {
        temp[j++] = folha.dados[i++];
    }

    // Aqui é feita a divisão
    // Corta na metade
    int meio = (MAX_CHAVES + 1) / 2;

    // Folha Nova (Irmã da direita)
    NoFolha nova_folha;
    nova_folha.nchaves = 0;
    nova_folha.id_prox_folha = folha.id_prox_folha; // A nova aponta para quem a velha apontava

    // Copia a segunda metade para a nova folha
    for (int k = meio; k < MAX_CHAVES + 1; k++) {
        nova_folha.dados[nova_folha.nchaves] = temp[k];
        nova_folha.nchaves++;
    }

    // Atualiza a Folha Velha (Esquerda)
    folha.nchaves = meio;
    for (int k = 0; k < meio; k++) {
        folha.dados[k] = temp[k];
    }

    // Salvar no Disco
    // Cria o arquivo novo fisicamente e pega o ID
    int id_nova = criar_nova_folha_arquivo(&nova_folha);
    
    // Linka a velha na nova
    folha.id_prox_folha = id_nova;
    salvar_folha(id_folha, &folha);

    // Retornar a Promoção para o Pai
    // A chave sobe ("copia"), mas o dado fica na folha nova (que é onde começa o 2º bloco)
    promo.chave_promovida = nova_folha.dados[0].ano; 
    promo.id_novo_filho = id_nova;
    promo.tem_promo = 1;

    return promo;
}
