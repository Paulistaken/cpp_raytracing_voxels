build:
	clang++ dtypes.cpp qtree.cpp otree.cpp main.cpp -lraylib -o ray.out
run :
	make build
	./ray.out

