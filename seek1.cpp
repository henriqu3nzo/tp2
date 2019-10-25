#include <iostream>
#include <stdlib.h>
#include <string>

#include "arvoreBMaisPrimaria.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]){
	if(argc != 2){
		cout << "Informa apenas o id que deseja procurar na arvore, no formato: ./seek1 <ID>" <<endl;
		return 1;
	}else{
		seek1(CAMINHOSHASH, INDICEPRIMARIO, atoi(argv[1]));
	}
}