#include <iostream>
#include "MorphingWithLocalWarping.h"

int main() {
	int frame = 11;
	MorphingWithLocalWarping test("image\\1.bmp", "point\\1.txt", "image\\2.bmp", "point\\2.txt", frame);
	test.StartMorphing();
}  