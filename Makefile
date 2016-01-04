
libmmqueue.a : mmqueue.cpp
	g++ -c mmqueue.cpp --std=c++11 -O2
	g++ -c base/*.cpp --std=c++11 -O2
	ar r libmmqueue.a *.o

test: libmmqueue.a test.cpp
	g++ test.cpp -L. -lmmqueue --std=c++11 -o test

clean:
	rm -f *.o *.a test 




