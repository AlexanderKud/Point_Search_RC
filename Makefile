default:
	g++ -m64 -mssse3 -O3 -c Ec.cpp -o Ec.o
	g++ -m64 -mssse3 -O3 -c utils.cpp -o utils.o
	g++ -m64 -mssse3 -O3 -c generate_bloom.cpp -o generate_bloom.o
	g++ -m64 -mssse3 -O3 -c point_search.cpp -o point_search.o
	g++ -march=native -O3 -o generate_bloom generate_bloom.o Ec.o utils.o
	g++ -march=native -O3 -o point_search point_search.o Ec.o utils.o
	rm *.o
