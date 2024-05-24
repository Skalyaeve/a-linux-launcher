NAME=launcher

CC=gcc
CFLAGS=-Wall -Wextra -Werror -g
LDFLAGS=-lX11

INCLUDE_DIR=include
HEADER_EXT=h
HEADER=$(shell find $(INCLUDE_DIR) -type f -name *.$(HEADER_EXT))

SRC_EXT=c
SRC_DIR=src
SRC=$(shell find $(SRC_DIR) -type f -name *.$(SRC_EXT))

OBJ_DIR=obj
OBJ=$(subst $(SRC_DIR),$(OBJ_DIR),$(SRC:.c=.o))

all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LDFLAGS)

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT) $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $(<:.$(SRC_EXT)=.o)
	@mv $(SRC_DIR)/*.o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

install:
	@mkdir $(HOME)/.config/$(NAME)
	@cp config $(HOME)/.config/$(NAME)/config
	@ln -sf $(PWD)/$(NAME) $(HOME)/.local/bin/$(NAME)
	@echo $(NAME) installed

uninstall:
	@rm -rf $(HOME)/.config/$(NAME)
	@rm -f $(HOME)/.local/bin/$(NAME)
	@echo $(NAME) uninstalled

.PHONY: all clean fclean re install uninstall
