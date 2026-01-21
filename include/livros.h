#ifndef LIVROS_H
#define LIVROS_H

typedef struct {
    int codigo;
    char titulo[100];
    char autor[80];
    char editora[60];
    int anoDepub;
    int quantidade;
    int emprestados;
    char status;
} CadastroLivro;

// funções de livros
void CadastrarLivro(CadastroLivro *l);
void pesquisarLivro(CadastroLivro livros[], int total);
void exibirLivro(CadastroLivro l);

#endif
