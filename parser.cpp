/*
Parser
Alunos: Davi Ricardo
        Emidio Jose
        Enzo Henrique
*/

#include <iostream>     // cout, endl
#include "parser.hpp"

int colisoes=0;

//Passa o conteúdo de b para a via ponteiro. 
void copyStringToStr(char* a, string b, int tam) {
    strncpy(a, b.c_str(), tam);
    a[tam-1] = 0;
}

//Imprime os dados do artigo em questão.
void imprimirArtigo(Artigo art) {
    cout<< "\n*ID: "             << art.id 
        << "\n*Titulo: "         << art.titulo 
        << "\n*Ano: "            << art.ano
        << "\n*Autor: "          << art.autor
        << "\n*Citações:"        << art.citacoes
        << "\n*Atualização: "    << art.atualizacao
        << "\n*Snippet: "        << art.snippet 
        << endl << endl;
}

//Faz uma copia do artigo. 
void copiaArtigo(Artigo *destino, Artigo *fonte){
    destino->id = fonte->id;
    strcpy(destino->titulo,fonte->titulo);
    destino->ano = fonte->ano;
    strcpy(destino->autor,fonte->autor);
    destino->citacoes = fonte->citacoes;
    strcpy(destino->atualizacao,fonte->atualizacao);
    strcpy(destino->snippet,fonte->snippet);
}


void pause() {
    std::cout << "Press enter to continue ..."; 
    cin.get();
}

//Pega os campos do argumento passado. verifica se possui aspas, se há espaçamento ou puladas de linha.
string getCampo(ifstream *arq){
    string coluna;
    int  numAspas = 0;
    char anterior = 0, atual;
    bool acabou = false;

    
    while(!acabou) {
        if(!arq->get(atual)) {
            return "";
        }
        switch(atual) {
            case '"':
                numAspas++;
                break;
            case '\r':
                //Lê e ignora o próximo caracter, no caso '\n'
                arq->get(); 

            case ';':
                if(coluna == "NULL"){
                    return coluna;
                }
                else if (
                    //Se não tiver aspas delimitando o campo ou se for um caso de aspas aninhadas "" 
                    numAspas == 0 || 
                    (anterior=='"' && (numAspas&1)==0)//Verifica se numAspas é par    
                )
                {
                    acabou = true;
                }
                break;
            default:;
        }
        coluna +=  anterior = atual;
    }
    return coluna.substr (1,coluna.length()-3);
}

//Retorna um bucket do hashfile através da busca por ID. 
Bloco getBucketHashFileByID(fstream *arq, int id) {
    Bloco buffer={0};
    int i,j;

    //Encontrando a posição do bucket no hashfile
    int enderecoBucket = hashSimples(id);
    arq->seekg (enderecoBucket*sizeof(Bloco),ios::beg);

    //Leitura de Bloco
    arq->read((char*)&buffer,sizeof(Bloco));
    return buffer;
}

//Retorna um ponteiro do bucket do hashfile através da busca por posicão no hashfile. 
Bloco * getBucketHashFileByPosition(fstream *arq, int position) {
    Bloco *buffer= NULL;
    int i,j;

    buffer = (Bloco *)malloc(sizeof(Bloco));

    //Encontrando a posição do bucket no hashfile
    arq->seekg (position*sizeof(Bloco),ios::beg);

    //Leitura de Bloco
    arq->read((char*)buffer,sizeof(Bloco));
    return buffer;
}

//Pega o artigo no hashfile através da posição em que se encontra e verifica se o id do artigo e o id de entrada são iguais. 
Artigo* getArtigoByPositionID(fstream *arq, int position, int id){
    Bloco *bucket;
    Artigo *article, *result=NULL;
    int maior, menor, posicaoPonteiro;

    result = (Artigo *) malloc(sizeof(Artigo));

    bucket = getBucketHashFileByPosition(arq, position);
    article = (Artigo*)&bucket->corpo;

    menor=0;
    maior=bucket->numRegistros-1;

    while(menor <= maior){
        posicaoPonteiro = (menor + maior) / 2;

        if(id == article[posicaoPonteiro].id){
            copiaArtigo(result, &article[posicaoPonteiro]);
            break;
        }else if(id < article[posicaoPonteiro].id){
            maior = posicaoPonteiro - 1;
        }else{
            menor = posicaoPonteiro + 1;
        }
    }

    free(bucket);
    return result;
}

//Pega o artigo no arquivo hash através da posição em que se encontra e verifica se o título do artigo e o título de entrada são iguais. 
Artigo* getArtigoByPositionTitle(fstream *arq, int position, char title[300]){
    Bloco *bucket;
    Artigo *article, *result=NULL;

    result = (Artigo *) malloc(sizeof(Artigo));

    bucket = getBucketHashFileByPosition(arq, position);
    article = (Artigo*)&bucket->corpo;

    for(int i=0; i < bucket->numRegistros; i++){
        if(strcmp(title, article[i].titulo) == 0){
            copiaArtigo(result, &article[i]);
            break;
        }
    }
    
    free(bucket);
    return result;
}

//Retorna o artigo do arquivo
Artigo getArtigo(ifstream *arq) {
    string aux;
    Artigo artigo = {0};

    for(int estado = 1; estado <= 7; estado++) {
        aux = getCampo(arq);

        if(aux=="NULL") {
            break;
        }

        switch(estado) {
            case (1):
                artigo.id=atoi(aux.c_str());
                break;
            case (2):
                copyStringToStr(artigo.titulo,aux,300); 
                break;
            case (3):
                artigo.ano =atoi(aux.c_str());
                break;
            case (4):
                copyStringToStr(artigo.autor,aux,100);
                break;
            case (5):
                artigo.citacoes = atoi(aux.c_str());
                break;
            case (6):
                copyStringToStr(artigo.atualizacao,aux,20);
                break;
            case (7):
                copyStringToStr(artigo.snippet,aux,100);
                break;
        }
    }

    return artigo;
}

//Retorna o bucket que possui o id de entrada.
int hashSimples(int id) {
    return id%NUM_BUCKETS;
}

//Insere no arquivo hash o ponteiro que identifica tal artigo
int inserirNoHashFile(fstream *arq, Artigo artigo) {

    int i,j;

    Bloco buffer= getBucketHashFileByID(arq,artigo.id);

    //Interpretar o corpo do bloco como um vetor de artigos, o tamanho do vetor é dado por buffer.numRegistros (cabeçalho)
    Artigo *vet;
    vet=(Artigo*)&buffer.corpo;

    //Verificando se há espaço no bloco
    if(buffer.numRegistros < FATOR_BLOCO) {


        for(i=0; i<buffer.numRegistros; i++) {
            //imprimirArtigo(artigo);
            if(artigo.id < vet[i].id) {
                
                //Deslocar todos os artigos
                for(j=buffer.numRegistros; j>i; j--) {

                    //Cada artigo recebe o que estava no anterior
                    memcpy(&vet[j],(char*)&vet[i-1], sizeof(Artigo));
                }
                break;

            }
            /*
            else if (artigo.id == vet[i].id) {
                //Artigo já está no hash
                return;
            }
            */
        }

        //Insere o artigo em sua devida posição
        memcpy(&vet[i],(char*)&artigo, sizeof(Artigo));

        //Atualizar o número de registros
        buffer.numRegistros=buffer.numRegistros+1;

        //Volta o cursor para o início do bloco atual, para sobrescrevê-lo            
        arq->seekp( - sizeof(Bloco) , ios::cur);

        //Escreve o bloco no arquivo
        arq->write((char*)&buffer,sizeof(Bloco));

        return 1;
    }
    else {
        //Numero de Buckets insuficientes
        colisoes++;
        return -1;
        //cout<< "Houve colisão ID: "<<contRegistro<<endl;
    }
}

// Busca no arquivo de dados por um registro com o ID informado, se existir, e retornar os campos do registro, a quantidade de blocos lidos para encontrá-lo e a quantidade total de bloco do arquivo de dados;
Artigo findrec(fstream *arq,int id, bool imprimir) {
    int contBlocosLidos =0;

    Bloco buffer= getBucketHashFileByID(arq,id);

    contBlocosLidos++;

    //Interpretar o corpo do bloco como um vetor de artigos, o tamanho do vetor é dado por buffer.numRegistros (cabeçalho)
    Artigo *vet;
    vet=(Artigo*)&buffer.corpo;

    //Verificar se o registro já foi inserido
    for(int i=0; i<buffer.numRegistros; i++) {
        //imprimirArtigo(vet[i]);
        if(id == vet[i].id) {
            if(imprimir) {
                imprimirArtigo(vet[i]);
                cout<< "------------------------------------------------\nBlocos lidos: "<<contBlocosLidos<<" Total de Blocos: "<<NUM_BUCKETS<<endl;
            }
            return vet[i];
        }
    }

    cout << "Não encontrado.\n";
}
//Retorna as colisões no arquivo hash.
int getColisoes() {
    return colisoes;
}