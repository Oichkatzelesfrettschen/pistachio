CXXFLAGS ?= -std=c++23
INCLUDES = -I../../user/include -I../../user/contrib/elf-loader/include

LIB = libuseripc.a
OBJS = user_ipc.o

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $(OBJS)

user_ipc.o: ipc/user_ipc.cc ../../user/include/l4/ipc_user.h
	g++ $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB)

.PHONY: all clean
