build:
	clang++ dtypes.cpp otree/otree.cpp otree/otree_ray.cpp main.cpp vox_render.cpp render_shader.cpp -lraylib -o ray.out
run :
	make build
	./ray.out

