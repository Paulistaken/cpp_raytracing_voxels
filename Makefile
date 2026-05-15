build:
	zig c++ dtypes.cpp otree/otree.cpp otree/otree_ray.cpp main.cpp game_data.cpp vox_render.cpp render_shader.cpp -lraylib -o ray.out
	# clang++ dtypes.cpp otree/otree.cpp otree/otree_ray.cpp main.cpp game_data.cpp vox_render.cpp render_shader.cpp -lraylib -o ray.out
run :
	make build
	./ray.out

