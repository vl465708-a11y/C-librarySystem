#ifndef USUARIOS_H
#define USUARIOS_H

typedef struct {
    int matricula;
    char Nome[100];
    char Curso[50];
    char tel[15];
    int diaCad, mesCad, anoCad;
} CadastroUsuario;

// funções de usuários
void CadastrarUsuario(CadastroUsuario *u);
void exibirUsuario(CadastroUsuario u);
int pesquisarusuario(CadastroUsuario usuarios[], int total);
int procurarUsuarioPorMatricula(int matricula, CadastroUsuario usuarios[], int total);

#endif
