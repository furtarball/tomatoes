# "I Have No Tomatoes" Makefile (GNU/Linux)
# Run 'make DEBUGMODE=1' if you need to debug the game.
# Or run 'make PROFILE=1' if you need to profile the game.

.PHONY: all clean veryclean rebuild compress

CC = g++
COMPRESS = upx --best
TARGET = ./tomatoes
MARCH = x86-64


# Directory defines, you can use these defaults or adjust them if
# necessary. Remember to include the trailing /

# MPK directory (where 'tomatoes.mpk' is), default: ./
MPKDIR = ./

# Music directory (where the music files are), default: ./music/
MUSICDIR = ./music/

# Hiscore directory (where the hiscores are written to), default: ./
# We need read/write access!
HISCOREDIR = ./

# Config directory (where the 'config.cfg' is), default: ./
# We need read/write access!
CONFIGDIR = ./

# Override directory (unused at the moment), default: ./data/
OVERRIDEDIR = ./data/


DIR_DEFINES = -DLINUX -DMPK_DIR=\"$(MPKDIR)\" -DMUSIC_DIR=\"$(MUSICDIR)\" -DHISCORE_DIR=\"$(HISCOREDIR)\" -DCONFIG_DIR=\"$(CONFIGDIR)\" -DOVERRIDE_DIR=\"$(OVERRIDEDIR)\"


# SDL flags
SDL_FLAGS = `sdl2-config --cflags`


# Debugmode stuff
ifdef DEBUGMODE
CFLAGS = -MMD -g3 -W -Wall -mcpu=$(MARCH) -DDEBUGMODE
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lGL -lGLU
else
ifdef PROFILE
CFLAGS = -MMD -g3 -O3 -march=$(MARCH) -Wall -pg
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lGL -lGLU -pg
else
CFLAGS = -MMD -O3 -march=$(MARCH) -Wall $(SDL_FLAGS)
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lGL -lGLU -s
endif
endif

# Source and object files
SOURCES = $(wildcard src/*.cpp)
OBJS = $(SOURCES:.cpp=.o)
OBJS := $(subst src/,obj/,$(OBJS))

# Include directories
INCLUDES = -I./include


# Targets
all: $(TARGET)

# Check dependancies
DEPS = $(subst .o,.d,$(OBJS))
-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

veryclean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

rebuild: veryclean all

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DIR_DEFINES) -c $< -o $@

# Compress the exe with UPX
compress: $(TARGET)
	$(COMPRESS) $(TARGET)
