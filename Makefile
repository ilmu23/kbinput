## ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
## ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
## █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
## ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
## ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
## ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
##
## <<Makefile>>

NAME	=	libkbinput.so

BUILD	=	fsan

CC				=	gcc
cflags.common	=	-Wall -Wextra -Werror -Wpedantic -pedantic-errors -std=gnu2x -fpic -I$(INCDIR)
cflags.debug	=	-g
cflags.fsan		=	$(cflags.debug) -fsanitize=address,undefined
cflags.normal	=	-s -O1
cflags.extra	=	
CFLAGS			=	$(cflags.common) $(cflags.$(BUILD)) $(cflags.extra)

LD		=	ld
LDFLAGS	=	-shared

SRCDIR	=	src
OBJDIR	=	obj
INCDIR	=	inc

UTILDIR	=	utils

FILES	=	init.c \
			listener.c \
			$(UTILDIR)/vector.c

SRCS	=	$(addprefix $(SRCDIR)/, $(FILES))
OBJS	=	$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@printf "\e[38;5;119;1mKBINPUT >\e[m Linking %s\n" $@
	@$(LD) $(LDFLAGS) $(OBJS) -o $@
	@printf "\e[38;5;119;1mKBINPUT >\e[m \e[1mDone!\e[m\n"

$(OBJDIR):
	@printf "\e[38;5;119;1mKBINPUT >\e[m Creating objdirs\n"
	@mkdir -p $(OBJDIR)/$(UTILDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "\e[38;5;119;1mKBINPUT >\e[m Compiling %s\n" $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -rf $(OBJDIR)
	@rm -f $(NAME)

re: fclean all

db:
	@printf "\e[38;5;119;1mKBINPUT >\e[m Creating compilation command database\n"
	@compiledb make --no-print-directory BUILD=$(BUILD) cflags.extra=$(cflags.extra) | sed -E '/^##.*\.\.\.$$|^[[:space:]]*$$/d'
	@printf "\e[38;5;119;1mKBINPUT >\e[m \e[1mDone!\e[m\n"

.PHONY: all clean fclean re db
