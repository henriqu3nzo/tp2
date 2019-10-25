#include <iostream>     // cout, endl
#include "parser.hpp"

int main(int argc, char *argv[])
{
    if(argc<2) {
        cout << "Digite o ID do registro a ser procurado.\nEx: findrec <ID>\n";
        return 1;
    }

    fstream *entrada = new fstream(CAMINHOSHASH,fstream::in|ios::binary);

    if (entrada->is_open()) {
        findrec(entrada,atoi(argv[1]),true);   
    }
    else cout << "Erro ao abrir o arquivo.\n";
    return 0;
}