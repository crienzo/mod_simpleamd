BASE=../../../..
LOCAL_OBJS= \
	simpleamd/amd.o \
	simpleamd/logger.o \
	simpleamd/vad.o
LOCAL_SOURCES=	\
	simpleamd/amd.c \
	simpleamd/logger.c \
	simpleamd/vad.c
include $(BASE)/build/modmake.rules

