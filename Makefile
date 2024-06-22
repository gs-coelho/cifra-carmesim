# MAKEFILE
# Gabriel Coelho dos Santos


# Impede o make de usar regras implícitas, que mudem o comportamento esperado
MAKEFLAGS += --no-builtin-rules

# Compilador utilizado, flags de compilação e nome do programa principal
COMPILADOR := g++
FLAGS := -Wall -g -lm
PROGRAMA := bin/main

# Extensões de arquivo
BIN_EXT := out
OBJ_EXT := o
SRC_EXT := cpp
INCLUDE_EXT = hpp

# Variáveis com os caminhos das pastas. Serão utilizadas para construir os
# nomes dos arquivos nos comandos de compilação, pois o vpath não funciona para
# os comandos e pré-requisitos, apenas os alvos.
BIN_DIR := ./bin
OBJ_DIR := ./obj
SRC_DIR := ./src
INCLUDE_DIR := ./include

# Acha os nomes dos objetos (*.o) que são pré-requisito para o programa
# principal, e os armazena em OBJS.
#
# A palavra-chave "wildcard" é uma função de texto, que encontra todos os nomes
# de arquivo que se encaixam no padrão especificado e os coloca em uma única
# string.
#
# O ":" é uma função de texto, que procura todas as ocorrências de uma
# substring e a substitui por outro valor.
SRC_DIRS := $(shell find src -type d)
SRC_PATHS := $(patsubst %, %/*.$(SRC_EXT) , $(SRC_DIRS))
SRC_PATHS := $(wildcard $(SRC_PATHS))
OBJS := $(patsubst src/%.$(SRC_EXT), obj/%.$(OBJ_EXT), $(SRC_PATHS))

INCLUDE_DIRS := $(shell find $(INCLUDE_DIR) -type d)
INCLUDE_PATHS := $(patsubst %, %/*.$(INCLUDE_EXT), $(INCLUDE_DIRS))
INCLUDES := $(wildcard $(INCLUDE_PATHS))

# Especifica que os alvos abaixo não são arquivos, mas estão
# sendo usados para nomear uma rotina de compilação.
.PHONY: all clean run memlog

all: $(PROGRAMA)

clean:
	@rm -rf gmon.out $(PROGRAMA) $(BIN_DIR)/* $(OBJ_DIR)/*

run: $(PROGRAMA)
	@$(PROGRAMA) -h


# ---------------------------------- BINÁRIOS ----------------------------------

# Caso especial da main, que depende de todos os arquivos objeto
$(PROGRAMA): $(OBJS)
#   Cria o diretório onde o arquivo gerado ficará, caso não exista.
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(COMPILADOR) $(FLAGS) -o $@ -I $(INCLUDE_DIR) $(OBJS)

# Compila os arquivos binários, onde cada um depende apenas do seu arquivo
# objeto
$(BIN_DIR)/%.$(BIN_EXT): $(OBJ_DIR)/%.$(OBJ_EXT)
#   Cria o diretório onde o arquivo gerado ficará, caso não exista.
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(COMPILADOR) $(FLAGS) -o $@ -I $(INCLUDE_DIR) $<


# ------------------------------ ARQUIVOS OBJETO -------------------------------

# Descomentar e modificar caso precise de uma regra mais específica para o
# arquivo objeto da main:
#
# $(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(INCLUDE)
# 	@$(COMPILADOR) $< -c -o $@ -I $(INCLUDE_DIR)


# Compila os arquivos objeto, onde cada um depende apenas do seu arquivo de
# código fonte, e cada arquivo de código fonte depende de TODOS os arquivos de
# cabeçalho (o makefile não consegue saber as dependências de cada arquivo de
# código fonte)
$(OBJ_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.$(SRC_EXT) $(INCLUDES)
#   Cria o diretório onde o arquivo gerado ficará, caso não exista.
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(COMPILADOR) $(FLAGS) $< -c -o $@ -I $(INCLUDE_DIR)