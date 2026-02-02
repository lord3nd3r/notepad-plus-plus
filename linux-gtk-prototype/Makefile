SCINTILLA_DIR = ../scintilla
SCINTILLA_BUILD_DIR = $(SCINTILLA_DIR)/bin
SCINTILLA_A = $(SCINTILLA_BUILD_DIR)/scintilla.a
SCINTILLA_SO = $(SCINTILLA_BUILD_DIR)/libscintilla.so

LEXILLA_DIR = ../lexilla
LEXILLA_BUILD_DIR = $(LEXILLA_DIR)/bin
LEXILLA_A = $(LEXILLA_BUILD_DIR)/liblexilla.a
LEXILLA_SO = $(LEXILLA_BUILD_DIR)/liblexilla.so

CXX = g++
PKG_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
PKG_LIBS = $(shell pkg-config --libs gtk+-3.0 gmodule-no-export-2.0)

CXXFLAGS = -std=c++17 -O2 $(PKG_CFLAGS) -DGTK -I$(SCINTILLA_DIR)/include -I$(SCINTILLA_DIR)/gtk -I$(LEXILLA_DIR)/include -I$(LEXILLA_DIR)/access
LDFLAGS = $(PKG_LIBS)

TARGET = gtk-proto
SRCS = main_gui.cxx
OBJS = $(SRCS:.cxx=.o)

.PHONY: all clean scintilla lexilla

all: lexilla scintilla $(TARGET)

lexilla:
	$(MAKE) -C $(LEXILLA_DIR)/src

scintilla:
	$(MAKE) -C $(SCINTILLA_DIR)/gtk GTK3=1

$(TARGET): $(OBJS)
	# Prefer static libs if available
	if [ -f "$(SCINTILLA_A)" ] && [ -f "$(LEXILLA_A)" ]; then \
		$(CXX) $(OBJS) -o $(TARGET) $(SCINTILLA_A) $(LEXILLA_A) $(LDFLAGS); \
	else \
		$(CXX) $(OBJS) -o $(TARGET) $(SCINTILLA_SO) $(LEXILLA_SO) $(LDFLAGS); \
	fi

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(TARGET)
