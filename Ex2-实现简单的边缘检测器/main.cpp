#include "Canny.h"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {

	Canny lena;
	lena.canny("lena.bmp");

	Canny twowos;
	twowos.canny("twows.bmp");

	Canny bigben;
	bigben.canny("bigben.bmp");

	Canny stpietro;
	stpietro.canny("stpietro.bmp");
	system("pause");

}