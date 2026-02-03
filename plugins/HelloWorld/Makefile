# Makefile for Hello World Plugin
# Compiles to .so on Linux, .dll on Windows

CXX = g++
PLUGIN_NAME = HelloWorld

# Flags
CXXFLAGS = -std=c++17 -fPIC -DGTK -I../../linux-gtk-prototype -I../../scintilla/include
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = -shared
LDFLAGS += $(shell pkg-config --libs gtk+-3.0)

# Source and output
SRC = HelloWorld.cpp
OUTPUT = lib$(PLUGIN_NAME).so

# Build
all: $(OUTPUT)

$(OUTPUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUTPUT) $(LDFLAGS)
	@echo "Plugin built: $(OUTPUT)"
	@echo "Copy to ~/.config/notepad-plus-plus-gtk/plugins/ to use"

clean:
	rm -f $(OUTPUT)

install:
	mkdir -p ~/.config/notepad-plus-plus-gtk/plugins
	cp $(OUTPUT) ~/.config/notepad-plus-plus-gtk/plugins/
	@echo "Plugin installed successfully!"

.PHONY: all clean install
