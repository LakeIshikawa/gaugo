# gauCore library
lib_LIBRARIES = libgauGoCore.a
libgauGoCore_a_SOURCES = board.c board_zobrist.c hashTable.c uctSearch.c \
	policy_pureRandom.c stoppers.c crash.c memoryPool.c uctTree.c timer.c

# gauCore library public headers
include_HEADERS = board.h uctSearch.h hashTable.h crash.h

#programs
bin_PROGRAMS = gauGo gauGo2p

AM_CPPFLAGS = -I$(top_srcdir)/src/gnugo/sgf

#gauGo GTP engine
gauGo_SOURCES = gauGoMain.c options.c gauGoEngine.c GTPBasicCommands.c \
	GTPArchiving.c GTPGogui.c
gauGo_LDADD = libgauGoCore.a $(top_srcdir)/build/src/gnugo/sgf/libsgf.a -lm

# gauGo 2-players
gauGo2p_SOURCES = gauGo2p.c
gauGo2p_LDADD = libgauGoCore.a
