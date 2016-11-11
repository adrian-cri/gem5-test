UTIL=../../util
INST=${UTIL}/m5/m5op_x86.S
FLAGS=-g -O2
NAME=foo

all: foo

foo: $(NAME).c
	gcc -I $(UTIL) $(INST) $(NAME).c -o $(NAME)$(NN) $(FLAGS) -DUNIX --static -DN=$(NN)

