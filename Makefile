PLATFORM ?= PLATFORM_DESKTOP
BUILD_MODE ?= DEBUG
DEFINES = -D _DEFAULT_SOURCE -D RAYLIB_BUILD_MODE=$(BUILD_MODE) -D $(PLATFORM)
PLATFORM_OS ?= $(shell uname)

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    
    CC = g++
    
    ifeq ($(findstring MINGW,$(PLATFORM_OS)),MINGW)
        EXT = .exe
        RAYLIB_DIR = C:/raylib
        INCLUDE_DIR = -I ./ -I $(RAYLIB_DIR)/raylib/src -I $(RAYLIB_DIR)/raygui/src
        LIBRARY_DIR = -L $(RAYLIB_DIR)/raylib/src
        ifeq ($(BUILD_MODE),RELEASE)
            CFLAGS ?= $(DEFINES) -Wall -mwindows -D NDEBUG -O3 $(INCLUDE_DIR) $(LIBRARY_DIR) 
        else
            CFLAGS ?= $(DEFINES) -Wall -g $(INCLUDE_DIR) $(LIBRARY_DIR)
        endif
        LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    endif

    ifeq ($(findstring Linux,$(PLATFORM_OS)),Linux)
        EXT=
        RAYLIB_DIR = ~/raylib
        INCLUDE_DIR = -I ./ -I $(RAYLIB_DIR)/raylib/src -I $(RAYLIB_DIR)/raygui/src
        LIBRARY_DIR = -L $(RAYLIB_DIR)/raylib/src
        ifeq ($(BUILD_MODE),RELEASE)
            CFLAGS ?= $(DEFINES) -Wall -Wno-format-truncation -D NDEBUG -O3 $(INCLUDE_DIR) $(LIBRARY_DIR)
        else
            CFLAGS ?= $(DEFINES) -Wall -Wno-format-truncation -g $(INCLUDE_DIR) $(LIBRARY_DIR)
        endif
        LIBS = -lraylib -lGL -lm -ldl -lpthread
    endif
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    CC = emcc
    RAYLIB_DIR = C:/raylib
    INCLUDE_DIR = -I ./ -I $(RAYLIB_DIR)/raylib/src -I $(RAYLIB_DIR)/raygui/src
    EXT = .html
    CFLAGS ?= $(DEFINES) $(RAYLIB_DIR)/raylib/src/libraylib.web.a -v -D NDEBUG -O1 -s USE_GLFW=3 -s FORCE_FILESYSTEM=1 -s MAX_WEBGL_VERSION=2 -s ALLOW_MEMORY_GROWTH=1 --preload-file $(dir $<)resources@resources --shell-file ./shell.html $(INCLUDE_DIR) $(LIBRARY_DIR)
endif

.PHONY: all

all: sdf

sdf: sdf.cpp
	$(CC) -o $@$(EXT) sdf.cpp $(CFLAGS) $(LIBS) 

clean:
	rm sdf$(EXT)
