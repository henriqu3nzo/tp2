#include <iostream>     // cout, endl
#include <stdio.h>

#include "parser.hpp"
#include "arvoreBMaisPrimaria.hpp"
#include "arvoreBMaisSecundaria.hpp"

int contRegistro=0;

int main(int argc, char *argv[]){
    if(argc<2) {
        cout << "Digite o nome do arquivo de entrada.\nEx: upload <file>\n";
        return 1;
    }

    remove(CAMINHOSHASH);
    remove(INDICEPRIMARIO);
    remove(INDICESECUNDARIO);

    ifstream *entrada = new ifstream(argv[1]);
    fstream *arqDados = new fstream(CAMINHOSHASH,fstream::in|fstream::out|fstream::trunc|ios::binary);

    Bloco buffer={0};
    
    cout << "Alocando arquivo de dados ("<<CAMINHOSHASH<<")."<<endl;

    //Preenchendo todos os buckets com blocos vazios
    for(int i=0;i<NUM_BUCKETS; i++) {
        arqDados->write((char*)&buffer,sizeof(Bloco));
    }

    cout << "Abrindo arquivo de entrada." << endl;

    if (entrada->is_open()) {
        
        cout << "Inserindo registros no arquivo de dados." << endl;

        while(!entrada->eof()) {

            Artigo artigo = getArtigo(entrada);

            if(!entrada->eof()) {
                if(inserirNoHashFile(arqDados,artigo) > 0){
                    contRegistro++;
                }
            }
        }

        entrada->close();
        arqDados->close();

        cout << "Arquivo de dados ordenado por hash criado com "<<contRegistro<<" registros."<<endl;

        uploadBMaisPrimaria(CAMINHOSHASH, INDICEPRIMARIO);

        uploadBMaisSecundaria(CAMINHOSHASH, INDICESECUNDARIO);
        
        //cout <<"Colisões: "<<getColisoes()<<" ContRegistro: "<<contRegistro<<endl;
    }
    else cout << "Erro ao abrir o arquivo.\n";

    if(entrada->is_open()){
        entrada->close();
    }

    if(arqDados->is_open()){
        arqDados->close();
    }
    
    return 0;
}