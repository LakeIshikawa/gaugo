# gauCore library
lib_LIBRARIES = libgauGoCore.a
libgauGoCore_a_SOURCES = board.c board_zobrist.c hashTable.c uctSearch.c \
	policy_pureRandom.c stoppers.c crash.c memoryPool.c uctTree.c timer.c \
	stonegroup.c p3x3info.c stack.c

nodist_libgauGoCore_a_SOURCES = p3x3info.c
BUILT_SOURCES = p3x3info.c

p3x3info.c : gauHbuilder
	./gauHbuilder --build 3x3info -o p3x3info.c

# gauCore library public headers
include_HEADERS = board.h uctSearch.h hashTable.h crash.h policies.h

#programs
bin_PROGRAMS = gauGo gauGo2p gauHbuilder

AM_CPPFLAGS = -I$(top_srcdir)/src/gnugo/sgf

#gauGo GTP engine
gauGo_SOURCES = gauGoMain.c options.c gauGoEngine.c GTPBasicCommands.c \
	GTPArchiving.c GTPGogui.c GTPPatterns.c GTPBench.c
gauGo_LDADD = libgauGoCore.a $(top_srcdir)/build/src/gnugo/sgf/libsgf.a -lm

# gauGo 2-players
gauGo2p_SOURCES = gauGo2p.c
gauGo2p_LDADD = libgauGoCore.a

#header builder
gauHbuilder_SOURCES = gauHbuilder.c build3x3info.c
