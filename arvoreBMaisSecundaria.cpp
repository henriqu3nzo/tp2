#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#include "arvoreBMaisSecundaria.hpp"
#include "parser.hpp"

#define M2 6
#define LIXO2 436

typedef struct tipoNoInterno2{ //Tipo de qualquer nó da arvore
	int tamanho; //Quantidade de chaves ocupadas no nó (>0)
	int posicao; //Posicao no arquivo de indice ocupada pelo nó (>0). Este valor é positivo pois só pode referenciar a um bloco no arquivo de indice
	char chave[2*M2][300];
	int ponteiro[2*M2+1];
	char lixo[LIXO2]; //Caracteres para completar o bloco, fazendo com que ele ocupe 4KB
} tipoNoInterno2;

typedef struct tipoNoAuxiliar2{ //Tipo temporario para indicar os valores para fazer a arvore crescer de nivel
	char chave[300];
	int ponteiroEsquerda;
	int ponteiroDireita;
} tipoNoAuxiliar2;

typedef struct tipoCabecalho2{
	int posRaiz;
	int posUltimoAdicionado; //Quantidade de nós == posicao do ultimo adicionado
} tipoCabecalho2;

tipoCabecalho2 *cabecalho2 = NULL;
fstream *arquivoIndice2, *arquivoDados2;

int POSULTIMOPONTEIRO2 = 2*M2;
int POSULTIMACHAVE2 = POSULTIMOPONTEIRO2 - 1;
int countNaoInseridos;

//---------- Header Interno

void insereNaArvore2(char chave[300], int ponteiro);

//atualiza os blocos no disco com um arquivo de índice secundário.
void atualizaBlocoNoDisco2(tipoNoInterno2 *bloco){
	arquivoIndice2->seekp(bloco->posicao*sizeof(tipoNoInterno2), arquivoIndice2->beg);
	arquivoIndice2->write((char*)bloco,sizeof(tipoNoInterno2));
}

//atualiza o cabeçalho com um arquivo de índice secundário.
void atualizaCabecalho2(){
	arquivoIndice2->seekp(0,arquivoIndice2->beg);
	arquivoIndice2->write((char*)cabecalho2, sizeof(tipoCabecalho2));
}

//Caso seja precisa essa função aloca um novo nó interno.
tipoNoInterno2* alocaNovoNoInterno2(){
	tipoNoInterno2 *aux = (tipoNoInterno2 *) malloc(sizeof(tipoNoInterno2));
	aux->tamanho = 0;
	aux->posicao = 0;
	for(int i = 0; i < POSULTIMACHAVE2; i++){
		strcpy(aux->chave[i],"\0");
		aux->ponteiro[i] = 0;
	}
	aux->ponteiro[POSULTIMOPONTEIRO2] = 0;

	return aux;
}


// Carrega um bloco de indice do arquivo de indice secundário.
tipoNoInterno2* carregaBlocoIndiceDoDisco2(int posicao){
	if(posicao > 0){
		cout << "Endereço errado" << endl;
		return NULL;
	}
	tipoNoInterno2 *aux = alocaNovoNoInterno2();
	arquivoIndice2->seekg(-1 * posicao * sizeof(tipoNoInterno2),arquivoIndice2->beg);
	arquivoIndice2->read((char *)aux, sizeof(tipoNoInterno2));
	return aux;
}

// Carrega um cabeçalho do arquivo de indice secundário.
void carregaCabecalho2(){
	if(cabecalho2 == NULL){
		cabecalho2 = (tipoCabecalho2 *)malloc(sizeof(tipoCabecalho2));
	}

	arquivoIndice2->seekg(0,arquivoIndice2->beg);
	arquivoIndice2->read((char *)cabecalho2, sizeof(tipoCabecalho2));
}
// Caso seja necessário, esta função adiciona novos blocos no disco.
int adicionaNovoBlocoNoDisco2(tipoNoInterno2 *bloco){
	if(cabecalho2 == NULL){
		carregaCabecalho2();
	}

	cabecalho2->posUltimoAdicionado++;
	arquivoIndice2->seekp(cabecalho2->posUltimoAdicionado*sizeof(tipoNoInterno2), arquivoIndice2->beg);
	arquivoIndice2->write((char*)bloco,sizeof(tipoNoInterno2));
	atualizaCabecalho2();
	return cabecalho2->posUltimoAdicionado;
}

//Cria um cabeçalho e atualiza o mesmo.
void criaCabecalho2(){
	cabecalho2 = (tipoCabecalho2 *)malloc(sizeof(tipoCabecalho2));
	cabecalho2->posRaiz = -1;
	cabecalho2->posUltimoAdicionado = 0;
	atualizaCabecalho2();
}

// Abre um arquivos de índices secundário.
int abreArquivoIndices2(const char *caminhoArquivoIndice){
	arquivoIndice2 = new fstream(caminhoArquivoIndice, fstream::in | fstream::out | ios::binary); 
	if(!arquivoIndice2){
		cout << "O arquivo de indice não pode ser aberto. Cancelando as operacoes" << endl;
		return -1;
	}else{
		return 1;
	}
}

// Abre um arquivo de dados.
int abreArquivoDados2(const char *caminhoArquivoDados){
	arquivoDados2 = new fstream(caminhoArquivoDados, fstream::in | ios::binary); 
	if(!arquivoIndice2){
		cout << "O arquivo de dados não pode ser aberto. Cancelando as operacoes" << endl;
		return -1;
	}else{
		return 1;
	}
}

//Fecha um arquivo de dados e índice secundário.
void fechaArquivos2(){
	arquivoIndice2->close();
	arquivoDados2->close();
	free(arquivoIndice2);
	free(arquivoDados2);
	free(cabecalho2);
}

//Cria um novo arquivo de índices secundário.
void criaArquivoIndice2(const char *caminhoArquivoDados, const char *caminhoArquivoIndice){
	ofstream fileOut(caminhoArquivoIndice, ofstream::binary);
	if(!fileOut){
		cout << "Erro ao criar arquivo de indice" << endl;
		return;
	}
	fileOut.close();

	if(abreArquivoIndices2(caminhoArquivoIndice) < 0){
		return;
	}
	criaCabecalho2();
	tipoNoInterno2 *primeiraRaiz = alocaNovoNoInterno2();
	primeiraRaiz->posicao = adicionaNovoBlocoNoDisco2(primeiraRaiz);
	atualizaBlocoNoDisco2(primeiraRaiz);
	free(primeiraRaiz);

	if(abreArquivoDados2(caminhoArquivoDados)){
		return;
	}
}

//Insere os índices no arquivo de índices secundário.
void povoaArquivoIndices2(){
	int id, countInseridos=0;
	Bloco buffer = {0};

	countNaoInseridos = 0;

	arquivoDados2->clear();
	arquivoDados2->seekg(0,arquivoDados2->beg);
	
	for (int i = 0; i < NUM_BUCKETS; i++) { 
        arquivoDados2->read((char*)&buffer,sizeof(Bloco));

        if(buffer.numRegistros > 0) {
            Artigo *vet;
            vet=(Artigo*)&buffer.corpo;

            for(int j=0; j<buffer.numRegistros; j++){
            	insereNaArvore2(vet[j].titulo, i);
            	countInseridos++;
            } 
        }

    }

	cout << "Quantidade de registros inseridos: " << countInseridos - countNaoInseridos << endl;
}

// Dar um upload na árvore B+ secundária.
void uploadBMaisSecundaria(const char *caminhoArquivoDados, const char *caminhoArquivoIndice){
	//---- Cria e inicializa o arquivo de indice
	cout << endl << "Criando arquivo de indice da arvore secundaria" << endl;
	ofstream fileOut(caminhoArquivoIndice, ofstream::trunc | ofstream::binary);
	if(!fileOut){
		cout << "Erro ao criar arquivo de indice. Cancelando a operacao" << endl;
		return;
	}
	fileOut.close();

	if(abreArquivoIndices2(caminhoArquivoIndice) < 0){
		return;
	}
	criaCabecalho2();
	tipoNoInterno2 *primeiraRaiz = alocaNovoNoInterno2();
	primeiraRaiz->posicao = adicionaNovoBlocoNoDisco2(primeiraRaiz);
	atualizaBlocoNoDisco2(primeiraRaiz);
	free(primeiraRaiz);

	if(abreArquivoDados2(caminhoArquivoDados) < 0){
		return;
	}

	//----- Povoa a arvore
	cout << "Povoando a arvore secundaria" << endl;
	povoaArquivoIndices2();

	//---- Fecha o arquivo de indice
	cout << "Fechando os arquivos" << endl;
	fechaArquivos2();
}

//---------- Inserção na arvore

// Insere uma folha na árvore B+ secundária.
void insereFolhaComEspaco2(tipoNoInterno2 *no, char chave[300], int ponteiro){
	int i,j;

	for(i=0; i<no->tamanho; i++){
		if(strcmp(no->chave[i],chave) > 0){
			for(j=no->tamanho; j>i; j--){
				strcpy(no->chave[j], no->chave[j-1]);
				no->ponteiro[j] = no->ponteiro[j-1];
			}
			break;
		} else if(strcmp(no->chave[i],chave) == 0){ //chave já inserida
			countNaoInseridos++;
			return;
		}
	}

	strcpy(no->chave[i], chave);
	no->ponteiro[i] = ponteiro;
	no->tamanho++;

	atualizaBlocoNoDisco2(no);
}

// Insere um nó interno em um grupo com espaços na árvore.
void insereInternoComEspaco2(tipoNoInterno2 *no, char chave[300], int ponteiro){
	int i,j;

	for(i=0; i<no->tamanho; i++){
		if(strcmp(no->chave[i],chave) > 0){
			for(j=no->tamanho; j>i; j--){
				strcpy(no->chave[j], no->chave[j-1]);
				no->ponteiro[j+1] = no->ponteiro[j];
			}
			break;
		}
	}

	strcpy(no->chave[i], chave);
	no->ponteiro[i+1] = -1 * ponteiro; //O -1 é para identificar que é um endereco de indice
	no->tamanho++;

	atualizaBlocoNoDisco2(no);
}

// Insere uma folha em um grupo sem espaços na árvore.
tipoNoAuxiliar2* insereFolhaSemEspaco2(tipoNoInterno2 *no, char chave[300], int ponteiro){
	int i,j, inseriu=0;
	char pivot[300];
	tipoNoInterno2 *novoNo = NULL;
	tipoNoAuxiliar2 *novoPai = NULL;

	novoNo = alocaNovoNoInterno2();
	novoPai = (tipoNoAuxiliar2 *) malloc(sizeof(tipoNoAuxiliar2));

	strcpy(pivot,no->chave[M2]); //Escolhe o pivot

	if(strcmp(chave,pivot) < 0){ //O novo valor vai ser adicionado no bloco da esquerda
		j=0;
		for(i=M2; i<=POSULTIMACHAVE2; i++){ //M2ove os valores do bloco da esquerda para o bloco da direita
			strcpy(novoNo->chave[j], no->chave[i]);
			novoNo->ponteiro[j] = no->ponteiro[i];
			novoNo->tamanho++;

			strcpy(no->chave[i], "\0");
			no->ponteiro[i] = 0;
			no->tamanho--;

			j++;
		}

		for(i=M2-1; i>=0; i--){ //Procura a posicao para inserir a chave
			if(strcmp(chave,no->chave[i]) > 0){
				//Posicao encontrada
				break;
			}else if(strcmp(chave,no->chave[i]) == 0){
				//valor já inserido, revertendo insercao
				for(i=i+1;i<=M2-1;i++){
					strcpy(no->chave[i], no->chave[i+1]);
					no->ponteiro[i] = no->ponteiro[i+1];
				}

				for(i=M2; i<=POSULTIMACHAVE2; i++){
					strcpy(no->chave[i], novoNo->chave[i-M2]);
					no->ponteiro[i] = novoNo->ponteiro[i-M2];
					no->tamanho++;
				}

				free(novoNo);
				free(novoPai);
				return NULL;
			}else{
				strcpy(no->chave[i+1], no->chave[i]);
				no->ponteiro[i+1] = no->ponteiro[i];
			}
		}

		strcpy(no->chave[i+1], chave);
		no->ponteiro[i+1] = ponteiro;
		no->tamanho++;
	}else{
		j=0;
		for(i=M2; i<=POSULTIMACHAVE2; i++){
			if(strcmp(chave, no->chave[i]) == 0){
				//valor já inserido, revertendo insercao
				for(i=0; i<novoNo->tamanho; i++){
					strcpy(no->chave[M2+i], novoNo->chave[i]);
					no->ponteiro[M2+i] = novoNo->ponteiro[i];
					no->tamanho++;
				}

				free(novoNo);
				free(novoPai);
				countNaoInseridos++;
				return NULL;
			}

			if(inseriu == 0 && strcmp(chave, no->chave[i]) < 0){//Verifica se a posicao para inserir a chave é a atual
				//Insere a nova chave
				strcpy(novoNo->chave[j],chave);
				novoNo->ponteiro[j] = ponteiro;
				novoNo->tamanho++;
				j++;
				inseriu = 1;
			}

			strcpy(novoNo->chave[j],no->chave[i]);//M2ove os valores do bloco da esquerda para o bloco da direita
			novoNo->ponteiro[j] = no->ponteiro[i];
			novoNo->tamanho++;

			strcpy(no->chave[i], "\0");
			no->ponteiro[i] = 0;
			no->tamanho--;

			j++;
		}

		if(inseriu == 0){
			//Insere a nova chave depois das outras chaves
			strcpy(novoNo->chave[j],chave);
			novoNo->ponteiro[j] = ponteiro;
			novoNo->tamanho++;
		}
	}


	novoNo->ponteiro[POSULTIMOPONTEIRO2] = no->ponteiro[POSULTIMOPONTEIRO2];
	novoNo->posicao = adicionaNovoBlocoNoDisco2(novoNo);
	no->ponteiro[POSULTIMOPONTEIRO2] = -1 * novoNo->posicao; //Recebe a posicao do novo no
	atualizaBlocoNoDisco2(no);
	atualizaBlocoNoDisco2(novoNo);

	strcpy(novoPai->chave, pivot);
	novoPai->ponteiroEsquerda = -1 * no->posicao;
	novoPai->ponteiroDireita = -1 * novoNo->posicao;

	free(novoNo);

	return novoPai; //Retorna os valores para serem adicionados no nó pai
}

// Insere um nó interno em um grupo sem espaços na árvore.
tipoNoAuxiliar2* insereInternoSemEspaco2(tipoNoInterno2 *no, char chave[300], int ponteiro){
	int i,j, inseriu=0;
	char pivot[300];
	tipoNoInterno2 *novoNo = NULL;
	tipoNoAuxiliar2 *novoPai = NULL;

	novoNo = alocaNovoNoInterno2();
	novoPai = (tipoNoAuxiliar2 *) malloc(sizeof(tipoNoAuxiliar2));

	strcpy(pivot, no->chave[M2]);
	strcpy(no->chave[M2], "\0"); //Deleta o pivot do nó intermediario
	no->tamanho--;

	if(strcmp(chave, pivot) < 0){ //O novo valor vai ser adicionado no bloco da esquerda
		j=0;

		novoNo->ponteiro[0] = no->ponteiro[M2+1];

		for(i=M2+1; i<=POSULTIMACHAVE2; i++){
			strcpy(novoNo->chave[j],no->chave[i]);
			novoNo->ponteiro[j+1] = no->ponteiro[i+1];
			novoNo->tamanho++;

			strcpy(no->chave[i], "\0");
			no->ponteiro[i+1] = 0;
			no->tamanho--;

			j++;
		}

		for(i=M2; i>0; i--){
			if(strcmp(chave, no->chave[i]) > 0){
				//Posicao para inserir a chave encontrado
				break;
			}else{
				strcpy(no->chave[i],no->chave[i-1]);
				no->ponteiro[i+1] = no->ponteiro[i];
			}
		}

		strcpy(no->chave[i],chave);
		no->ponteiro[i+1] = -1 * ponteiro;
		no->tamanho++;
	}else{
		j=0;

		novoNo->ponteiro[0] = no->ponteiro[M2+1];

		for(i=M2+1; i<=POSULTIMACHAVE2; i++){
			if(inseriu == 0 && strcmp(chave, no->chave[i]) < 0){
				//Insere a nova chave
				strcpy(novoNo->chave[j],chave);
				novoNo->ponteiro[j+1] = -1 * ponteiro;
				novoNo->tamanho++;
				j++;
				inseriu = 1;
			}

			strcpy(novoNo->chave[j],no->chave[i]);
			novoNo->ponteiro[j+1] = no->ponteiro[i+1];
			novoNo->tamanho++;

			strcpy(no->chave[i], "\0");
			no->ponteiro[i+1] = 0;
			no->tamanho--;

			j++;
		}

		if(inseriu == 0){
			//Insere a nova chave depois das outras chaves
			strcpy(novoNo->chave[j],chave);
			novoNo->ponteiro[j+1] = -1 * ponteiro;
			novoNo->tamanho++;
		}
	}


	novoNo->posicao = adicionaNovoBlocoNoDisco2(novoNo);
	atualizaBlocoNoDisco2(novoNo);
	atualizaBlocoNoDisco2(no);

	strcpy(novoPai->chave, pivot);
	novoPai->ponteiroEsquerda = -1 * no->posicao;
	novoPai->ponteiroDireita = -1 * novoNo->posicao;

	free(novoNo);

	return novoPai; //Retorna os valores para serem adicionados no nó pai
}

// Insere na parte mais interna da árvore B+ secundária.
tipoNoAuxiliar2* insereNaArvoreInterno2(tipoNoInterno2 *no, char chave[300], int ponteiro){
	int i,j;
	tipoNoAuxiliar2 *retorno = NULL;

	if(no->ponteiro[0] < 0){ //O nó é interno
		//Busca filho contendo a posicao para inserir a chave
		int menor=0,maior=no->tamanho-1,posicao;

		while(menor <= maior){
			posicao = (menor + maior) / 2;

			if(strcmp(chave, no->chave[posicao]) < 0){
				maior = posicao - 1;
			}else{
				menor = posicao + 1;
			}
		}

		if(maior < 0){
			maior = 0;
		}else if(strcmp(chave, no->chave[maior]) >= 0){
			maior++;
		}

		posicao = maior;

		//Faz a chamada recursiva para inserir o valor
		retorno = insereNaArvoreInterno2(carregaBlocoIndiceDoDisco2(no->ponteiro[posicao]),chave,ponteiro);

		//Verifica se o filho estava cheio ao inserir o novo valor
		if(retorno != NULL){
			if(strcmp(no->chave[POSULTIMACHAVE2], "\0") == 0){ //Verifica se o no atual tem espaco
				insereInternoComEspaco2(no, retorno->chave, -1 * retorno->ponteiroDireita);
				free(retorno);
				retorno = NULL;
			}else{
				tipoNoAuxiliar2 *novoRetorno = NULL;
				novoRetorno = insereInternoSemEspaco2(no, retorno->chave, -1 * retorno->ponteiroDireita);
				free(retorno);
				retorno = novoRetorno;
			}
		}
	}else{ 
		//O nó atual é folha
		if(strcmp(no->chave[POSULTIMACHAVE2], "\0") == 0){ 
			//A folha tem espaco
			insereFolhaComEspaco2(no,chave,ponteiro);
		}else{
			retorno = insereFolhaSemEspaco2(no,chave,ponteiro);
			//Se retornar algum valor, este valor vai ser adicionado no nó pai
		}
	}

	free(no);
	return retorno;
}

// Insere um dado na árvore B+ secundária.
void insereNaArvore2(char chave[300], int ponteiro){
	tipoNoAuxiliar2 *retorno;

	if(strcmp(chave,"\0") == 0){ //se o titulo for vazio, nao insere
		countNaoInseridos++;
		return;
	}
	
		//cout << ">inserindo o valor:" << chave << endl; //debug
	retorno = insereNaArvoreInterno2(carregaBlocoIndiceDoDisco2(cabecalho2->posRaiz),chave,ponteiro);
		//cout << "-valor inserido" << endl; //debug

	if(retorno != NULL){
		//Se retorno for diferente de null, isso quer dizer que a arvore precisa crescer um nivel
		tipoNoInterno2 *novoNo;
		int pos;

		novoNo = alocaNovoNoInterno2();
		strcpy(novoNo->chave[0], retorno->chave);
			//cout << "pontEsq:" <<retorno->ponteiroEsquerda<<endl; //debug
		novoNo->ponteiro[0] = retorno->ponteiroEsquerda;
			//cout << "pontDir:" <<retorno->ponteiroDireita<<endl; //debug
		novoNo->ponteiro[1] = retorno->ponteiroDireita;
		novoNo->tamanho++;

		pos = adicionaNovoBlocoNoDisco2(novoNo);
		novoNo->posicao = pos;
		atualizaBlocoNoDisco2(novoNo);
		cabecalho2->posRaiz = -1 * pos; //Apenas para indicar que o valor é um endereco de indice (e evitar a conversão ao buscar um bloco do disco)
		atualizaCabecalho2();

		free(novoNo);
		free(retorno);
	}
}

// Busca um dado na árvore B+ secundária.
int buscaNaArvore2(const char *caminhoArquivoDados,const char *caminhoArquivoIndice, char chave[300]){
	int ponteiro, pos, menor, maior, posicaoPonteiro, i, countBlocosLidos=0;
	tipoNoInterno2 *no,*aux;

	if(strcmp(chave,"\0") == 0){ //Estava gerando um erro ao comparar os valores com o cabecario
		return -1;
	}

	if(abreArquivoIndices2(caminhoArquivoIndice) < 0){
		return -1;
	}
	if(abreArquivoDados2(caminhoArquivoDados) < 0){
		return -1;
	}

	carregaCabecalho2();
	countBlocosLidos++;

	cout << endl << "Total de blocos de indice armazenados: " << cabecalho2->posUltimoAdicionado << endl;

	pos = cabecalho2->posRaiz;
		//cout << "pos raiz:" <<pos <<endl; //debug

	while(1){

			//cout << "-------" <<endl; //debug

		no = carregaBlocoIndiceDoDisco2(pos);
		countBlocosLidos++;

			/*if(pos >= 0){
				cout << "endereco positivo" << pos << endl;
			}*/ //debug

		if(no != NULL){
			//Nó foi lido

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
				//Nó atual é interno, então deve buscar a posicao do filho que pode conter o valor
				menor=0;
				maior=no->tamanho-1;

				while(menor <= maior){
					posicaoPonteiro = (menor + maior) / 2;

					if(strcmp(chave, no->chave[posicaoPonteiro]) < 0){
						maior = posicaoPonteiro - 1;
					}else{
						menor = posicaoPonteiro + 1;
					}
				}

				if(maior < 0){
					maior = 0;
				}else if(strcmp(chave, no->chave[maior]) >= 0){
					maior++;
				}

				posicaoPonteiro = maior;
				pos = no->ponteiro[posicaoPonteiro];
			}else{
				//Nó atual é folha

					//cout << "Proxima folha:" << no->ponteiro[POSULTIMOPONTEIRO2] << endl;  //debug

				break;
			}
		}else{
			cout << "Erro: nó não foi carregado na função buscaNaArvore" << endl;
			return -1;
		}
	}

	//Folha encontrada, procurando valor no bloco lido
	cout << "Quantidade de blocos lidos: " << countBlocosLidos << endl;

	menor=0;
	maior=no->tamanho-1;

	while(menor <= maior){
		posicaoPonteiro = (menor + maior) / 2;

		if(strcmp(chave, no->chave[posicaoPonteiro]) == 0){
			return no->ponteiro[posicaoPonteiro];
		}else if(strcmp(chave, no->chave[posicaoPonteiro]) < 0){
			maior = posicaoPonteiro - 1;
		}else{
			menor = posicaoPonteiro + 1;
		}
	}

	fechaArquivos2();
	return -1; //O valor não foi encontrado
}

// Chama o programa seek2 que busca na ávore pelo título, através de um arquivo de índice  secundário.
void seek2(const char *caminhoArquivoDados,const  char *caminhoArquivoIndice, char chave[300]){
	int posicao = 0;
	Artigo *registro;

	posicao = buscaNaArvore2(caminhoArquivoDados,caminhoArquivoIndice,chave);

	if(posicao < 0){
		cout << "Registro com o titulo \"" << chave << "\" nao encontrado" << endl;
	}else{
		cout << "Registro com o titulo \"" << chave << "\" encontrado na posicao [" << posicao << "] do arquivo de dados" << endl;
		abreArquivoDados2(caminhoArquivoDados);
		registro = getArtigoByPositionTitle(arquivoDados2,posicao, chave);
		imprimirArtigo(*registro);
		free(registro);
		arquivoDados2->close();
	}
}

/*int main(int argc, char *argv[]){
	if(argc < 4){
		cout << "Parametros invalidos. \nPara criar o arquivo de indices, digite: <nome do exec> -c <caminho do arquivo de dados> <caminho aonde deseja criar o arquivo de indices> \nPara buscar um registro, digite: <nome do executavel> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <titulo do registro que deseja procurar>" << endl;
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
			cout << "Parametros invalidos. Digite: <nome do exec> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <titulo do registro que deseja procurar>" << endl;
			exit(1);
		}else{
			seek2(argv[2],argv[3], argv[4]);
		}
		
	}else{
		cout << "Parametros invalidos. \nPara criar o arquivo de indices, digite: <nome do exec> -c <caminho do arquivo de dados> <caminho aonde deseja criar o arquivo de indices> \nPara buscar um registro, digite: <nome do executavel> -b <caminho do arquivo de dados> <caminho do arquivo de indices> <titulo do registro que deseja procurar>" << endl;
		exit(1);
	}
}*/