#ifndef PARSER_h
#define PARSER_h

#include <fstream>      // fstream
#include <string.h>     // strcpye outras
#include <stdlib.h>     // atoi

#define NUM_BUCKETS 215000
#define FATOR_BLOCO 7

#define CAMINHOSHASH "hashFile"

using namespace std;

typedef struct {
    unsigned int  id;
    char titulo[300];
    unsigned int  ano;
    char autor[100];
    unsigned int  citacoes;
    char atualizacao[20];
    char snippet[100];
}Artigo;

typedef struct {
    unsigned int  numRegistros;
    char cabecalho[365];
    char corpo[3724];
}Bloco;

void imprimirArtigo(Artigo art);

Bloco * getBucketHashFileByPosition(fstream *arq, int position);

Artigo* getArtigoByPositionID(fstream *arq, int position, int id);

Artigo* getArtigoByPositionTitle(fstream *arq, int position, char title[300]);

int hashSimples(int id);

int inserirNoHashFile(fstream *arq, Artigo artigo);

Artigo findrec(fstream *arq,int id, bool imprimir);

Artigo getArtigo(ifstream *arq);

int getColisoes();

#endif