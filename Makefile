build:
	clang++ dtypes.cpp otree.cpp otree_ray.cpp main.cpp vox_render.cpp -lraylib -o ray.out
run :
	make build
	./ray.out

