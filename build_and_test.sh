#!/bin/bash

# build_and_test.sh
# [AI GENERATED] Comprehensive build and test script for Piano Synth layered architecture

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
CMAKE_BUILD_TYPE="Release"
RUN_TESTS=true
RUN_COVERAGE=false
RUN_VALGRIND=false
RUN_BENCHMARKS=false
CLEAN_BUILD=false
VERBOSE=false
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)
GENERATE_DOCS=false

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

print_section() {
    echo -e "\n${PURPLE}========================================${NC}"
    echo -e "${PURPLE}$1${NC}"
    echo -e "${PURPLE}========================================${NC}\n"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build (remove build directory)"
    echo "  -d, --debug         Build in Debug mode (default: Release)"
    echo "  -r, --release       Build in Release mode"
    echo "  -t, --no-tests      Skip running tests"
    echo "  -v, --verbose       Verbose output"
    echo "  --coverage          Enable code coverage analysis (implies debug)"
    echo "  --valgrind          Run tests with Valgrind memory checking"
    echo "  --benchmark         Run performance benchmarks"
    echo "  --docs              Generate documentation with Doxygen"
    echo "  -j, --jobs N        Number of parallel build jobs (default: $PARALLEL_JOBS)"
    echo ""
    echo "Examples:"
    echo "  $0                  # Standard release build with tests"
    echo "  $0 -d --coverage    # Debug build with code coverage"
    echo "  $0 -c -r            # Clean release build"
    echo "  $0 --valgrind       # Build and run with memory checking"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -d|--debug)
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            CMAKE_BUILD_TYPE="Release"
            shift
            ;;
        -t|--no-tests)
            RUN_TESTS=false
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --coverage)
            RUN_COVERAGE=true
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        --valgrind)
            RUN_VALGRIND=true
            shift
            ;;
        --benchmark)
            RUN_BENCHMARKS=true
            shift
            ;;
        --docs)
            GENERATE_DOCS=true
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate parallel jobs argument
if ! [[ "$PARALLEL_JOBS" =~ ^[0-9]+$ ]] || [ "$PARALLEL_JOBS" -eq 0 ]; then
    print_error "Invalid number of jobs: $PARALLEL_JOBS"
    exit 1
fi

# Function to check for required tools
check_dependencies() {
    print_section "Checking Dependencies"

    local missing_deps=()

    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    else
        local cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
        print_status "Found CMake version $cmake_version"
    fi

    # Check for compiler
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        missing_deps+=("g++ or clang++")
    else
        if command -v g++ &> /dev/null; then
            local gcc_version=$(g++ --version | head -n1)
            print_status "Found $gcc_version"
        fi
        if command -v clang++ &> /dev/null; then
            local clang_version=$(clang++ --version | head -n1)
            print_status "Found $clang_version"
        fi
    fi

    # Check for make
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi

    # Check optional dependencies
    if [ "$RUN_VALGRIND" = true ] && ! command -v valgrind &> /dev/null; then
        print_warning "Valgrind not found - memory checking will be skipped"
        RUN_VALGRIND=false
    elif [ "$RUN_VALGRIND" = true ]; then
        local valgrind_version=$(valgrind --version)
        print_status "Found $valgrind_version"
    fi

    if [ "$RUN_COVERAGE" = true ] && ! command -v gcov &> /dev/null; then
        print_warning "gcov not found - coverage analysis will be limited"
    elif [ "$RUN_COVERAGE" = true ]; then
        print_status "Found gcov for coverage analysis"
    fi

    if [ "$GENERATE_DOCS" = true ] && ! command -v doxygen &> /dev/null; then
        print_warning "Doxygen not found - documentation generation will be skipped"
        GENERATE_DOCS=false
    elif [ "$GENERATE_DOCS" = true ]; then
        local doxygen_version=$(doxygen --version)
        print_status "Found Doxygen $doxygen_version"
    fi

    # Report missing dependencies
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies:"
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        echo ""
        echo "Please install the missing dependencies and try again."
        exit 1
    fi

    print_success "All required dependencies found"
}

# Function to clean build directory
clean_build() {
    if [ -d "$BUILD_DIR" ]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
}

# Function to configure build
configure_build() {
    print_section "Configuring Build"

    print_status "Build type: $CMAKE_BUILD_TYPE"
    print_status "Parallel jobs: $PARALLEL_JOBS"

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    local cmake_args="-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"

    if [ "$RUN_COVERAGE" = true ]; then
        cmake_args="$cmake_args -DCMAKE_CXX_FLAGS=--coverage -DCMAKE_EXE_LINKER_FLAGS=--coverage"
        print_status "Code coverage enabled"
    fi

    if [ "$VERBOSE" = true ]; then
        cmake_args="$cmake_args -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi

    print_status "Running CMake configuration..."
    if ! cmake $cmake_args ..; then
        print_error "CMake configuration failed"
        exit 1
    fi

    cd ..
    print_success "Build configured successfully"
}

# Function to build project
build_project() {
    print_section "Building Project"

    cd "$BUILD_DIR"

    print_status "Building with $PARALLEL_JOBS parallel jobs..."
    if [ "$VERBOSE" = true ]; then
        if ! make -j"$PARALLEL_JOBS"; then
            print_error "Build failed"
            exit 1
        fi
    else
        if ! make -j"$PARALLEL_JOBS" > build.log 2>&1; then
            print_error "Build failed. Check build.log for details:"
            tail -20 build.log
            exit 1
        fi
    fi

    cd ..
    print_success "Build completed successfully"
}

# Function to run all tests
run_tests() {
    if [ "$RUN_TESTS" = false ]; then
        print_warning "Skipping tests (--no-tests specified)"
        return 0
    fi

    print_section "Running Test Suite"

    cd "$BUILD_DIR"

    local test_failed=false
    local total_tests=0
    local passed_tests=0

    # List of test executables in order of complexity
    local test_executables=(
        "test_midi_input"
        "test_abstractor"
        "test_note_synth"
        "test_output_handler"
        "test_input_layer"
        "test_abstraction_layer"
        "test_output_layer"
        "test_midi_device"
        "test_integration"
        "test_legacy_compatibility"
    )

    for test_exe in "${test_executables[@]}"; do
        if [ -f "$test_exe" ]; then
            print_status "Running $test_exe..."
            total_tests=$((total_tests + 1))

            if [ "$RUN_VALGRIND" = true ]; then
                print_status "Running with Valgrind memory checking..."
                if valgrind --leak-check=full --error-exitcode=1 --quiet ./"$test_exe" > "${test_exe}_valgrind.log" 2>&1; then
                    print_success "$test_exe passed (with Valgrind)"
                    passed_tests=$((passed_tests + 1))
                else
                    print_error "$test_exe failed (Valgrind detected issues)"
                    echo "Valgrind output:"
                    cat "${test_exe}_valgrind.log"
                    test_failed=true
                fi
            else
                if ./"$test_exe" > "${test_exe}.log" 2>&1; then
                    print_success "$test_exe passed"
                    passed_tests=$((passed_tests + 1))
                else
                    print_error "$test_exe failed"
                    echo "Test output:"
                    cat "${test_exe}.log"
                    test_failed=true
                fi
            fi
        else
            print_warning "$test_exe not found - skipping"
        fi
    done

    # Run CTest for additional tests
    if [ -f "CTestTestfile.cmake" ]; then
        print_status "Running CTest suite..."
        if ctest --output-on-failure -j"$PARALLEL_JOBS"; then
            print_success "CTest suite passed"
        else
            print_error "CTest suite failed"
            test_failed=true
        fi
    fi

    cd ..

    # Test summary
    echo ""
    print_section "Test Results Summary"
    echo "Total custom tests run: $total_tests"
    echo "Tests passed: $passed_tests"
    echo "Tests failed: $((total_tests - passed_tests))"

    if [ "$test_failed" = true ]; then
        print_error "Some tests failed!"
        return 1
    else
        print_success "All tests passed!"
        return 0
    fi
}

# Function to run benchmarks
run_benchmarks() {
    if [ "$RUN_BENCHMARKS" = false ]; then
        return 0
    fi

    print_section "Running Performance Benchmarks"

    cd "$BUILD_DIR"

    if [ -f "test_integration" ]; then
        print_status "Running integration benchmarks..."
        if ./test_integration --benchmark > benchmark_results.txt 2>&1; then
            print_success "Benchmarks completed"
            echo "Benchmark results:"
            cat benchmark_results.txt
        else
            print_warning "Benchmarks failed or not supported"
        fi
    else
        print_warning "Benchmark executable not found"
    fi

    cd ..
}

# Function to generate code coverage report
generate_coverage() {
    if [ "$RUN_COVERAGE" = false ]; then
        return 0
    fi

    print_section "Generating Code Coverage Report"

    cd "$BUILD_DIR"

    if command -v lcov &> /dev/null; then
        print_status "Generating coverage report with lcov..."

        # Capture coverage data
        lcov --capture --directory . --output-file coverage.info > coverage.log 2>&1 || true

        # Filter out system files
        lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info > coverage.log 2>&1 || true

        # Generate HTML report
        if command -v genhtml &> /dev/null; then
            genhtml coverage_filtered.info --output-directory coverage_html > coverage.log 2>&1 || true
            print_success "Coverage report generated in coverage_html/"
        else
            print_warning "genhtml not found - HTML report not generated"
        fi

        # Display summary
        if [ -f "coverage_filtered.info" ]; then
            lcov --list coverage_filtered.info | tail -10
        fi
    else
        print_warning "lcov not found - using basic gcov"
        find . -name "*.gcno" -exec gcov {} \; > coverage.log 2>&1 || true
        print_status "Basic coverage files generated"
    fi

    cd ..
}

# Function to generate documentation
generate_documentation() {
    if [ "$GENERATE_DOCS" = false ]; then
        return 0
    fi

    print_section "Generating Documentation"

    if [ -f "Doxyfile" ]; then
        print_status "Running Doxygen..."
        if doxygen Doxyfile > doxygen.log 2>&1; then
            print_success "Documentation generated"
        else
            print_warning "Documentation generation had warnings"
        fi
    else
        print_warning "Doxyfile not found - skipping documentation generation"
    fi
}

# Function to create demo files
create_demo() {
    print_section "Creating Demo Audio Files"

    cd "$BUILD_DIR"

    if [ -f "piano_synth" ]; then
        print_status "Generating demo audio files..."
        if ./piano_synth --demo > demo.log 2>&1; then
            print_success "Demo files created successfully"
            echo "Generated files:"
            ls -la *.wav 2>/dev/null || echo "No WAV files found"
        else
            print_warning "Demo generation failed"
            echo "Demo output:"
            cat demo.log 2>/dev/null || echo "No demo log found"
        fi
    else
        print_warning "piano_synth executable not found"
    fi

    cd ..
}

# Function to display build information
show_build_info() {
    print_section "Build Information"

    echo "Build Type: $CMAKE_BUILD_TYPE"
    echo "Parallel Jobs: $PARALLEL_JOBS"
    echo "Run Tests: $RUN_TESTS"
    echo "Code Coverage: $RUN_COVERAGE"
    echo "Valgrind: $RUN_VALGRIND"
    echo "Benchmarks: $RUN_BENCHMARKS"
    echo "Generate Docs: $GENERATE_DOCS"
    echo "Clean Build: $CLEAN_BUILD"
    echo "Verbose: $VERBOSE"

    if [ -d "$BUILD_DIR" ]; then
        echo ""
        echo "Build directory contents:"
        ls -la "$BUILD_DIR" | head -10
        if [ $(ls -1 "$BUILD_DIR" | wc -l) -gt 10 ]; then
            echo "... and $(($(ls -1 "$BUILD_DIR" | wc -l) - 10)) more files"
        fi
    fi
}

# Main execution function
main() {
    local start_time=$(date +%s)

    print_section "Piano Synth Build System"
    echo "Layered Architecture Build and Test Script"
    echo "[AI GENERATED] Comprehensive build automation"
    echo ""

    show_build_info

    # Clean if requested
    if [ "$CLEAN_BUILD" = true ]; then
        clean_build
    fi

    # Check dependencies
    check_dependencies

    # Configure and build
    configure_build
    build_project

    # Run tests if enabled
    local test_result=0
    if ! run_tests; then
        test_result=1
    fi

    # Generate coverage report
    generate_coverage

    # Run benchmarks
    run_benchmarks

    # Generate documentation
    generate_documentation

    # Create demo files
    create_demo

    # Final summary
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))

    print_section "Build Summary"
    echo "Total build time: ${duration} seconds"
    echo "Build type: $CMAKE_BUILD_TYPE"

    if [ -d "$BUILD_DIR" ]; then
        echo "Build artifacts:"
        find "$BUILD_DIR" -maxdepth 1 -type f -executable | head -5
        if [ $(find "$BUILD_DIR" -maxdepth 1 -type f -executable | wc -l) -gt 5 ]; then
            echo "... and more"
        fi
    fi

    if [ $test_result -eq 0 ]; then
        print_success "🎉 BUILD AND TESTS COMPLETED SUCCESSFULLY! 🎉"
        echo ""
        echo "Next steps:"
        echo "  • Run './build/piano_synth --demo' to generate demo audio"
        echo "  • Check coverage_html/ for test coverage report"
        echo "  • Look at benchmark_results.txt for performance metrics"
        echo "  • Review docs/ for generated documentation"
        return 0
    else
        print_error "❌ BUILD COMPLETED BUT TESTS FAILED"
        echo ""
        echo "Check the test logs in the build directory for details."
        return 1
    fi
}

# Trap to ensure we return to original directory on exit
trap 'cd "$(dirname "$0")"' EXIT

# Change to script directory
cd "$(dirname "$0")"

# Run main function
main "$@"
