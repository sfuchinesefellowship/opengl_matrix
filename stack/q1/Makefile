GCC_OPTIONS=-Wall -pedantic -I ../include
GL_OPTIONS=-lGL -lglut -lGLEW 
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)


myrobot: 
	g++ $@.cpp ../Common/initShader.o $(OPTIONS) -o $@  

