
libmmqueue.a : mmqueue.cpp
	g++ -c mmqueue.cpp --std=c++11 -g3 -rdynamic -lrt
	g++ -c base/*.cpp --std=c++11 -g3 -rdynamic -lrt 
	ar r libmmqueue.a *.o

test: libmmqueue.a test.cpp
	g++ test.cpp -L. -lmmqueue --std=c++11 -o test -g3 -pthread -rdynamic

clean:
	rm -f *.o *.a test




