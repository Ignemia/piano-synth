#!/bin/bash

# Piano Synthesizer Build and Test Script
# This script builds the project and runs all tests

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Project directory
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

print_status "Piano Synthesizer Build and Test Script"
print_status "Project directory: $PROJECT_DIR"

# Check if we're in the right directory
if [ ! -f "$PROJECT_DIR/CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found in $PROJECT_DIR"
    exit 1
fi

# Parse command line arguments
BUILD_TYPE="Release"
RUN_TESTS=true
CLEAN_BUILD=false
VERBOSE=false
INSTALL=false
COVERAGE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --no-tests)
            RUN_TESTS=false
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --coverage)
            BUILD_TYPE="Debug"
            COVERAGE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode (default: Release)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  -v, --verbose   Verbose build output"
            echo "      --no-tests  Skip running tests"
            echo "      --install   Install after building"
            echo "      --coverage  Build with coverage support (implies Debug)"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_status "Build configuration:"
print_status "  Build type: $BUILD_TYPE"
print_status "  Clean build: $CLEAN_BUILD"
print_status "  Run tests: $RUN_TESTS"
print_status "  Coverage: $COVERAGE"

# Check for required dependencies
print_status "Checking dependencies..."

check_dependency() {
    if command -v $1 &> /dev/null; then
        print_success "$1 found"
    else
        print_error "$1 not found. Please install it."
        return 1
    fi
}

check_library() {
    if pkg-config --exists $1 2>/dev/null; then
        print_success "lib$1 found"
    elif ldconfig -p | grep -q $1 2>/dev/null; then
        print_success "lib$1 found"
    else
        print_error "lib$1 not found. Please install lib$1-dev"
        return 1
    fi
}

# Check for required tools
DEPS_OK=true
check_dependency cmake || DEPS_OK=false
check_dependency make || DEPS_OK=false
check_dependency g++ || DEPS_OK=false

# Check for required libraries
# Updated to check the generic portaudio library instead of the
# non-existent portaudio19 variant on modern distributions. [AI GENERATED]
check_library portaudio || DEPS_OK=false
check_library mp3lame || DEPS_OK=false
check_library rtmidi || DEPS_OK=false

if [ "$RUN_TESTS" = true ]; then
    if ! pkg-config --exists gtest 2>/dev/null && ! find /usr -name "libgtest*" 2>/dev/null | grep -q .; then
        print_warning "Google Test not found. Tests may not build."
    else
        print_success "Google Test found"
    fi
fi

if [ "$DEPS_OK" = false ]; then
    print_error "Missing dependencies. Please install them and try again."
    print_status "On Ubuntu/Debian, run:"
    print_status "  sudo apt update"
    print_status "  sudo apt install cmake build-essential libportaudio19-dev libmp3lame-dev librtmidi-dev libgtest-dev"
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
print_status "Configuring with CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_TESTS=ON
)

if [ "$COVERAGE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage")
fi

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" "$PROJECT_DIR"

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_status "Building the project..."
MAKE_ARGS=(-j$(nproc))

if [ "$VERBOSE" = true ]; then
    MAKE_ARGS+=(VERBOSE=1)
fi

make "${MAKE_ARGS[@]}"

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    print_status "Running tests..."
    
    # Check if test executables exist
    if [ ! -f "piano_synth_tests" ] && [ ! -f "test_math_utils" ]; then
        print_warning "No test executables found. Tests may not have been built."
    else
        # Run CTest
        if command -v ctest &> /dev/null; then
            print_status "Running tests with CTest..."
            if [ "$VERBOSE" = true ]; then
                ctest --output-on-failure --verbose
            else
                ctest --output-on-failure
            fi
            
            if [ $? -eq 0 ]; then
                print_success "All tests passed!"
            else
                print_error "Some tests failed"
                exit 1
            fi
        else
            # Run tests manually
            print_status "Running individual tests..."
            
            for test_exe in test_*; do
                if [ -x "$test_exe" ]; then
                    print_status "Running $test_exe..."
                    ./"$test_exe"
                    if [ $? -ne 0 ]; then
                        print_error "$test_exe failed"
                        exit 1
                    fi
                fi
            done
            
            if [ -x "piano_synth_tests" ]; then
                print_status "Running piano_synth_tests..."
                ./piano_synth_tests
                if [ $? -ne 0 ]; then
                    print_error "piano_synth_tests failed"
                    exit 1
                fi
            fi
            
            print_success "All tests passed!"
        fi
    fi
fi

# Generate coverage report if requested
if [ "$COVERAGE" = true ]; then
    print_status "Generating coverage report..."
    
    if command -v lcov &> /dev/null && command -v genhtml &> /dev/null; then
        lcov --directory . --capture --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --remove coverage.info '*/tests/*' --output-file coverage.info
        genhtml coverage.info --output-directory coverage_html
        
        print_success "Coverage report generated in coverage_html/"
        
        if command -v xdg-open &> /dev/null; then
            print_status "Opening coverage report in browser..."
            xdg-open coverage_html/index.html &
        fi
    else
        print_warning "lcov and genhtml not found. Cannot generate coverage report."
        print_status "Install with: sudo apt install lcov"
    fi
fi

# Install if requested
if [ "$INSTALL" = true ]; then
    print_status "Installing..."
    sudo make install
    
    if [ $? -eq 0 ]; then
        print_success "Installation completed"
    else
        print_error "Installation failed"
        exit 1
    fi
fi

# Summary
print_success "Build and test completed successfully!"

if [ -f "piano_synth" ]; then
    print_status "Executable location: $BUILD_DIR/piano_synth"
    print_status "To run: $BUILD_DIR/piano_synth"
fi

print_status "Build artifacts are in: $BUILD_DIR"

# Check if config file exists
if [ -f "$BUILD_DIR/config/piano_config.json" ]; then
    print_status "Configuration file: $BUILD_DIR/config/piano_config.json"
else
    print_warning "Configuration file not found. Using defaults."
fi

# Performance info
if [ -f "piano_synth_tests" ] && [ "$RUN_TESTS" = true ]; then
    print_status "For performance testing, run: $BUILD_DIR/piano_synth_tests --gtest_filter=\"*Performance*\""
fi

if [ -f "piano_synth_integration_tests" ] && [ "$RUN_TESTS" = true ]; then
    print_status "For integration testing, run: $BUILD_DIR/piano_synth_integration_tests"
fi

print_status "Done!"