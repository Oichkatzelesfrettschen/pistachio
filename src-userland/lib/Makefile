CXXFLAGS ?= -std=c++23
INCLUDES = -I../../user/include -I../../user/contrib/elf-loader/include

LIB = libuseripc.a
LIB2 = libsched.a
OBJS = user_ipc.o
OBJS2 = sched/sched_client.o

all: $(LIB) $(LIB2)

$(LIB): $(OBJS)
	ar rcs $@ $(OBJS)

$(LIB2): $(OBJS2)
	ar rcs $@ $(OBJS2)

user_ipc.o: ipc/user_ipc.cc ../../user/include/l4/ipc_user.h
	g++ $(CXXFLAGS) $(INCLUDES) -c $< -o $@

sched/sched_client.o: sched/sched_client.cc sched/sched_client.h
	g++ $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS2) $(LIB) $(LIB2)

.PHONY: all clean
