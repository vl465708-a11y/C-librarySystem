#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Necessário para gerenciar datas e prazos

#include "../include/livros.h"
#include "../include/usuarios.h"
#include "../include/emprestimos.h"



// Definição das constantes de capacidade máxima
#define MAX_LIVROS 200
#define MAX_USU   200
#define MAX_EMP   500

// --- Estruturas de Dados ---

// Estrutura auxiliar para Devolução (Não utilizada no código final, mantida por consistência)
typedef struct {
    int codEmprestimo;
} Devolucao ;


// --- Protótipos das Funções ---

// Funções Auxiliares de Persistência
void gravarTodosEmprestimos(emprestimo emp[], int total);
void gravarTodosLivros(CadastroLivro livros[], int total);
int carregarLivros( CadastroLivro livros[], int max);
int carregarUsuarios(CadastroUsuario usuarios[], int max);
int carregarEmprestimos(emprestimo emp[], int max);
void salvarusuario(CadastroUsuario *u); // Salva o usuário no modo 'append'
void salvaremprestimo(emprestimo *e); // Salva o empréstimo no modo 'append'
int encontrarPosicaoLivro(int codigo, CadastroLivro livros[], int total);
int procurarUsuarioPorMatricula(int matricula, CadastroUsuario usuarios[], int total);
int diasTotais(int dia, int mes, int ano);
void exibirLivro(CadastroLivro l);
void exibirUsuario(CadastroUsuario u);
void exibirEmprestimo(emprestimo e);
void backup();

// Funções Principais do Sistema
void CadastrarLivro(CadastroLivro *l);
void CadastrarUsuario(CadastroUsuario *u);
void Emprestimo(emprestimo *e, CadastroLivro livros[], int qtdLivros);
int pesquisarusuario(CadastroUsuario usuarios[], int max);
void listarEmprestimosAtivos(emprestimo emp[], int total);
void verificarEmprestimo(emprestimo vet[], int total);
void relatorioAtrasos(emprestimo emp[], int qtdEmp, CadastroUsuario usuarios[], int qtdUsuarios);
void renovarEmprestimo(emprestimo emp[], int qtdEmp);
void buscaAvancadaLivros(CadastroLivro livros[], int total);
void relatorioLivrosMaisEmprestados(CadastroLivro livros[], int total);


// --- Implementações das Funções Auxiliares ---

// Converte data para dias corridos (base 30 dias/mês, 365 dias/ano) para cálculos de atraso.
int diasTotais(int dia, int mes, int ano){
    return dia + (mes * 30) + (ano * 365);
}

// Cria cópias de segurança dos arquivos de dados
void backup() {
    // Comando 'copy' é específico do Windows. Em Linux/macOS seria 'cp'.
    system("copy livros.txt livros.bak >nul");
    system("copy usuarios.txt usuarios.bak >nul");
    system("copy emprestimos.txt emprestimos.bak >nul");
    printf("\nBackup realizado com sucesso!\n");
}

// Exibe os dados de um único livro formatados
void exibirLivro(CadastroLivro l) {
    printf("\n--- Detalhes do Livro ---\n");
    printf("Código: %d\n", l.codigo);
    printf("Título: %s\n", l.titulo);
    printf("Autor: %s\n", l.autor);
    printf("Editora: %s\n", l.editora);
    printf("Ano de Publicação: %d\n", l.anoDepub);
    printf("Exemplares Totais: %d\n", l.quantidade);
    printf("Exemplares Emprestados: %d\n", l.emprestados);
    printf("Status: %c (%s)\n", l.status,
           (l.status == 'D' ? "Disponível" : (l.status == 'E' ? "Emprestado (Esgotado)" : "Indefinido")));
    printf("-------------------------\n");
}

// Retorna o índice (posição) do livro no array pelo seu código, ou -1 se não encontrar
int encontrarPosicaoLivro(int codigo, CadastroLivro livros[], int total){
    for(int i = 0; i < total; i++){
        if(livros[i].codigo == codigo){
            return i;
        }
    }
    return -1;
}

// Reescreve TODO o arquivo de livros.txt (usado após cadastro/empréstimo/devolução)
void gravarTodosLivros(CadastroLivro livros[], int total) {
    FILE *arq = fopen("livros.txt", "w");
    if (!arq) {
        printf("Erro ao reescrever livros.txt!\n");
        return;
    }

    for (int i = 0; i < total; i++) {
        fprintf(arq, "%d;%s;%s;%s;%d;%d;%d;%c\n",
            livros[i].codigo,
            livros[i].titulo,
            livros[i].autor,
            livros[i].editora,
            livros[i].anoDepub,
            livros[i].quantidade,
            livros[i].emprestados,
            livros[i].status
        );
    }
    fclose(arq);
}

// Carrega os livros do arquivo para a memória
int carregarLivros( CadastroLivro livros[], int max) {
    FILE *arq = fopen("livros.txt", "r");
    if(!arq) return 0;

    int count = 0;
    while(count < max &&
          fscanf(arq, "%d;%[^;];%[^;];%[^;];%d;%d;%d;%c\n",
                 &livros[count].codigo,
                 livros[count].titulo,
                 livros[count].autor,
                 livros[count].editora,
                 &livros[count].anoDepub,
                 &livros[count].quantidade,
                 &livros[count].emprestados,
                 &livros[count].status) != EOF) {
        count++;
                 }

    fclose(arq);
    return count;
}

// Salva um único usuário (faz append no arquivo)
void salvarusuario(CadastroUsuario *u) {
    FILE *arquivo = fopen("usuarios.txt", "a");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }
    fprintf(arquivo, "%d;%s;%s;%s;%d;%d;%d\n",
        u->matricula, u->Nome, u->Curso, u->tel,
        u->diaCad, u->mesCad, u->anoCad
    );
    fclose(arquivo);
}

// Carrega os usuários do arquivo para a memória
int carregarUsuarios(CadastroUsuario usuarios[], int max) {
    FILE *arq = fopen("usuarios.txt", "r");
    if(!arq) return 0;

    int count = 0;
    while(count < max &&
          fscanf(arq, "%d;%[^;];%[^;];%[^;];%d;%d;%d\n",
                 &usuarios[count].matricula,
                 usuarios[count].Nome,
                 usuarios[count].Curso,
                 usuarios[count].tel,
                 &usuarios[count].diaCad,
                 &usuarios[count].mesCad,
                 &usuarios[count].anoCad) != EOF) {
        count++;
                 }

    fclose(arq);
    return count;
}

// Exibe os dados de um único usuário formatados
void exibirUsuario(CadastroUsuario u) {
    printf("\n--- Detalhes do Usuário ---\n");
    printf("Matrícula: %d\n", u.matricula);
    printf("Nome: %s\n", u.Nome);
    printf("Curso: %s\n", u.Curso);
    printf("Telefone: %s\n", u.tel);
    printf("Data de Cadastro: %02d/%02d/%04d\n", u.diaCad, u.mesCad, u.anoCad);
    printf("---------------------------\n");
}

// Retorna o índice (posição) do usuário no array pela matrícula, ou -1 se não encontrar
int procurarUsuarioPorMatricula(int matricula, CadastroUsuario usuarios[], int total) {
    for(int i = 0; i < total; i++){
        if(usuarios[i].matricula == matricula){
            return i; // Retorna o índice
        }
    }
    return -1; // Não encontrado
}

// Carrega os empréstimos do arquivo para a memória
int carregarEmprestimos(emprestimo emp[], int max) {
    FILE *arq = fopen("emprestimos.txt", "r");
    if(!arq) return 0;

    int count = 0;
    while(count < max &&
          fscanf(arq, "%d;%d;%d;%d;%d;%d;%d;%d;%d;%c\n",
                 &emp[count].codEmprestimo,
                 &emp[count].matriculaUsuario,
                 &emp[count].codigoLivro,
                 &emp[count].dia,
                 &emp[count].mes,
                 &emp[count].ano,
                 &emp[count].diaDevo,
                 &emp[count].mesDevo,
                 &emp[count].anoDevo,
                 &emp[count].status) != EOF) {
        count++;
                 }

    fclose(arq);
    return count;
}

// Salva um único empréstimo (faz append no arquivo)
void salvaremprestimo(emprestimo *e) {
    FILE *arquivo = fopen("emprestimos.txt", "a");
    if (!arquivo)  {
        printf("Erro ao salvar o arquivo!\n");
        return;
    }
    fprintf(arquivo,"%d;%d;%d;%d;%d;%d;%d;%d;%d;%c\n",
        e->codEmprestimo,
        e->matriculaUsuario,
        e->codigoLivro,
        e->dia,
        e->mes,
        e->ano,
        e->diaDevo,
        e->mesDevo,
        e->anoDevo,
        e->status);
    fclose(arquivo);
}

// Reescreve TODO o arquivo de empréstimos.txt (usado após devolução/renovação)
void gravarTodosEmprestimos(emprestimo emp[], int total) {
    FILE *arq = fopen("emprestimos.txt", "w");
    if (!arq) {
        printf("Erro ao reescrever emprestimos.txt!\n");
        return;
    }

    for (int i = 0; i < total; i++) {
        fprintf(arq, "%d;%d;%d;%d;%d;%d;%d;%d;%d;%c\n",
            emp[i].codEmprestimo,
            emp[i].matriculaUsuario,
            emp[i].codigoLivro,
            emp[i].dia,
            emp[i].mes,
            emp[i].ano,
            emp[i].diaDevo,
            emp[i].mesDevo,
            emp[i].anoDevo,
            emp[i].status
        );
    }
    fclose(arq);
}

// Exibe os detalhes de um empréstimo
void exibirEmprestimo(emprestimo e) {
    printf("\n--- Detalhes do Empréstimo ---\n");
    printf("Código do Empréstimo: %d\n", e.codEmprestimo);
    printf("Matrícula do Usuário: %d\n", e.matriculaUsuario);
    printf("Código do Livro: %d\n", e.codigoLivro);
    printf("Data do Empréstimo: %02d/%02d/%04d\n", e.dia, e.mes, e.ano);
    printf("Devolução Prevista: %02d/%02d/%04d\n", e.diaDevo, e.mesDevo, e.anoDevo);
    printf("Status: %c (%s)\n", e.status, (e.status == 'A' ? "ATIVO" : "DEVOLVIDO"));
    printf("------------------------------\n");
}

void verificarPrazo( emprestimo *e) {
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);

    // Calcula os dias totais da data atual
    int hoje = diasTotais(data->tm_mday, data->tm_mon+1, data->tm_year+1900);
    // Calcula os dias totais da data de devolução prevista
    int devolucaoPrevista = diasTotais(e->diaDevo, e->mesDevo, e->anoDevo);

    int faltam = devolucaoPrevista - hoje;

    printf("\n====== Situação do Empréstimo %d ======\n", e->codEmprestimo);
    printf("Pegou em: %02d/%02d/%04d\n", e->dia, e->mes, e->ano);
    printf("Devolver até: %02d/%02d/%04d\n", e->diaDevo, e->mesDevo, e->anoDevo);

    if (e->status == 'D') {
        printf("Status: DEVOLVIDO. Prazo não é mais relevante.\n");
    } else if(faltam > 0){
        printf("Faltam %d dias para devolver.\n", faltam);
    } else if(faltam == 0){
        printf("Hoje é o último dia para devolver!\n");
    } else {
        printf("Atrasado por %d dias!!!\n", faltam * -1);
    }
    printf("====================================\n");
}


// --- Implementações das Funções Principais ---

// Opção 1: Cadastro de Livros
void CadastrarLivro(CadastroLivro *l) {
    printf("\n--- Cadastro de Novo Livro ---\n");
    printf("Codigo (Inteiro): ");
    if (scanf("%d", &l->codigo) != 1) { while(getchar()!='\n'); printf("Entrada invalida. Cancelando.\n"); l->codigo = 0; return; }
    printf ("Titulo: ");
    scanf (" %[^\n]", l->titulo);
    printf ("Autor: ");
    scanf (" %[^\n]", l->autor);
    printf ("Editora: ");
    scanf (" %[^\n]", l->editora);
    printf ("Ano de Publicação: ");
    if (scanf("%d", &l->anoDepub) != 1) { while(getchar()!='\n'); printf("Entrada invalida. Cancelando.\n"); l->codigo = 0; return; }

    printf ("Quantidade de Exemplares: ");
    if (scanf("%d", &l->quantidade) != 1) { while(getchar()!='\n'); printf("Entrada invalida. Cancelando.\n"); l->codigo = 0; return; }

    // Inicializa campos de controle
    l->emprestados = 0;
    l->status = 'D'; // Disponível
}

// Opção 2: Cadastro de Usuário
void CadastrarUsuario(CadastroUsuario *u) {
    printf("\n--- Cadastro de Novo Usuário ---\n");
    printf("Matricula (Inteiro): ");
    if (scanf("%d", &u->matricula) != 1) { while(getchar()!='\n'); printf("Entrada invalida. Cancelando.\n"); u->matricula = 0; return; }
    printf("Nome: ");
    scanf(" %[^\n]", u->Nome);
    printf("Curso: ");
    scanf(" %[^\n]", u->Curso);
    printf("Telefone: ");
    scanf(" %[^\n]", u->tel);
    printf("Data de Cadastro (d/m/a): ");
    if (scanf("%d/%d/%d", &u->diaCad, &u->mesCad, &u->anoCad) != 3) { while(getchar()!='\n'); printf("Entrada de data invalida. Cancelando.\n"); u->matricula = 0; return; }
}

// Opção 3: Alugar Livro
void Emprestimo(emprestimo *e, CadastroLivro livros[], int qtdLivros) {

    printf("\n--- Registro de Empréstimo ---\n");
    printf("Codigo do Empréstimo (Inteiro): ");
    if (scanf("%d", &e->codEmprestimo) != 1) {
        while(getchar()!='\n'); printf("Entrada invalida\n"); return;
    }
    printf("Numero da Matrícula do Usuário: ");
    if (scanf("%d", &e->matriculaUsuario) != 1) {
        while(getchar()!='\n'); printf("Entrada invalida\n"); return;
    }
    printf("Codigo do Livro: ");
    if (scanf("%d", &e->codigoLivro) != 1) {
        while(getchar()!='\n'); printf("Entrada invalida\n"); return;
    }

    /* 1. Procura e verifica a disponibilidade do livro */
    int pos = encontrarPosicaoLivro(e->codigoLivro, livros, qtdLivros);
    if (pos == -1) {
        printf("Erro: Livro não encontrado!\n");
        return;
    }
    int disponiveis = livros[pos].quantidade - livros[pos].emprestados;
    if (disponiveis <= 0) {
        printf("Atenção: Nenhum exemplar disponível deste livro (%s).\n", livros[pos].titulo);
        livros[pos].status = 'E';
        gravarTodosLivros(livros, qtdLivros);
        return;
    }

    /* 2. Registra data de empréstimo (hoje) */
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);
    e->dia  = data->tm_mday;
    e->mes  = data->tm_mon + 1;
    e->ano  = data->tm_year + 1900;

    /* 3. Calcula devolução (7 dias após o empréstimo) */
    struct tm dt = *data;
    dt.tm_mday += 7;
    mktime(&dt); // Normaliza a data (ex: 32 de janeiro vira 1 de fevereiro)
    e->diaDevo = dt.tm_mday;
    e->mesDevo = dt.tm_mon + 1;
    e->anoDevo = dt.tm_year + 1900;

    e->status = 'A'; // Status Ativo

    /* 4. Atualiza o livro em memória e no arquivo */
    livros[pos].emprestados++;
    if (livros[pos].emprestados >= livros[pos].quantidade) livros[pos].status = 'E';
    else livros[pos].status = 'D';

    gravarTodosLivros(livros, qtdLivros);
    salvaremprestimo(e); // Adiciona o novo registro de empréstimo no arquivo

    printf("\nEmpréstimo registrado com sucesso.\n");
    printf("Livro: %s\n", livros[pos].titulo);
    printf("Devolução prevista: %02d/%02d/%04d\n", e->diaDevo, e->mesDevo, e->anoDevo);
}

// Opção 4: Verificar prazo de aluguel (código empréstimo)
void verificarEmprestimo(emprestimo vet[], int total) {
    int cod;
    printf("\nDigite o código do empréstimo a ser verificado: ");
    if (scanf("%d", &cod) != 1) { while(getchar()!='\n'); printf("Entrada invalida\n"); return; }

    for (int i = 0; i < total; i++) {
        if (vet[i].codEmprestimo == cod) {
            verificarPrazo(&vet[i]);
            return;
        }
    }

    printf("\nEmpréstimo não encontrado!\n");
}

// Opção 5: Realizar Devolução
void devolverLivro(CadastroLivro livros[], int qtdLivros, emprestimo emp[], int qtdEmp) {
    int codEmprestimo;
    int posEmprestimo = -1;
    int posLivro = -1;

    printf("\n--- Devolução de Livro ---\n");
    printf("Digite o CODIGO do empréstimo a ser devolvido: ");
    if (scanf("%d", &codEmprestimo) != 1) {
        while(getchar()!='\n'); printf("Entrada inválida.\n"); return;
    }

    // 1. Procurar o empréstimo ativo
    for (int i = 0; i < qtdEmp; i++) {
        if (emp[i].codEmprestimo == codEmprestimo && emp[i].status == 'A') {
            posEmprestimo = i;
            break;
        }
    }

    if (posEmprestimo == -1) {
        printf("Empréstimo ativo com código %d não encontrado.\n", codEmprestimo);
        return;
    }

    // 2. Procurar o livro correspondente
    int codigoLivro = emp[posEmprestimo].codigoLivro;
    posLivro = encontrarPosicaoLivro(codigoLivro, livros, qtdLivros);

    if (posLivro == -1) {
        printf("Erro: Livro (cód %d) associado ao empréstimo não encontrado no acervo.\n", codigoLivro);
        return;
    }

    // 3. Atualiza o status do empréstimo para 'D' (Devolvido)
    emp[posEmprestimo].status = 'D';

    // 4. Atualiza a contagem de exemplares emprestados do livro
    if (livros[posLivro].emprestados > 0) {
        livros[posLivro].emprestados--;

        // 5. Atualiza o status do livro no acervo
        if (livros[posLivro].emprestados < livros[posLivro].quantidade) {
            livros[posLivro].status = 'D'; // Volta a ficar Disponível
        }

        // 6. Persistir as alterações
        gravarTodosLivros(livros, qtdLivros);
        gravarTodosEmprestimos(emp, qtdEmp);

        printf("\nDevolução registrada com sucesso!\n");
        printf("   Livro: %s\n", livros[posLivro].titulo);

    } else {
        printf("Atenção: Erro de contagem. Exemplares emprestados já estava em zero. Status de empréstimo corrigido.\n");
        gravarTodosEmprestimos(emp, qtdEmp);
    }
}

// Opção 6: Pesquisar Usuários
int pesquisarusuario(CadastroUsuario usuarios[], int max) {
    int opcao;
    int matriculaBuscada;
    char nomeBuscado[100];
    int encontrado = 0;

    printf ("\n--- Pesquisa de Usuários ---\n");
    printf ("1. Pesquisa por Matrícula\n");
    printf ("2. Pesquisa por Nome\n");
    printf ("Escolha sua opção de busca: ");
    scanf ("%d", &opcao);
    getchar();

    switch (opcao) {
        case 1:
            printf("Digite a Matrícula: ");
            if (scanf("%d", &matriculaBuscada) != 1) { while(getchar()!='\n'); printf("Entrada inválida.\n"); return 0 ; }

            for (int i = 0; i < max; i++) {
                if (usuarios[i].matricula == matriculaBuscada) {
                    exibirUsuario(usuarios[i]);
                    encontrado = 1;
                    break;
                }
            }
            break;

        case 2:
            printf("Digite o Nome completo ou parte dele: ");
            scanf(" %[^\n]", nomeBuscado);

            printf("\nResultados da Busca por Nome:\n");
            for (int i = 0; i < max; i++) {
                if (strstr(usuarios[i].Nome, nomeBuscado) != NULL) {
                    exibirUsuario(usuarios[i]);
                    encontrado = 1;
                }
            }
            break;

        default:
            printf("Opção inválida.\n");
            return 0;
    }

    if (encontrado == 0) {
        printf("\nUsuário não encontrado com o critério fornecido.\n");
    }
    return encontrado;
}

// Opção 7: Pesquisar Livros (por Código, Título ou Autor)
void pesquisarLivro(CadastroLivro livros[], int total) {
    int opcao;
    int codigoBuscado;
    char termoBuscado[100];
    int encontrado = 0;

    printf ("\n--- Pesquisa de Livros ---\n");
    printf ("1. Pesquisa por Código\n");
    printf ("2. Pesquisa por Título\n");
    printf ("3. Pesquisa por Autor\n");
    printf ("Escolha a opção de busca: ");
        if (scanf ("%d", &opcao) != 1 ) {
            while(getchar()!='\n'); printf("Entrada inválida.\n"); return;
        }
    getchar();

    switch (opcao) {
        case 1: // Por Código
            printf ("Digite o código do livro: ");
            if (scanf("%d", &codigoBuscado) != 1) { while(getchar()!='\n'); printf("Entrada de código inválida.\n"); return; }
            for (int i = 0; i < total; i++) {
                if (livros[i].codigo == codigoBuscado) {
                    exibirLivro(livros[i]);
                    encontrado = 1;
                    break;
                }
            }
            break;

        case 2: // Por Título (Busca parcial)
            printf("Digite o Título ou parte dele: ");
            scanf(" %[^\n]", termoBuscado);
            for (int i = 0; i < total; i++) {
                if (strstr(livros[i].titulo, termoBuscado) != NULL) {
                    exibirLivro(livros[i]);
                    encontrado = 1;
                }
            }
            break;

        case 3: // Por Autor (Busca parcial)
            printf("Digite o Nome do Autor ou parte dele: ");
            scanf(" %[^\n]", termoBuscado);
            for (int i = 0; i < total; i++) {
                if (strstr(livros[i].autor, termoBuscado) != NULL) {
                    exibirLivro(livros[i]);
                    encontrado = 1;
                }
            }
            break;

        default:
            printf("Opção de busca inválida.\n");
            return;
    }

    if (encontrado == 0) {
        printf("\nLivro não encontrado com o critério fornecido.\n");
    }
}

// Opção 8: Listar Empréstimos Ativos
void listarEmprestimosAtivos(emprestimo emp[], int total) {
    int ativos = 0;

    printf("\n==================================\n");
    printf("      LISTA DE EMPRÉSTIMOS ATIVOS\n");
    printf("==================================\n");

    for (int i = 0; i < total; i++) {
        if (emp[i].status == 'A') {
            exibirEmprestimo(emp[i]);
            ativos++;
        }
    }

    if (ativos == 0) {
        printf("\nNenhum empréstimo ativo encontrado no momento.\n");
    } else {
        printf("\nTotal de empréstimos ativos: %d\n", ativos);
    }
}

// Opção 9: Relatório de Empréstimos em Atraso
void relatorioAtrasos(emprestimo emp[], int qtdEmp, CadastroUsuario usuarios[], int qtdUsuarios) {
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);
    int hoje = diasTotais(data->tm_mday, data->tm_mon + 1, data->tm_year + 1900);
    int atrasos = 0;

    printf("\n=============================================\n");
    printf("     RELATÓRIO DE EMPRÉSTIMOS EM ATRASO\n");
    printf("=============================================\n");

    for (int i = 0; i < qtdEmp; i++) {
        // Só verifica empréstimos ativos
        if (emp[i].status == 'A') {
            int devolucaoPrevista = diasTotais(emp[i].diaDevo, emp[i].mesDevo, emp[i].anoDevo);

            if (hoje > devolucaoPrevista) {
                int diasAtraso = hoje - devolucaoPrevista;
                int posUsuario = procurarUsuarioPorMatricula(emp[i].matriculaUsuario, usuarios, qtdUsuarios);

                printf("\n=============================================\n");
                printf("CÓDIGO EMPRÉSTIMO: %d (Atrasado por %d dias)\n", emp[i].codEmprestimo, diasAtraso);
                printf("---------------------------------------------\n");

                if (posUsuario != -1) {
                    printf("Usuário: %s (Matrícula: %d)\n", usuarios[posUsuario].Nome, emp[i].matriculaUsuario);
                    printf("Telefone: %s\n", usuarios[posUsuario].tel);
                } else {
                    printf("Usuário não encontrado (Matrícula: %d)\n", emp[i].matriculaUsuario);
                }
                printf("Devolução Prevista: %02d/%02d/%04d\n", emp[i].diaDevo, emp[i].mesDevo, emp[i].anoDevo);
                atrasos++;
            }
        }
    }

    printf("\n=============================================\n");
    if (atrasos == 0) {
        printf(" Nenhum empréstimo ativo está em atraso.\n");
    } else {
        printf("TOTAL DE %d EMPRÉSTIMOS ENCONTRADOS EM ATRASO.\n", atrasos);
    }
    printf("=============================================\n");
}

// Opção 10: Renovar Empréstimo
void renovarEmprestimo(emprestimo emp[], int qtdEmp) {
    int codEmprestimo;
    int posEmprestimo = -1;

    printf("\n==================================\n");
    printf("     RENOVAÇÃO DE EMPRÉSTIMO\n");
    printf("==================================\n");

    printf("Digite o CÓDIGO do empréstimo que deseja renovar: ");
    if (scanf("%d", &codEmprestimo) != 1) {
        while(getchar()!='\n'); printf("Entrada inválida.\n"); return;
    }

    for (int i = 0; i < qtdEmp; i++) {
        if (emp[i].codEmprestimo == codEmprestimo && emp[i].status == 'A') {
            posEmprestimo = i;
            break;
        }
    }

    if (posEmprestimo == -1) {
        printf("Empréstimo ativo com código %d não encontrado ou já devolvido.\n", codEmprestimo);
        return;
    }

    // Calcula a nova data (7 dias a partir da data de devolução atual)
    struct tm dt = {0};
    dt.tm_mday = emp[posEmprestimo].diaDevo;
    dt.tm_mon  = emp[posEmprestimo].mesDevo - 1;
    dt.tm_year = emp[posEmprestimo].anoDevo - 1900;
    dt.tm_isdst = -1;

    time_t tempoAtual = mktime(&dt);
    time_t tempoRenovado = tempoAtual + (7 * 24 * 60 * 60);

    struct tm *dataRenovada = localtime(&tempoRenovado);

    int diaAntigo = emp[posEmprestimo].diaDevo;
    int mesAntigo = emp[posEmprestimo].mesDevo;
    int anoAntigo = emp[posEmprestimo].anoDevo;

    // Atualiza com a nova data
    emp[posEmprestimo].diaDevo = dataRenovada->tm_mday;
    emp[posEmprestimo].mesDevo = dataRenovada->tm_mon + 1;
    emp[posEmprestimo].anoDevo = dataRenovada->tm_year + 1900;

    gravarTodosEmprestimos(emp, qtdEmp);

    printf("\nEmpréstimo %d renovado com sucesso!\n", codEmprestimo);
    printf("   Data de Devolução Antiga: %02d/%02d/%04d\n", diaAntigo, mesAntigo, anoAntigo);
    printf("   Nova Data de Devolução:   %02d/%02d/%04d\n", emp[posEmprestimo].diaDevo, emp[posEmprestimo].mesDevo, emp[posEmprestimo].anoDevo);
    printf("==================================\n");
}

// Opção 11: Busca Avançada de Livros
void buscaAvancadaLivros(CadastroLivro livros[], int total) {
    char termoTitulo[100] = "";
    char termoAutor[80] = "";
    int anoMin = 0;
    char statusBuscado = '\0';
    int encontrados = 0;

    printf("\n==================================\n");
    printf("     🔬 BUSCA AVANÇADA DE LIVROS\n");
    printf("==================================\n");
    printf("Deixe o campo vazio (ou 0 para ano) para ignorar o critério.\n\n");

    printf("1. Termo no Título: ");
    scanf(" %[^\n]", termoTitulo);

    printf("2. Termo no Autor: ");
    scanf(" %[^\n]", termoAutor);

    printf("3. Publicados a partir do Ano (ex: 2020): ");
    scanf("%d", &anoMin);

    printf("4. Status do Livro (D/E, ou enter para ignorar): ");
    while(getchar() != '\n');
    statusBuscado = getchar();
    if (statusBuscado == '\n' || statusBuscado == ' ') statusBuscado = '\0';

    printf("\n--- Resultados (Combinando Todos os Critérios) ---\n");

    for (int i = 0; i < total; i++) {
        int passaTitulo = 1;
        int passaAutor = 1;
        int passaAno = 1;
        int passaStatus = 1;

        if (termoTitulo[0] != '\0' && strstr(livros[i].titulo, termoTitulo) == NULL) { passaTitulo = 0; }
        if (termoAutor[0] != '\0' && strstr(livros[i].autor, termoAutor) == NULL) { passaAutor = 0; }
        if (anoMin > 0 && livros[i].anoDepub < anoMin) { passaAno = 0; }
        if (statusBuscado != '\0' && (statusBuscado == 'D' || statusBuscado == 'E')) {
             if (livros[i].status != statusBuscado) { passaStatus = 0; }
        }

        if (passaTitulo && passaAutor && passaAno && passaStatus) {
            exibirLivro(livros[i]);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        printf("\nNenhum livro encontrado que corresponda a todos os critérios.\n");
    } else {
        printf("\nBusca avançada finalizada. Total de %d livros encontrados.\n", encontrados);
    }
}

// Opção 12: Relatório de Livros Mais Emprestados
void relatorioLivrosMaisEmprestados(CadastroLivro livros[], int total) {
    CadastroLivro livrosOrdenados[MAX_LIVROS];
    for (int i = 0; i < total; i++) {
        livrosOrdenados[i] = livros[i];
    }

    // Ordenação (Bubble Sort Decrescente por 'emprestados')
    CadastroLivro temp;
    for (int i = 0; i < total - 1; i++) {
        for (int j = 0; j < total - 1 - i; j++) {
            if (livrosOrdenados[j].emprestados < livrosOrdenados[j + 1].emprestados) {
                temp = livrosOrdenados[j];
                livrosOrdenados[j] = livrosOrdenados[j + 1];
                livrosOrdenados[j + 1] = temp;
            }
        }
    }

    printf("\n=============================================\n");
    printf("      RELATÓRIO DE LIVROS MAIS EMPRESTADOS\n");
    printf("=============================================\n");

    int livrosListados = 0;
    for (int i = 0; i < total; i++) {
        if (livrosOrdenados[i].emprestados > 0) {
            printf("[%dº] %s (Cód: %d)\n", i + 1, livrosOrdenados[i].titulo, livrosOrdenados[i].codigo);
            printf("      Total de Empréstimos: %d\n", livrosOrdenados[i].emprestados);
            printf("---------------------------------------------\n");
            livrosListados++;
        }
    }

    if (livrosListados == 0) {
        printf("Nenhum livro teve empréstimo registrado ainda.\n");
    } else {
        printf("Relatório concluído. %d livros com empréstimos registrados.\n", livrosListados);
    }
}

// --- Main e Gerenciamento de Memória ---

CadastroLivro livros[MAX_LIVROS];
int qtdLivros = 0;

CadastroUsuario usuarios[MAX_USU];
int qtdUsuarios = 0;

emprestimo emp[MAX_EMP];
int qtdEmp = 0;

 int main() {

     // 1. Carrega os dados persistidos em arquivos
     qtdLivros = carregarLivros(livros, MAX_LIVROS);
     qtdEmp    = carregarEmprestimos(emp, MAX_EMP);
     qtdUsuarios = carregarUsuarios(usuarios, MAX_USU);

        int escolha;

         CadastroLivro l = {0}; // Inicializa para garantir que dados antigos não interfiram
         CadastroUsuario u = {0};
         emprestimo e = {0};


    // Loop principal para o menu
    do {
         printf ("\n\n=============== MENU PRINCIPAL (BIBLIOTECA) =============\n");
         printf ("Total em Acervo: %d livros | %d usuários | %d empréstimos\n", qtdLivros, qtdUsuarios, qtdEmp);
         printf ("---------------------------------------------------------\n");
         printf ("| 1. Cadastro de livros            | 8. Listar empréstimos ATIVOS         |\n");
         printf ("| 2. Cadastrar usuário             | 9. Relatório de Atrasos              |\n");
         printf ("| 3. Alugar livro                  | 10. Renovar Empréstimo              |\n");
         printf ("| 4. Verificar prazo (cód. empr.)  | 11. Busca Avançada de Livros        |\n");
         printf ("| 5. Realizar devolução            | 12. Relatório de Livros Mais Populares|\n");
         printf ("| 6. Pesquisar usuários            | 13. SAIR e SALVAR (Backup)          |\n");
         printf ("| 7. Pesquisar livros (cód/título/autor) |\n");
         printf ("---------------------------------------------------------\n");
         printf ("\nDigite sua escolha: ");
         if (scanf ("%d", &escolha) != 1) {
             while(getchar()!='\n');
             printf("\nOpção inválida. Digite um número de 1 a 13.\n");
             escolha = 0; // Para garantir que o loop continue
             continue;
         }

        switch (escolha) {
            case 1:
                CadastrarLivro(&l);
                if (qtdLivros < MAX_LIVROS && l.codigo != 0) {
                    livros[qtdLivros] = l;
                    qtdLivros++;
                    gravarTodosLivros(livros, qtdLivros);
                    printf("\nLivro cadastrado e salvo com sucesso.\n");
                } else if (l.codigo != 0) {
                    printf("\nErro: Limite de livros atingido ou entrada inválida.\n");
                }
            break;

            case 2:
                CadastrarUsuario(&u);
                if (qtdUsuarios < MAX_USU && u.matricula != 0) {
                    salvarusuario(&u); // Adiciona no arquivo
                    usuarios[qtdUsuarios] = u; // Atualiza a lista em memória
                    qtdUsuarios++;
                    printf("\nUsuário cadastrado e salvo com sucesso.\n");
                } else if (u.matricula != 0) {
                     printf("\nErro: Limite de usuários atingido ou entrada inválida.\n");
                }
            break;

            case 3:
                Emprestimo(&e, livros, qtdLivros);
                if (e.status == 'A') {
                    emp[qtdEmp] = e;
                    qtdEmp++;
                }
            break;

            case 4:
                verificarEmprestimo(emp, qtdEmp);
            break;

            case 5:
                devolverLivro(livros, qtdLivros, emp, qtdEmp);
            break;

            case 6:
                pesquisarusuario(usuarios, qtdUsuarios);
            break;

            case 7:
                pesquisarLivro(livros, qtdLivros);
            break;

            case 8:
                listarEmprestimosAtivos(emp, qtdEmp);
            break;

            case 9:
                relatorioAtrasos(emp, qtdEmp, usuarios, qtdUsuarios);
            break;

            case 10:
                renovarEmprestimo(emp, qtdEmp);
            break;
            case 11:
                buscaAvancadaLivros(livros, qtdLivros);
            break;
            case 12:
                relatorioLivrosMaisEmprestados(livros, qtdLivros);
            break;
            case 13: // SAIR
                printf("\nExecutando rotina de salvamento e backup...\n");
                break;

            default:
                if (escolha != 0) printf ("\nOpção inválida! Tente novamente.\n");
        }

    } while (escolha != 13);


     // 2. Rotina de salvamento final e backup
     backup();

     return 0;
 }