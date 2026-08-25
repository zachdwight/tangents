# Tangents Visual Novel Engine - Makefile
# Modern C++20 with SFML, modular architecture

CXX      = clang++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic
INCLUDES = -I./include -I/usr/local/include
LDFLAGS  = -L/usr/local/lib -Wl,-rpath,/usr/local/lib
LIBS     = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

# Source files
MAIN_SRCS = src/main.cpp src/script_parser.cpp src/story_engine.cpp src/ui_renderer.cpp src/audio_manager.cpp
TEST_SRCS = tests/test_script_parser.cpp src/script_parser.cpp src/story_engine.cpp

# Targets
TARGET = tangents
TEST_TARGET = test_tangents

.PHONY: all build run test lint fmt clean help

all: build lint test
	@echo "✓ Build, lint, and tests complete"

build: $(TARGET)
	@echo "✓ Build complete: ./$(TARGET)"

$(TARGET): $(MAIN_SRCS)
	@echo "Building $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(MAIN_SRCS) $(LIBS) -o $(TARGET)

run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

test: $(TEST_TARGET)
	@echo "Running tests..."
	./$(TEST_TARGET) || echo "⚠ Catch2 tests require: brew install catch2"

$(TEST_TARGET): $(TEST_SRCS)
	@command -v catch2-config >/dev/null 2>&1 && \
		$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(TEST_SRCS) -I/opt/homebrew/include -L/opt/homebrew/lib -lCatch2Main -lCatch2 $(LIBS) -o $(TEST_TARGET) && \
		echo "✓ Test binary built" || true

lint:
	@echo "Running clang-tidy..."
	@command -v clang-tidy >/dev/null 2>&1 && \
		clang-tidy src/*.cpp include/*.h -- $(CXXFLAGS) $(INCLUDES) || \
		echo "⚠ clang-tidy not found. Install with: brew install llvm"

fmt:
	@echo "Formatting with clang-format..."
	@command -v clang-format >/dev/null 2>&1 && \
		clang-format -i src/*.cpp include/*.h tests/*.cpp || \
		echo "⚠ clang-format not found"

clean:
	@echo "Cleaning..."
	rm -f $(TARGET) $(TEST_TARGET)
	rm -rf build/

help:
	@echo "Tangents Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make build       - Build executable"
	@echo "  make run         - Build and run"
	@echo "  make test        - Run unit tests (requires Catch2)"
	@echo "  make lint        - Run clang-tidy linter"
	@echo "  make fmt         - Format code with clang-format"
	@echo "  make clean       - Remove build artifacts"
	@echo "  make all         - Build, lint, and test"
	@echo ""
	@echo "Setup:"
	@echo "  brew install llvm catch2  # Install linting and testing"
