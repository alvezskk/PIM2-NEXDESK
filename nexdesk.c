/*
 * NexDesk
 * Como guardamos os dados:
 *  - Cada informacao fica em um vetor. A posicao i de todos os vetores de um
 *    grupo (ex.: ch_titulo[i], ch_status[i]) forma UM registro.
 *  - Textos ficam em matrizes de caracteres: cada linha da matriz e um texto.
 *  - O numero (ID) de cada registro e a posicao + 1. Nada e apagado,
 *    entao o ID nunca muda.
 */
#include <stdio.h>

#define MAX_CLI 50
#define MAX_USU 20
#define MAX_CH 100
#define MAX_OBS 200
#define MAX_HIST 500
#define TAM 100 /* tamanho maximo de um texto (99 letras + fim) */
#define ARQUIVO "nexdesk.txt"
/* ---------- DADOS ---------- */
int total_cli = 0, total_usu = 0, total_ch = 0, total_obs = 0, total_hist = 0;
/* RF01: clientes e usuarios */
char cli_nome[MAX_CLI][TAM];
char usu_nome[MAX_USU][TAM];
int usu_perfil[MAX_USU]; /* 1 = Atendente, 2 = Tecnico */
/* RF02, RF03, RF04, RF05: chamados */
int ch_cliente[MAX_CH];    /* ID do cliente dono do chamado */
char ch_titulo[MAX_CH][TAM];
char ch_desc[MAX_CH][TAM];
int ch_categoria[MAX_CH];  /* 1 a 5 */
int ch_prioridade[MAX_CH]; /* 1 a 3 */
int ch_tecnico[MAX_CH];    /* ID do usuario tecnico; 0 = sem tecnico */
int ch_status[MAX_CH];     /* 0 Aberto, 1 Em andamento, 2 Concluido */
/* RF06: observacoes */
int obs_chamado[MAX_OBS];
char obs_texto[MAX_OBS][TAM];
/* RF08: historico. tipo 1=abertura, 2=tecnico, 3=status */
int hist_chamado[MAX_HIST];
int hist_tipo[MAX_HIST];
int hist_de[MAX_HIST];   /* valor antigo */
int hist_para[MAX_HIST]; /* valor novo */
/* Tira o \n do fim. Se a linha era maior que o vetor, descarta o resto. */
void limpar_fim(char t[])
{
    int i = 0, c;
    while (t[i] != '\0' && t[i] != '\n') i++;
    if (t[i] == '\n') {
        t[i] = '\0';
    } else {
        while ((c = fgetc(stdin)) != '\n' && c != EOF) {
        }
    }
}

char minuscula(char c)
{
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

/* Retorna 1 se "busca" aparece dentro de "texto" (sem diferenciar maiuscula). */
int contem(char texto[], char busca[])
{
    int i, j;
    for (i = 0; texto[i] != '\0'; i++) {
        j = 0;
        while (busca[j] != '\0' && minuscula(texto[i + j]) == minuscula(busca[j])) j++;
        if (busca[j] == '\0') return 1;
    }
    return 0;
}

/* Pede um texto (nao aceita vazio). Retorna 1 se leu, 0 se acabou a entrada. */
int ler_texto(char msg[], char dest[])
{
    while (1) {
        printf("%s", msg);
        if (fgets(dest, TAM, stdin) == NULL) return 0;
        limpar_fim(dest);
        if (dest[0] != '\0') return 1;
        printf("Campo vazio. Tente de novo.\n");
    }
}

/* Converte um texto em numero, letra por letra: "123" vira 1*100 + 2*10 + 3.
 * Aceita so digitos (espacos antes e depois sao ignorados) e ate 9 digitos.
 * Retorna 1 se deu certo; o resultado volta pelo PONTEIRO valor. */
int texto_para_inteiro(char t[], int *valor)
{
    int i = 0, n = 0, digitos = 0;
    while (t[i] == ' ') i++;
    while (t[i] >= '0' && t[i] <= '9') {
        digitos++;
        if (digitos > 9) return 0; /* evita numero grande demais */
        n = n * 10 + (t[i] - '0');
        i++;
    }
    while (t[i] == ' ') i++;
    if (digitos == 0 || t[i] != '\0') return 0; /* nao era so numero */
    *valor = n;
    return 1;
}

/* Pede um numero inteiro e guarda em *valor (PONTEIRO).
 * Retorna 1 = ok, 0 = digitou algo invalido, -1 = acabou a entrada. */
int ler_numero(char msg[], int *valor)
{
    char linha[30];
    printf("%s", msg);
    if (fgets(linha, 30, stdin) == NULL) return -1;
    limpar_fim(linha);
    return texto_para_inteiro(linha, valor);
}

/* Pede um numero entre minimo e maximo. Retorna 1 se valido, 0 se nao. */
int ler_faixa(char msg[], int minimo, int maximo, int *valor)
{
    if (ler_numero(msg, valor) != 1 || *valor < minimo || *valor > maximo) {
        printf("Valor invalido.\n");
        return 0;
    }
    return 1;
}

void imprimir_status(int s)
{
    switch (s) {
    case 0: printf("Aberto"); break;
    case 1: printf("Em andamento"); break;
    default: printf("Concluido"); break;
    }
}

void imprimir_categoria(int c)
{
    switch (c) {
    case 1: printf("Hardware"); break;
    case 2: printf("Software"); break;
    case 3: printf("Rede"); break;
    case 4: printf("Impressora"); break;
    default: printf("Outros"); break;
    }
}

void imprimir_prioridade(int p)
{
    switch (p) {
    case 1: printf("Baixa"); break;
    case 2: printf("Media"); break;
    default: printf("Alta"); break;
    }
}

void imprimir_tecnico(int id)
{
    if (id == 0) printf("(sem tecnico)");
    else printf("%s", usu_nome[id - 1]);
}

void listar_clientes(void)
{
    int i;
    for (i = 0; i < total_cli; i++) printf("  %d - %s\n", i + 1, cli_nome[i]);
}

/* Lista so os usuarios tecnicos. Retorna quantos existem. */
int listar_tecnicos(void)
{
    int i, qtd = 0;
    for (i = 0; i < total_usu; i++) {
        if (usu_perfil[i] == 2) {
            printf("  %d - %s\n", i + 1, usu_nome[i]);
            qtd++;
        }
    }
    return qtd;
}

/* Uma linha resumida do chamado i (posicao no vetor). */
void linha_chamado(int i)
{
    printf("#%d | %s | ", i + 1, ch_titulo[i]);
    imprimir_status(ch_status[i]);
    printf(" | prioridade ");
    imprimir_prioridade(ch_prioridade[i]);
    printf(" | ");
    imprimir_tecnico(ch_tecnico[i]);
    printf("\n");
}

int avisou_erro_arquivo = 0; /* para avisar so uma vez */
/* Grava tudo no arquivo, um valor por linha. Retorna 1 se gravou. */
int salvar(void)
{
    FILE *f;
    int i;
    f = fopen(ARQUIVO, "w");
    if (f == NULL) {
        if (!avisou_erro_arquivo) {
            printf("AVISO: nao foi possivel gravar %s. O programa continua, mas os dados\n"
                   "serao perdidos ao fechar.\n", ARQUIVO);
            avisou_erro_arquivo = 1;
        }
        return 0;
    }
    fprintf(f, "%d\n%d\n%d\n%d\n%d\n", total_cli, total_usu, total_ch, total_obs, total_hist);
    for (i = 0; i < total_cli; i++) fprintf(f, "%s\n", cli_nome[i]);
    for (i = 0; i < total_usu; i++) fprintf(f, "%d\n%s\n", usu_perfil[i], usu_nome[i]);
    for (i = 0; i < total_ch; i++) {
        fprintf(f, "%d\n%d\n%d\n%d\n%d\n", ch_cliente[i], ch_categoria[i], ch_prioridade[i],
                ch_tecnico[i], ch_status[i]);
        fprintf(f, "%s\n%s\n", ch_titulo[i], ch_desc[i]);
    }
    for (i = 0; i < total_obs; i++) fprintf(f, "%d\n%s\n", obs_chamado[i], obs_texto[i]);
    for (i = 0; i < total_hist; i++)
        fprintf(f, "%d\n%d\n%d\n%d\n", hist_chamado[i], hist_tipo[i], hist_de[i], hist_para[i]);
    if (fclose(f) != 0) return 0;
    return 1;
}

/* Le uma linha do arquivo para dest (tamanho tam), sem o \n. Retorna 1 se leu. */
int ler_linha(FILE *f, char dest[], int tam)
{
    int i = 0;
    if (fgets(dest, tam, f) == NULL) return 0;
    while (dest[i] != '\0' && dest[i] != '\n') i++;
    dest[i] = '\0';
    return 1;
}

/* Le uma linha do arquivo e converte em numero. Retorna 1 se deu certo. */
int ler_inteiro_arquivo(FILE *f, int *valor)
{
    char linha[30];
    if (!ler_linha(f, linha, 30)) return 0;
    return texto_para_inteiro(linha, valor);
}

/* Le todo o arquivo ja aberto. Retorna 1 se estiver tudo certo, 0 se estiver
 * corrompido (faltando linhas ou numeros fora do esperado). */
int ler_tudo(FILE *f)
{
    char linha[30];
    int i;
    if (!ler_linha(f, linha, 30)) return 1; /* arquivo vazio = comeco do zero */
    if (!texto_para_inteiro(linha, &total_cli)) return 0;
    if (!ler_inteiro_arquivo(f, &total_usu) || !ler_inteiro_arquivo(f, &total_ch) ||
        !ler_inteiro_arquivo(f, &total_obs) || !ler_inteiro_arquivo(f, &total_hist)) return 0;
    if (total_cli > MAX_CLI || total_usu > MAX_USU || total_ch > MAX_CH ||
        total_obs > MAX_OBS || total_hist > MAX_HIST) return 0;
    for (i = 0; i < total_cli; i++) {
        if (!ler_linha(f, cli_nome[i], TAM)) return 0;
    }
    for (i = 0; i < total_usu; i++) {
        if (!ler_inteiro_arquivo(f, &usu_perfil[i])) return 0;
        if (usu_perfil[i] < 1 || usu_perfil[i] > 2) return 0;
        if (!ler_linha(f, usu_nome[i], TAM)) return 0;
    }
    for (i = 0; i < total_ch; i++) {
        if (!ler_inteiro_arquivo(f, &ch_cliente[i]) || !ler_inteiro_arquivo(f, &ch_categoria[i]) ||
            !ler_inteiro_arquivo(f, &ch_prioridade[i]) || !ler_inteiro_arquivo(f, &ch_tecnico[i]) ||
            !ler_inteiro_arquivo(f, &ch_status[i])) return 0;
        if (ch_cliente[i] < 1 || ch_cliente[i] > total_cli) return 0;
        if (ch_categoria[i] < 1 || ch_categoria[i] > 5) return 0;
        if (ch_prioridade[i] < 1 || ch_prioridade[i] > 3) return 0;
        if (ch_tecnico[i] > total_usu) return 0;
        if (ch_tecnico[i] > 0 && usu_perfil[ch_tecnico[i] - 1] != 2) return 0;
        if (ch_status[i] > 2) return 0;
        if (!ler_linha(f, ch_titulo[i], TAM) || !ler_linha(f, ch_desc[i], TAM)) return 0;
    }
    for (i = 0; i < total_obs; i++) {
        if (!ler_inteiro_arquivo(f, &obs_chamado[i])) return 0;
        if (obs_chamado[i] < 1 || obs_chamado[i] > total_ch) return 0;
        if (!ler_linha(f, obs_texto[i], TAM)) return 0;
    }
    for (i = 0; i < total_hist; i++) {
        if (!ler_inteiro_arquivo(f, &hist_chamado[i]) || !ler_inteiro_arquivo(f, &hist_tipo[i]) ||
            !ler_inteiro_arquivo(f, &hist_de[i]) || !ler_inteiro_arquivo(f, &hist_para[i])) return 0;
        if (hist_chamado[i] < 1 || hist_chamado[i] > total_ch) return 0;
        if (hist_tipo[i] < 1 || hist_tipo[i] > 3) return 0;
        if (hist_tipo[i] == 2 && (hist_de[i] > total_usu || hist_para[i] < 1 ||
                                  hist_para[i] > total_usu)) return 0;
        if (hist_tipo[i] == 3 && (hist_de[i] > 2 || hist_para[i] > 2)) return 0;
    }
    return 1;
}

/* Retorna 1 se carregou (ou se nao havia arquivo), 0 se o arquivo e invalido. */
int carregar(void)
{
    FILE *f = fopen(ARQUIVO, "r");
    int ok;
    if (f == NULL) return 1; /* primeira execucao */
    ok = ler_tudo(f);
    fclose(f);
    return ok;
}

void registrar_historico(int chamado, int tipo, int de, int para)
{
    hist_chamado[total_hist] = chamado;
    hist_tipo[total_hist] = tipo;
    hist_de[total_hist] = de;
    hist_para[total_hist] = para;
    total_hist++;
}

/* RF01 */
void cadastrar_cliente(void)
{
    if (total_cli >= MAX_CLI) {
        printf("Limite de clientes atingido.\n");
        return;
    }
    if (!ler_texto("Nome do cliente: ", cli_nome[total_cli])) return;
    total_cli++;
    salvar();
    printf("Cliente %d cadastrado.\n", total_cli);
}

/* RF01 */
void cadastrar_usuario(void)
{
    int perfil;
    if (total_usu >= MAX_USU) {
        printf("Limite de usuarios atingido.\n");
        return;
    }
    if (!ler_texto("Nome do usuario: ", usu_nome[total_usu])) return;
    if (!ler_faixa("Perfil (1=Atendente, 2=Tecnico): ", 1, 2, &perfil)) return;
    usu_perfil[total_usu] = perfil;
    total_usu++;
    salvar();
    printf("Usuario %d cadastrado.\n", total_usu);
}

/* RF02, RF03 */
void abrir_chamado(void)
{
    int cli, cat, prio, i;
    if (total_ch >= MAX_CH || total_hist >= MAX_HIST) {
        printf("Limite de chamados (ou de historico) atingido.\n");
        return;
    }
    if (total_cli == 0) {
        printf("Cadastre um cliente primeiro.\n");
        return;
    }
    printf("Clientes:\n");
    listar_clientes();
    if (!ler_faixa("Numero do cliente: ", 1, total_cli, &cli)) return;
    i = total_ch; /* posicao do novo chamado */
    if (!ler_texto("Titulo: ", ch_titulo[i])) return;
    if (!ler_texto("Descricao: ", ch_desc[i])) return;
    printf("Categorias: 1=Hardware 2=Software 3=Rede 4=Impressora 5=Outros\n");
    if (!ler_faixa("Categoria: ", 1, 5, &cat)) return;
    printf("Prioridades: 1=Baixa 2=Media 3=Alta\n");
    if (!ler_faixa("Prioridade: ", 1, 3, &prio)) return;
    ch_cliente[i] = cli;
    ch_categoria[i] = cat;
    ch_prioridade[i] = prio;
    ch_tecnico[i] = 0;
    ch_status[i] = 0;
    total_ch++;
    registrar_historico(total_ch, 1, 0, 0);
    salvar();
    printf("Chamado %d aberto.\n", total_ch);
}

/* RF09 (lista de acompanhamento) */
void listar_chamados(void)
{
    int i;
    for (i = 0; i < total_ch; i++) linha_chamado(i);
}

/* Mostra todos os dados do chamado i (consulta por numero). */
void detalhar_chamado(int i)
{
    printf("Chamado #%d\n", i + 1);
    printf("Cliente: %s\n", cli_nome[ch_cliente[i] - 1]);
    printf("Titulo: %s\n", ch_titulo[i]);
    printf("Descricao: %s\n", ch_desc[i]);
    printf("Categoria: ");
    imprimir_categoria(ch_categoria[i]);
    printf("\nPrioridade: ");
    imprimir_prioridade(ch_prioridade[i]);
    printf("\nTecnico: ");
    imprimir_tecnico(ch_tecnico[i]);
    printf("\nStatus: ");
    imprimir_status(ch_status[i]);
    printf("\n");
}

/* RF07 */
void pesquisar_chamados(void)
{
    int opcao, numero, status, i, achou = 0;
    char palavra[TAM];
    printf("1 - Por numero\n2 - Por status\n3 - Por palavra no titulo ou descricao\n");
    if (!ler_faixa("Opcao: ", 1, 3, &opcao)) return;
    if (opcao == 1) {
        if (!ler_faixa("Numero do chamado: ", 1, total_ch, &numero)) return;
        detalhar_chamado(numero - 1);
    } else if (opcao == 2) {
        if (!ler_faixa("Status (0=Aberto, 1=Em andamento, 2=Concluido): ", 0, 2, &status)) return;
        for (i = 0; i < total_ch; i++) {
            if (ch_status[i] == status) {
                linha_chamado(i);
                achou = 1;
            }
        }
        if (!achou) printf("Nenhum chamado encontrado.\n");
    } else {
        if (!ler_texto("Palavra: ", palavra)) return;
        for (i = 0; i < total_ch; i++) {
            if (contem(ch_titulo[i], palavra) || contem(ch_desc[i], palavra)) {
                linha_chamado(i);
                achou = 1;
            }
        }
        if (!achou) printf("Nenhum chamado encontrado.\n");
    }
}

/* RF04 */
void atribuir_tecnico(void)
{
    int numero, tec, i;
    if (total_hist >= MAX_HIST) {
        printf("Limite de historico atingido.\n");
        return;
    }
    if (!ler_faixa("Numero do chamado: ", 1, total_ch, &numero)) return;
    i = numero - 1;
    printf("Tecnicos:\n");
    if (listar_tecnicos() == 0) {
        printf("Nenhum tecnico cadastrado.\n");
        return;
    }
    if (!ler_faixa("Numero do tecnico: ", 1, total_usu, &tec)) return;
    if (usu_perfil[tec - 1] != 2) {
        printf("Esse usuario nao e tecnico.\n");
        return;
    }
    registrar_historico(numero, 2, ch_tecnico[i], tec);
    ch_tecnico[i] = tec;
    salvar();
    printf("Tecnico atribuido ao chamado %d.\n", numero);
}

/* RF05 */
void atualizar_status(void)
{
    int numero, novo, i;
    if (total_hist >= MAX_HIST) {
        printf("Limite de historico atingido.\n");
        return;
    }
    if (!ler_faixa("Numero do chamado: ", 1, total_ch, &numero)) return;
    i = numero - 1;
    printf("Status atual: ");
    imprimir_status(ch_status[i]);
    printf("\n");
    if (!ler_faixa("Novo status (0=Aberto, 1=Em andamento, 2=Concluido): ", 0, 2, &novo)) return;
    if (novo == ch_status[i]) {
        printf("O chamado ja esta com esse status.\n");
        return;
    }
    registrar_historico(numero, 3, ch_status[i], novo);
    ch_status[i] = novo;
    salvar();
    printf("Status do chamado %d atualizado.\n", numero);
}

/* RF06 */
void registrar_observacao(void)
{
    int numero;
    if (total_obs >= MAX_OBS) {
        printf("Limite de observacoes atingido.\n");
        return;
    }
    if (!ler_faixa("Numero do chamado: ", 1, total_ch, &numero)) return;
    if (!ler_texto("Observacao: ", obs_texto[total_obs])) return;
    obs_chamado[total_obs] = numero;
    total_obs++;
    salvar();
    printf("Observacao registrada.\n");
}

/* RF08 + RF06 (mostra historico e observacoes de um chamado) */
void ver_historico(void)
{
    int numero, i;
    if (!ler_faixa("Numero do chamado: ", 1, total_ch, &numero)) return;
    printf("--- Historico do chamado %d ---\n", numero);
    for (i = 0; i < total_hist; i++) {
        if (hist_chamado[i] != numero) continue;
        if (hist_tipo[i] == 1) {
            printf("Chamado aberto\n");
        } else if (hist_tipo[i] == 2) {
            printf("Tecnico: ");
            imprimir_tecnico(hist_de[i]);
            printf(" -> ");
            imprimir_tecnico(hist_para[i]);
            printf("\n");
        } else {
            printf("Status: ");
            imprimir_status(hist_de[i]);
            printf(" -> ");
            imprimir_status(hist_para[i]);
            printf("\n");
        }
    }
    printf("--- Observacoes ---\n");
    for (i = 0; i < total_obs; i++) {
        if (obs_chamado[i] == numero) printf("- %s\n", obs_texto[i]);
    }
}

/* Preenche a matriz m[status][prioridade] com a quantidade de chamados.
 * A matriz e o ponteiro sem_tecnico voltam preenchidos para quem chamou. */
void montar_matriz(int m[3][3], int *sem_tecnico)
{
    int i, s, p;
    for (s = 0; s < 3; s++) {
        for (p = 0; p < 3; p++) m[s][p] = 0;
    }
    *sem_tecnico = 0;
    for (i = 0; i < total_ch; i++) {
        m[ch_status[i]][ch_prioridade[i] - 1]++;
        if (ch_tecnico[i] == 0) (*sem_tecnico)++;
    }
}

/* RF09 */
void resumo_acompanhamento(void)
{
    char nomes[3][14] = {"Aberto", "Em andamento", "Concluido"};
    int m[3][3], sem_tecnico, s, p, soma;
    montar_matriz(m, &sem_tecnico);
    printf("Total de chamados: %d\n", total_ch);
    for (s = 0; s < 3; s++) {
        soma = 0;
        for (p = 0; p < 3; p++) soma = soma + m[s][p];
        printf("%s: %d\n", nomes[s], soma);
    }
    printf("Sem tecnico: %d\n", sem_tecnico);
    printf("\nStatus x prioridade\n");
    printf("%-14s Baixa Media Alta\n", "");
    for (s = 0; s < 3; s++) {
        printf("%-14s", nomes[s]);
        for (p = 0; p < 3; p++) printf(" %5d", m[s][p]);
        printf("\n");
    }
}

int main(void)
{
    int opcao, r;
    if (!carregar()) {
        printf("O arquivo %s esta invalido. Corrija ou apague o arquivo.\n", ARQUIVO);
        return 1;
    }
    do {
        printf("\n=== NexDesk ===\n");
        printf("1 - Cadastrar cliente\n");
        printf("2 - Cadastrar usuario\n");
        printf("3 - Abrir chamado\n");
        printf("4 - Listar chamados\n");
        printf("5 - Consultar / pesquisar\n");
        printf("6 - Atribuir tecnico\n");
        printf("7 - Atualizar status\n");
        printf("8 - Registrar observacao\n");
        printf("9 - Ver historico e observacoes\n");
        printf("10 - Resumo de acompanhamento\n");
        printf("0 - Sair\n");
        r = ler_numero("Opcao: ", &opcao);
        if (r == -1) break;
        if (r == 0) {
            printf("Opcao invalida.\n");
            continue;
        }
        if (opcao >= 4 && opcao <= 9 && total_ch == 0) {
            printf("Nenhum chamado cadastrado ainda.\n");
            continue;
        }
        switch (opcao) {
        case 1: cadastrar_cliente(); break;
        case 2: cadastrar_usuario(); break;
        case 3: abrir_chamado(); break;
        case 4: listar_chamados(); break;
        case 5: pesquisar_chamados(); break;
        case 6: atribuir_tecnico(); break;
        case 7: atualizar_status(); break;
        case 8: registrar_observacao(); break;
        case 9: ver_historico(); break;
        case 10: resumo_acompanhamento(); break;
        case 0: printf("Ate logo!\n"); break;
        default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);
    return 0;
}
