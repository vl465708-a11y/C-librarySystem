#ifndef EMPRESTIMOS_H
#define EMPRESTIMOS_H

#include "livros.h"

// struct de empréstimo
typedef struct {
    int codEmprestimo;
    int matriculaUsuario;
    int codigoLivro;
    int dia, mes, ano;
    int diaDevo, mesDevo, anoDevo;
    char status;
} emprestimo;

// funções de empréstimos
void realizarEmprestimo(emprestimo *e);
void devolverLivro(CadastroLivro livros[], int qtdLivros, emprestimo emp[], int qtdEmp);
void listarEmprestimos(emprestimo emp[], int total);
void exibirEmprestimo(emprestimo e);

#endif
