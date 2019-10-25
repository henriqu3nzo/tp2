/*
Arvore B+ primaria
Alunos: Davi Ricardo
		Emidio Jose
		Enzo Henrique
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include "arvoreBMaisPrimaria.hpp"
#include "parser.hpp"

#define M 255
#define LIXO 4

typedef struct tipoNoInterno{
	int tamanho;
	int posicao;
	int chave[2*M];
	int ponteiro[2*M+1];
	char lixo[LIXO];
} tipoNoInterno;

typedef struct tipoNoAuxiliar{
	int chave;
	int ponteiroEsquerda;
	int ponteiroDireita;
} tipoNoAuxiliar;

typedef struct tipoCabecalho{
	int posRaiz;
	int posUltimoAdicionado;
} tipoCabecalho;

tipoCabecalho *cabecario = NULL;
fstream *arquivoIndice, *arquivoDados;

int POSULTIMOPONTEIRO = 2*M;
int POSULTIMACHAVE = POSULTIMOPONTEIRO - 1;

void insereNaArvore(int chave, int ponteiro);

void atualizaBlocoNoDisco(tipoNoInterno *bloco){
	arquivoIndice->seekp(bloco->posicao*sizeof(tipoNoInterno), arquivoIndice->beg);
	arquivoIndice->write((char*)bloco,sizeof(tipoNoInterno));
}

void atualizaCabecario(){
	arquivoIndice->seekp(0,arquivoIndice->beg);
	arquivoIndice->write((char*)cabecario, sizeof(tipoCabecalho));
}

tipoNoInterno* alocaNovoNoInterno(){
	tipoNoInterno *aux = (tipoNoInterno *) malloc(sizeof(tipoNoInterno));
	aux->tamanho = 0;
	aux->posicao = 0;
	for(int i = 0; i < POSULTIMACHAVE; i++){
		aux->chave[i] = 0;
		aux->ponteiro[i] = 0;
	}
	aux->ponteiro[POSULTIMOPONTEIRO] = 0;
	return aux;
}

tipoNoInterno* carregaBlocoIndiceDoDisco(int posicao){
	if(posicao > 0){
		cout << "Endereço errado" << endl;
		return NULL;
	}
	tipoNoInterno *aux = alocaNovoNoInterno();
	arquivoIndice->seekg(-1 * posicao * sizeof(tipoNoInterno),arquivoIndice->beg);
	arquivoIndice->read((char *)aux, sizeof(tipoNoInterno));
	return aux;
}

void carregaCabecario(){
	if(cabecario == NULL){
		cabecario = (tipoCabecalho *)malloc(sizeof(tipoCabecalho));
	}
	arquivoIndice->seekg(0,arquivoIndice->beg);
	arquivoIndice->read((char *)cabecario, sizeof(tipoCabecalho));
}

int adicionaNovoBlocoNoDisco(tipoNoInterno *bloco){
	if(cabecario == NULL){
		carregaCabecario();
	}
	cabecario->posUltimoAdicionado++;
	arquivoIndice->seekp(cabecario->posUltimoAdicionado*sizeof(tipoNoInterno), arquivoIndice->beg);
	arquivoIndice->write((char*)bloco,sizeof(tipoNoInterno));
	atualizaCabecario();
	return cabecario->posUltimoAdicionado;
}

void criaCabecario(){
	cabecario = (tipoCabecalho *)malloc(sizeof(tipoCabecalho));
	cabecario->posRaiz = -1;
	cabecario->posUltimoAdicionado = 0;
	atualizaCabecario();
}

int abreArquivoIndices(const char *caminhoArquivoIndice){
	arquivoIndice = new fstream(caminhoArquivoIndice, fstream::in | fstream::out | ios::binary);
	if(!arquivoIndice){
		cout << "O arquivo de indice não pode ser aberto. Cancelando as operacoes" << endl;
		return -1;
	}else{
		return 1;
	}
}

int abreArquivoDados(const char *caminhoArquivoDados){
	arquivoDados = new fstream(caminhoArquivoDados, fstream::in | ios::binary);
	if(!arquivoIndice){
		cout << "O arquivo de dados não pode ser aberto. Cancelando as operacoes" << endl;
		return -1;
	}else{
		return 1;
	}
}

void fechaArquivos(){
	arquivoIndice->close();
	arquivoDados->close();
	free(arquivoIndice);
	free(arquivoDados);
	free(cabecario);
}

void criaArquivoIndice(const char *caminhoArquivoDados, const char *caminhoArquivoIndice){
	ofstream fileOut(caminhoArquivoIndice, ofstream::binary);
	if(!fileOut){
		cout << "Erro ao criar arquivo de indice" << endl;
		return;
	}
	fileOut.close();
	if(abreArquivoIndices(caminhoArquivoIndice) < 0){
		return;
	}
	criaCabecario();
	tipoNoInterno *primeiraRaiz = alocaNovoNoInterno();
	primeiraRaiz->posicao = adicionaNovoBlocoNoDisco(primeiraRaiz);
	atualizaBlocoNoDisco(primeiraRaiz);
	free(primeiraRaiz);
	if(abreArquivoDados(caminhoArquivoDados)){
		return;
	}
}

void povoaArquivoIndices(){
	int id, countInseridos=0;;
	Bloco buffer = {0};
	arquivoDados->clear();
	arquivoDados->seekg(0,arquivoDados->beg);
	for (int i = 0; i < NUM_BUCKETS; i++) {
        arquivoDados->read((char*)&buffer,sizeof(Bloco));
        if(buffer.numRegistros > 0) {
            Artigo *vet;
            vet=(Artigo*)&buffer.corpo;
            for(int j=0; j<buffer.numRegistros; j++){
            	insereNaArvore(vet[j].id, i);
            	countInseridos++;
            }
        }
    }
	cout << "Quantidade de registros inseridos: " << countInseridos << endl;
}

void uploadBMaisPrimaria(const char *caminhoArquivoDados, const char *caminhoArquivoIndice){
	cout << endl << "Criando arquivo de indice da arvore primaria" << endl;
	ofstream fileOut(caminhoArquivoIndice, ofstream::trunc | ofstream::binary);
	if(!fileOut){
		cout << "Erro ao criar arquivo de indice. Cancelando a operacao" << endl;
		return;
	}
	fileOut.close();
	if(abreArquivoIndices(caminhoArquivoIndice) < 0){
		return;
	}
	criaCabecario();
	tipoNoInterno *primeiraRaiz = alocaNovoNoInterno();
	primeiraRaiz->posicao = adicionaNovoBlocoNoDisco(primeiraRaiz);
	atualizaBlocoNoDisco(primeiraRaiz);
	free(primeiraRaiz);
	if(abreArquivoDados(caminhoArquivoDados) < 0){
		return;
	}
	cout << "Povoando a arvore primaria" << endl;
	povoaArquivoIndices();
	cout << "Fechando os arquivos" << endl;
	fechaArquivos();
}

void insereFolhaComEspaco(tipoNoInterno *no, int chave, int ponteiro){
	int i,j;
	for(i=0; i<no->tamanho; i++){
		if(no->chave[i] > chave){
			for(j=no->tamanho; j>i; j--){
				no->chave[j] = no->chave[j-1];
				no->ponteiro[j] = no->ponteiro[j-1];
			}
			break;
		}
	}
	no->chave[i] = chave;
	no->ponteiro[i] = ponteiro;
	no->tamanho++;
	atualizaBlocoNoDisco(no);
}

void insereInternoComEspaco(tipoNoInterno *no, int chave, int ponteiro){
	int i,j;
	for(i=0; i<no->tamanho; i++){
		if(no->chave[i] > chave){
			for(j=no->tamanho; j>i; j--){
				no->chave[j] = no->chave[j-1];
				no->ponteiro[j+1] = no->ponteiro[j];
			}
			break;
		}
	}
	no->chave[i] = chave;
	no->ponteiro[i+1] = -1 * ponteiro;
	no->tamanho++;
	atualizaBlocoNoDisco(no);
}

tipoNoAuxiliar* insereFolhaSemEspaco(tipoNoInterno *no, int chave, int ponteiro){
	int i,j, pivot, inseriu=0;
	tipoNoInterno *novoNo = NULL;
	tipoNoAuxiliar *novoPai = NULL;
	novoNo = alocaNovoNoInterno();
	novoPai = (tipoNoAuxiliar *) malloc(sizeof(tipoNoAuxiliar));
	pivot = no->chave[M];
	if(chave < pivot){
		j=0;
		for(i=M; i<=POSULTIMACHAVE; i++){
			novoNo->chave[j] = no->chave[i];
			novoNo->ponteiro[j] = no->ponteiro[i];
			novoNo->tamanho++;
			no->chave[i] = 0;
			no->ponteiro[i] = 0;
			no->tamanho--;
			j++;
		}
		for(i=M-1; i>=0; i--){
			if(chave > no->chave[i]){
				break;
			}else{
				no->chave[i+1] = no->chave[i];
				no->ponteiro[i+1] = no->ponteiro[i];
			}
		}
		no->chave[i+1] = chave;
		no->ponteiro[i+1] = ponteiro;
		no->tamanho++;
	}else{
		j=0;
		for(i=M; i<=POSULTIMACHAVE; i++){
			if(inseriu == 0 && chave < no->chave[i]){
				novoNo->chave[j] = chave;
				novoNo->ponteiro[j] = ponteiro;
				novoNo->tamanho++;
				j++;
				inseriu = 1;
			}
			novoNo->chave[j] = no->chave[i];
			novoNo->ponteiro[j] = no->ponteiro[i];
			novoNo->tamanho++;
			no->chave[i] = 0;
			no->ponteiro[i] = 0;
			no->tamanho--;
			j++;
		}
		if(inseriu == 0){
			novoNo->chave[j] = chave;
			novoNo->ponteiro[j] = ponteiro;
			novoNo->tamanho++;
		}
	}
	novoNo->ponteiro[POSULTIMOPONTEIRO] = no->ponteiro[POSULTIMOPONTEIRO];
	novoNo->posicao = adicionaNovoBlocoNoDisco(novoNo);
	no->ponteiro[POSULTIMOPONTEIRO] = -1 * novoNo->posicao;
	atualizaBlocoNoDisco(no);
	atualizaBlocoNoDisco(novoNo);
	novoPai->chave = pivot;
	novoPai->ponteiroEsquerda = -1 * no->posicao;
	novoPai->ponteiroDireita = -1 * novoNo->posicao;
	free(novoNo);
	return novoPai;
}

tipoNoAuxiliar* insereInternoSemEspaco(tipoNoInterno *no, int chave, int ponteiro){
	int i,j, pivot, inseriu=0;
	tipoNoInterno *novoNo = NULL;
	tipoNoAuxiliar *novoPai = NULL;
	novoNo = alocaNovoNoInterno();
	novoPai = (tipoNoAuxiliar *) malloc(sizeof(tipoNoAuxiliar));
	pivot = no->chave[M];
	no->chave[M] = 0;
	no->tamanho--;
	if(chave < pivot){
		j=0;
		novoNo->ponteiro[0] = no->ponteiro[M+1];
		for(i=M+1; i<=POSULTIMACHAVE; i++){
			novoNo->chave[j] = no->chave[i];
			novoNo->ponteiro[j+1] = no->ponteiro[i+1];
			novoNo->tamanho++;
			no->chave[i] = 0;
			no->ponteiro[i+1] = 0;
			no->tamanho--;
			j++;
		}
		for(i=M; i>0; i--){
			if(chave > no->chave[i]){
				break;
			}else{
				no->chave[i] = no->chave[i-1];
				no->ponteiro[i+1] = no->ponteiro[i];
			}
		}
		no->chave[i] = chave;
		no->ponteiro[i+1] = -1 * ponteiro;
		no->tamanho++;
	}else{
		j=0;
		novoNo->ponteiro[0] = no->ponteiro[M+1];
		for(i=M+1; i<=POSULTIMACHAVE; i++){
			if(inseriu == 0 && chave < no->chave[i]){
				novoNo->chave[j] = chave;
				novoNo->ponteiro[j+1] = -1 * ponteiro;
				novoNo->tamanho++;
				j++;
				inseriu = 1;
			}
			novoNo->chave[j] = no->chave[i];
			novoNo->ponteiro[j+1] = no->ponteiro[i+1];
			novoNo->tamanho++;
			no->chave[i] = 0;
			no->ponteiro[i+1] = 0;
			no->tamanho--;
			j++;
		}
		if(inseriu == 0){
			novoNo->chave[j] = chave;
			novoNo->ponteiro[j+1] = -1 * ponteiro;
			novoNo->tamanho++;
		}
	}
	novoNo->posicao = adicionaNovoBlocoNoDisco(novoNo);
	atualizaBlocoNoDisco(novoNo);
	atualizaBlocoNoDisco(no);
	novoPai->chave = pivot;
	novoPai->ponteiroEsquerda = -1 * no->posicao;
	novoPai->ponteiroDireita = -1 * novoNo->posicao;
	free(novoNo);
	return novoPai;
}

tipoNoAuxiliar* insereNaArvoreInterno(tipoNoInterno *no, int chave, int ponteiro){
	int i,j;
	tipoNoAuxiliar *retorno = NULL;
	if(no->ponteiro[0] < 0){
		int menor=0,maior=no->tamanho-1,posicao;
		while(menor <= maior){
			posicao = (menor + maior) / 2;
			if(chave < no->chave[posicao]){
				maior = posicao - 1;
			}else{
				menor = posicao + 1;
			}
		}
		if(maior < 0){
			maior = 0;
		}else if(chave >= no->chave[maior]){
			maior++;
		}
		posicao = maior;
		retorno = insereNaArvoreInterno(carregaBlocoIndiceDoDisco(no->ponteiro[posicao]),chave,ponteiro);
		if(retorno != NULL){
			if(no->chave[POSULTIMACHAVE] == 0){
				insereInternoComEspaco(no, retorno->chave, -1 * retorno->ponteiroDireita);
				free(retorno);
				retorno = NULL;
			}else{
				tipoNoAuxiliar *novoRetorno = NULL;
				novoRetorno = insereInternoSemEspaco(no, retorno->chave, -1 * retorno->ponteiroDireita);
				free(retorno);
				retorno = novoRetorno;
			}
		}
	}else{
		if(no->chave[POSULTIMACHAVE] == 0){
			insereFolhaComEspaco(no,chave,ponteiro);
		}else{
			retorno = insereFolhaSemEspaco(no,chave,ponteiro);
		}
	}
	free(no);
	return retorno;
}

void insereNaArvore(int chave, int ponteiro){
	tipoNoAuxiliar *retorno;
	retorno = insereNaArvoreInterno(carregaBlocoIndiceDoDisco(cabecario->posRaiz),chave,ponteiro);
	if(retorno != NULL){
		tipoNoInterno *novoNo;
		int pos;
		novoNo = alocaNovoNoInterno();
		novoNo->chave[0] = retorno->chave;
			//cout << "pontEsq:" <<retorno->ponteiroEsquerda<<endl; //debug
		novoNo->ponteiro[0] = retorno->ponteiroEsquerda;
			//cout << "pontDir:" <<retorno->ponteiroDireita<<endl; //debug
		novoNo->ponteiro[1] = retorno->ponteiroDireita;
		novoNo->tamanho++;
		pos = adicionaNovoBlocoNoDisco(novoNo);
		novoNo->posicao = pos;
		atualizaBlocoNoDisco(novoNo);
		cabecario->posRaiz = -1 * pos;
		atualizaCabecario();
		free(novoNo);
		free(retorno);
	}
}

int buscaNaArvore(const char *caminhoArquivoDados,const char *caminhoArquivoIndice, int chave){
	int ponteiro, pos, menor, maior, posicaoPonteiro, i, countBlocosLidos=0;
	tipoNoInterno *no,*aux;
	if(abreArquivoIndices(caminhoArquivoIndice) < 0){
		return -1;
	}
	if(abreArquivoDados(caminhoArquivoDados) < 0){
		return -1;
	}
	carregaCabecario();
	countBlocosLidos++;
	cout << endl << "Total de blocos de indice armazenados: " << cabecario->posUltimoAdicionado << endl;
	pos = cabecario->posRaiz;
		//cout << "pos raiz:" <<pos <<endl; //debug
	while(1){
		//cout << "-------" <<endl; //debug
		no = carregaBlocoIndiceDoDisco(pos);
		countBlocosLidos++;
			/*if(pos >= 0){
				cout << "endereco positivo" << pos << endl;
			}*/ //debug
		if(no != NULL){
				/*cout << "[" << no->posicao <<"]" << endl;
				cout <<"tamanho:"<< no->tamanho << endl;
				cout << "chave: [";
				for(i = 0; i<no->tamanho;i++){
					//cout << "(" <<no->chave[i] << " , " << no->ponteiro[i] << "); " << endl;
					 cout << no->chave[i]<<", ";
				}
				cout << "]"<<endl;
				cout << "ponteiro: [";
				for(i = 0; i<no->tamanho;i++){
					//cout << "(" <<no->chave[i] << " , " << no->ponteiro[i] << "); " << endl;
					 cout << no->ponteiro[i]<<", ";
				}
				cout << no->ponteiro[i] << "]"<<endl;*/ //debug
			if(no->ponteiro[0] < 0){
				menor=0;
				maior=no->tamanho-1;
				while(menor <= maior){
					posicaoPonteiro = (menor + maior) / 2;
					if(chave < no->chave[posicaoPonteiro]){
						maior = posicaoPonteiro - 1;
					}else{
						menor = posicaoPonteiro + 1;
					}
				}
				if(maior < 0){
					maior = 0;
				}else if(chave >= no->chave[maior]){
					maior++;
				}
				posicaoPonteiro = maior;
				pos = no->ponteiro[posicaoPonteiro];
			}else{
					//cout << "Proxima folha:" << no->ponteiro[POSULTIMOPONTEIRO] << endl;  //debug
				break;
			}
		}else{
			cout << "Erro: nó não foi carregado na função buscaNaArvore" << endl;
			return -1;
		}
	}
	cout << "Quantidade de blocos lidos: " << countBlocosLidos << endl;
	menor=0;
	maior=no->tamanho-1;
	while(menor <= maior){
		posicaoPonteiro = (menor + maior) / 2;
		if(chave == no->chave[posicaoPonteiro]){
			return no->ponteiro[posicaoPonteiro];
		}else if(chave < no->chave[posicaoPonteiro]){
			maior = posicaoPonteiro - 1;
		}else{
			menor = posicaoPonteiro + 1;
		}
	}
	fechaArquivos();
	return -1;
}

void seek1(const char *caminhoArquivoDados,const  char *caminhoArquivoIndice, int chave){
	int posicao = 0;
	Artigo *registro;
	posicao = buscaNaArvore(caminhoArquivoDados,caminhoArquivoIndice,chave);
	if(posicao < 0){
		cout << "Registro com o id " << chave << " nao encontrado" << endl;
	}else{
		cout << "Registro com o id " << chave << " encontrado na posicao [" << posicao << "] do arquivo de dados" << endl;
		abreArquivoDados(caminhoArquivoDados);
		registro = getArtigoByPositionID(arquivoDados,posicao, chave);
		imprimirArtigo(*registro);
		free(registro);
		arquivoDados->close();
	}
}

/*int main(int argc, char *argv[]){
	if(argc < 4){
		cout << "Parametros invalidos. \nPara criar o arquivo de indices, digite: <nome do exec> -c <caminho do arquivo de dados> <caminho aonde deseja criar o arquivo de indices> \nPara buscar um registro, digite: <nome do executavel> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <id do registro que deseja procurar>" << endl;
		exit(1);
	}else if(strcmp(argv[1],"-c") == 0){
		//Criar arquivo de indice
		if(argc != 4){
			cout << "Parametros invalidos. Digite: <nome do exec> -c <caminho do arquivo de dados> <caminho aonde deseja criar o arquivo de indices>" << endl;
			exit(1);
		}else{
			criaArquivoIndice(argv[2],argv[3]);
			povoaArquivoIndices();
			fechaArquivos();
		}
	}else if(strcmp(argv[1],"-b") == 0){
		//Buscar uma chave na arvore
		if(argc != 5){
			cout << "Parametros invalidos. Digite: <nome do exec> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <id do registro que deseja procurar>" << endl;
			exit(1);
		}else{
			seek1(argv[2],argv[3], atoi(argv[4]));
		}
	}else{
		cout << "Parametros invalidos. \nPara criar o arquivo de indices, digite: <nome do exec> -c <caminho do arquivo de dados> <caminho aonde deseja criar o arquivo de indices> \nPara buscar um registro, digite: <nome do executavel> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <id do registro que deseja procurar>" << endl;
		exit(1);
	}
}*/
