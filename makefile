// Compila

all: clear compila
compila: upload findrec seek1 seek2
upload: upload.cpp arvoreBMaisPrimaria.cpp arvoreBMaisSecundaria.cpp parser.cpp
	g++ upload.cpp arvoreBMaisPrimaria.cpp arvoreBMaisSecundaria.cpp parser.cpp -o upload
findrec: findrec.cpp parser.cpp
	g++ findrec.cpp parser.cpp -o findrec
seek1: seek1.cpp arvoreBMaisPrimaria.cpp parser.cpp
	g++ seek1.cpp arvoreBMaisPrimaria.cpp parser.cpp -o seek1
seek2:seek2.cpp arvoreBMaisSecundaria.cpp parser.cpp
	g++ seek2.cpp arvoreBMaisSecundaria.cpp parser.cpp -o seek2
clear:
	rm -rf upload
	rm -rf findrec
	rm -rf seek1
	rm -rf seek2

// Exemplos
Ler o arquivo:
$ g++ upload.cpp arvoreBMaisPrimaria.cpp arvoreBMaisSecundaria.cpp parser.cpp -o upload
$ ./upload artigo.csv

Buscar pelo id no arquivo:
$ g++ findrec.cpp parser.cpp -o findrec
$ ./findrec 50

Seek pelo id:
$ g++ seek1.cpp arvoreBMaisPrimaria.cpp parser.cpp -o seek1
$ ./seek1 50