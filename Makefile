# # **************************************************************************** #
# #                                                                              #
# #                                                         :::      ::::::::    #
# #    Makefile                                           :+:      :+:    :+:    #
# #                                                     +:+ +:+         +:+      #
# #    By: max <max@student.42.fr>                    +#+  +:+       +#+         #
# #                                                 +#+#+#+#+#+   +#+            #
# #    Created: 2023/09/05 21:27:42 by max               #+#    #+#              #
# #    Updated: 2023/09/05 22:17:13 by max              ###   ########.fr        #
# #                                                                              #
# # **************************************************************************** #

##-----ALIAS-----##

NAME 	= 	webserv
CC 		= 	g++
FLAGS 	= 	-Wall -Wextra -Werror -std=c++98
RM		= 	rm -rf
MKDIR	= 	mkdir -p
AR		=	ar rcs

##-----PATHS-----##

PROJECT_DIR		= 	src/
HEADERS_DIR 	= 	Includes/

DIRS = $(OBJS_DIR) \
       $(OBJS_DIR)src/ \
       $(OBJS_DIR)src/Config/ \
       $(OBJS_DIR)src/Servers/ \
       $(OBJS_DIR)src/Errors/ \
       $(OBJS_DIR)src/Cgi/ \
       $(OBJS_DIR)src/Request/ \
       $(OBJS_DIR)src/Response/ \
       $(OBJS_DIR)src/Utils/ \
       $(OBJS_DIR)src/Sockets/

OBJS_DIR 		= 	obj/

##-----FILES-----##


SRC_HEADER      =  	ServerConfig.hpp \
					WebServer.hpp \
					CgiHandler.hpp \
					Colors.hpp \
					MasterSocket.hpp \
					Response.hpp \
					ResponseHeader.hpp

SRC_FILES       =   main.cpp \
					ServerConfig.cpp \
					Request.cpp \
					Response/CgiHandler.cpp \
					Response/Response.cpp \
					Response/ResponseHeader.cpp \
					WebServer.cpp \
					MasterSocket.cpp \
					Utils.cpp

SRCS			=	$(addprefix $(PROJECT_DIR),$(SRC_FILES))


##-----SOURCES-----##

OBJ				=	$(SRCS:.cpp=.o)
OBJS			=	$(addprefix $(OBJS_DIR),$(OBJ))
HEADERS			=	$(addprefix $(HEADERS_DIR),$(SRC_HEADER))


##-----COULEURS-----##

GREEN			=		\033[1;32m
BG_GREEN		=		\033[42m
BLUE			=		\033[0;94m
RED				=		\033[1;31m
GREY			=		\033[0;37m
ENDCOLOR		=		\033[0m

##-----TEXTES-----##

START			=		echo "$(GREEN)Compilation of $(NAME) started\n$(ENDCOLOR)"
TEST			=		echo "$(GREY)Running some test\n$(ENDCOLOR)"
END_COMP		=		echo "$(GREEN)Compilation is done$(ENDCOLOR)"
CLEAN_TXT		=		echo "$(RED)Deleting objects$(ENDCOLOR)"
FCLEAN_TXT		=		echo "$(RED)Deleting program$(ENDCOLOR)"
CHARG_LINE		=		echo "$(BG_GREEN)    $(ENDCOLOR)\c"
BS_N			=		echo "\n"

TOTAL_FILES		=		$(words $(SRCS))
COMPILED_FILE	=		0
MESSAGE			=		"Compilation en cours : $(COMPILED_FILES)/$(TOTAL_FILES) ($(shell expr $(COMPILED_FILES) \* 100 / $(TOTAL_FILES))%)"



$(OBJS_DIR)$(PROJECT_DIR)%.o : $(PROJECT_DIR)%.cpp $(HEADERS)
	@mkdir -p $(OBJS_DIR)$(PROJECT_DIR)
	@mkdir -p $(DIRS)
	@$(CC) $(FLAGS) -o $(@) -c $(<)
	$(eval COMPILED_FILES=$(shell echo $$(($(COMPILED_FILES)+1))))
	@printf "$(GREEN)%s$(ENDCOLOR)" $(MESSAGE)
	@sleep 0.001
	@printf "\r"


##-----RULES-----##

all: logo start $(NAME)

start:
			@tput setaf 2; cat ascii_art/webserv; tput setaf default
			@$(BS_N)
			@$(START)

logo :
			@tput setaf 2; cat ascii_art/hibou; tput setaf default

$(NAME):	$(OBJS)
	@$(CC) $(FLAGS) $(^) -o $(@)
	@$(END_COMP)
	@tput setaf 2; cat ascii_art/small_hibou1; tput setaf default

clean:
		@$(CLEAN_TXT)
		@$(RM)  $(OBJS)
		@$(RM) $(OBJS_DIR)
		@tput setaf 1; cat ascii_art/trash; tput setaf default

fclean: clean
		@$(FCLEAN_TXT)
		@$(RM) $(NAME)
		@echo "$(GREEN)Cleaning succes$(ENDCOLOR)"

re:		fclean all


###########
# TESTING #
###########

test_setup: all
	@rm -rf test_us/root
	@mkdir -p test_us/root
	@cp test_us/index/* test_us/root/
	@cp test_us/root/index.html test_us/root/index_permission.html
	@chmod 000 test_us/root/index_permission.html
	@clang++ -o client test_us/client.cpp

test: test_setup
	@osascript -e 'tell application "Terminal" to do script "cd $(PWD) && clear && ./client"'
	@osascript -e 'tell application "Terminal" to activate'
	./webserv test_us/conf/webserv.conf

bocal: all
	@mkdir -p YoupiBanane/put_here
	@osascript -e 'tell application "Terminal" to do script "cd $(PWD) && clear && time ./test_mac/macos_tester http://localhost:8000"'
	@osascript -e 'tell application "Terminal" to activate'
	./webserv test_mac/mac.conf

.PHONY: libft clean fclean re test test_setup bocal