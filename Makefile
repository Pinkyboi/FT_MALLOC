
ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc_$(HOSTTYPE).so

SYMLINK = libft_malloc.so

CC = gcc

SRC_PATH = ./src/
OBJ_PATH = ./obj/
INC_PATH = ./inc/

SRC_NAME =	calloc.c\
			free.c\
			log.c\
			malloc.c\
			realloc.c\
			reallocf.c\
			utils.c

DEPS = $(INC_PATH)/malloc.h

OBJ_NAME = $(SRC_NAME:.c=.o)

SRC = $(addprefix $(SRC_PATH),$(SRC_NAME))
OBJ = $(addprefix $(OBJ_PATH),$(OBJ_NAME))

FLAGS = -Wall -Wextra -Werror -fPIC  -I $(INC_PATH)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(FLAGS) -shared $^ -o $@
	@ln -sf $(NAME) $(SYMLINK)
	@echo "\033[32m[OK]\033[0m \033[33mLibft_malloc compiled\033[0m"

$(OBJ_PATH)%.o: $(SRC_PATH)%.c $(DEPS)
	@mkdir -p $(OBJ_PATH)
	@$(CC) $(FLAGS) -I $(INC_PATH) -o $@ -c $<

clean:
	@rm -rf $(OBJ_PATH)
	@echo "\033[32m[OK]\033[0m \033[33mLibft_malloc cleaned\033[0m"

fclean: clean
	@rm -f $(NAME)
	@rm -f $(SYMLINK)
	@echo "\033[32m[OK]\033[0m \033[33mLibft_malloc fcleaned\033[0m"

re: fclean all