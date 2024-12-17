default:
	g++ experiments/uhr_salcp.cpp -o uhr_salcp -std=c++20 -O0 -Wall -Wpedantic
	./uhr_salcp results_salcp.csv 128 1 4 1
	g++ experiments/uhr_sasdsl.cpp -o uhr_sasdsl -std=c++20 -O0 -Wall -Wpedantic -lsdsl -ldivsufsort -ldivsufsort64
	./uhr_sasdsl results_sasdsl.csv 128 1 4 1
	g++ experiments/uhr_fmindex.cpp -o uhr_fmindex -std=c++20 -O0 -Wall -Wpedantic -lsdsl -ldivsufsort -ldivsufsort64
	./uhr_fmindex results_fmindex.csv 128 1 4 1
