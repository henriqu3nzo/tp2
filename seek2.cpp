#include <iostream>
#include <stdlib.h>
#include <string>

#include "arvoreBMaisSecundaria.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]){
	if(argc != 2){
		cout << "Informa apenas o titulo que deseja procurar na arvore, no formato: ./seek2 <titulo>" <<endl;
		return 1;
	}else{
		seek2(CAMINHOSHASH, INDICESECUNDARIO, argv[1]);
	}
}