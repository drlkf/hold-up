#!/usr/bin/env make -f

CC	:= g++
CFLAGS	:= -Wall -Wextra -Werror
LFLAGS	:= -Wall -Werror

RM	:= rm -rf

TARGET	:= hold-up
SRCDIR	:= src
OBJDIR	:= .obj
TGTDIR	:= bin

SRC	:= $(wildcard $(SRCDIR)/*.cpp)
OBJ	:= $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)


all: $(TARGET)

$(OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	$(RM) $(OBJDIR) $(TGTDIR)

.PHONY: re
re: clean all
