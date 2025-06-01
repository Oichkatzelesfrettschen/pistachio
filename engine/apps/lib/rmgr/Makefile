include../../
    Makeconf $(wildcard.depend)

        LIB =../ librmgr.a

                 SRCS = librmgr.cpp OBJS = $(patsubst %.S, %.o,
                                             $(patsubst %.cpp, %.o, $(SRCS)))

                 INCLUDES =../../ rmgr /
                               include

                               all : $(LIB)

                                         $(LIB)
    : $(OBJS) $(AR) cvrs $ @$ ^
