all:
	cython --cplus stan.pyx
	python setup.py build_ext --inplace

lib:
	g++ -I../src -I../lib/eigen_3.1.3 -I../lib/boost_1.53.0  -c  -Wall -fpic eight.cpp
	gcc -shared -o libeight.so eight.o
