.PHONY: all build clean install uninstall help

# Default target
all: build

# Create build directory and build the project
build:
	@echo "Building Notepad++ GTK Prototype..."
	@mkdir -p build
	@cd build && cmake .. && $(MAKE)
	@echo ""
	@echo "Build complete! Binaries are in build/linux-gtk-prototype/"
	@echo "Run 'make install' or 'sudo make install' to install."

# Install the binary to /usr/local/bin
install:
	@echo "Installing Notepad++ to /usr/local/bin..."
	@cd build && $(MAKE) install
	@echo "Installation complete! Run 'notepad++' to start the application."

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf build
	@echo "Clean complete."

# Uninstall the binary
uninstall:
	@echo "Uninstalling Notepad++..."
	@rm -f /usr/local/bin/notepad++
	@echo "Uninstall complete."

# Rebuild from scratch
rebuild: clean build

# Run the application (for testing)
run: build
	@echo "Running gtk-proto..."
	@./build/linux-gtk-prototype/gtk-proto

# Display help information
help:
	@echo "Notepad++ GTK Prototype - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build the project (default)"
	@echo "  make install  - Install to /usr/local/bin (requires sudo)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and rebuild from scratch"
	@echo "  make run      - Build and run the gtk-proto executable"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make"
	@echo "  2. sudo make install"
	@echo "  3. notepad++"
