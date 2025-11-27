#!/usr/bin/env bash
# build-all-languages.sh
# Builds Dyscover packages for all supported languages
# Part of Feature 006: Language-Specific Resource Optimization

set -e  # Exit on error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SUPPORTED_LANGUAGES=("nl" "nl_be")
LICENSING="${LICENSING:-demo}"
BUILD_TESTS="${BUILD_TESTS:-OFF}"
PARALLEL="${PARALLEL:-ON}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Function to build a single language
build_language() {
    local language=$1
    local build_dir="build-${language}"
    
    print_info "Building language: ${language}"
    
    # Create language-specific build directory
    mkdir -p "${PROJECT_ROOT}/${build_dir}"
    cd "${PROJECT_ROOT}/${build_dir}"
    
    # Configure CMake
    print_info "Configuring CMake for ${language}..."
    if ! cmake \
        -DLANGUAGE="${language}" \
        -DLICENSING="${LICENSING}" \
        -DBUILD_TESTS="${BUILD_TESTS}" \
        -DPACKAGING_ENABLE=ON \
        ..; then
        print_error "CMake configuration failed for ${language}"
        return 1
    fi
    
    # Build
    print_info "Building Dyscover for ${language}..."
    local cpu_count=$(nproc 2>/dev/null || echo 4)
    if ! cmake --build . --target Dyscover --parallel ${cpu_count}; then
        print_error "Build failed for ${language}"
        return 1
    fi
    
    # Create package
    print_info "Creating package for ${language}..."
    if ! cpack; then
        print_error "Package creation failed for ${language}"
        return 1
    fi
    
    print_success "Completed build for ${language}"
    
    # Display package info
    local package_files=$(find . -maxdepth 1 -name "Dyscover-${language}-*.zip" -o -name "Dyscover-${language}-*.tar.gz" 2>/dev/null)
    if [ -n "$package_files" ]; then
        print_info "Package(s) created:"
        echo "$package_files" | while read -r file; do
            local size=$(du -h "$file" | cut -f1)
            echo "  - $(basename "$file") (${size})"
        done
    fi
    
    return 0
}

# Function to build all languages in parallel
build_parallel() {
    print_info "Building all languages in parallel..."
    local pids=()
    local failed=()
    
    for language in "${SUPPORTED_LANGUAGES[@]}"; do
        build_language "$language" &
        pids+=($!)
    done
    
    # Wait for all builds to complete
    for i in "${!pids[@]}"; do
        if ! wait "${pids[$i]}"; then
            failed+=("${SUPPORTED_LANGUAGES[$i]}")
        fi
    done
    
    if [ ${#failed[@]} -gt 0 ]; then
        print_error "Failed builds: ${failed[*]}"
        return 1
    fi
    
    return 0
}

# Function to build all languages sequentially
build_sequential() {
    print_info "Building all languages sequentially..."
    local failed=()
    
    for language in "${SUPPORTED_LANGUAGES[@]}"; do
        if ! build_language "$language"; then
            failed+=("$language")
        fi
    done
    
    if [ ${#failed[@]} -gt 0 ]; then
        print_error "Failed builds: ${failed[*]}"
        return 1
    fi
    
    return 0
}

# Function to collect and display summary
display_summary() {
    print_info "Build Summary:"
    echo ""
    
    local total_size=0
    for language in "${SUPPORTED_LANGUAGES[@]}"; do
        local build_dir="${PROJECT_ROOT}/build-${language}"
        if [ -d "$build_dir" ]; then
            echo "Language: ${language}"
            
            # Count audio files
            local audio_dir="${build_dir}/audio"
            if [ -d "$audio_dir" ]; then
                local audio_count=$(find "$audio_dir" -name "*.wav" | wc -l)
                local audio_size=$(du -sh "$audio_dir" 2>/dev/null | cut -f1)
                echo "  Audio files: ${audio_count} (${audio_size})"
            fi
            
            # List packages
            local packages=$(find "$build_dir" -maxdepth 1 \( -name "Dyscover-${language}-*.zip" -o -name "Dyscover-${language}-*.tar.gz" \) 2>/dev/null)
            if [ -n "$packages" ]; then
                echo "  Packages:"
                echo "$packages" | while read -r pkg; do
                    local size=$(du -h "$pkg" | cut -f1)
                    echo "    - $(basename "$pkg") (${size})"
                done
            fi
            echo ""
        fi
    done
}

# Main execution
main() {
    print_info "Multi-Language Build Script for Dyscover"
    print_info "Project root: ${PROJECT_ROOT}"
    print_info "Supported languages: ${SUPPORTED_LANGUAGES[*]}"
    print_info "Licensing: ${LICENSING}"
    print_info "Build tests: ${BUILD_TESTS}"
    print_info "Parallel build: ${PARALLEL}"
    echo ""
    
    cd "${PROJECT_ROOT}"
    
    # Build based on parallel setting
    if [ "$PARALLEL" = "ON" ]; then
        if ! build_parallel; then
            print_error "Multi-language build failed"
            exit 1
        fi
    else
        if ! build_sequential; then
            print_error "Multi-language build failed"
            exit 1
        fi
    fi
    
    echo ""
    display_summary
    
    print_success "All language builds completed successfully!"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --licensing)
            LICENSING="$2"
            shift 2
            ;;
        --tests)
            BUILD_TESTS="$2"
            shift 2
            ;;
        --sequential)
            PARALLEL="OFF"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --licensing <demo|full>  Set licensing mode (default: demo)"
            echo "  --tests <ON|OFF>         Build tests (default: OFF)"
            echo "  --sequential             Build languages sequentially (default: parallel)"
            echo "  --help                   Show this help message"
            echo ""
            echo "Environment variables:"
            echo "  LICENSING                Set licensing mode"
            echo "  BUILD_TESTS              Build tests"
            echo "  PARALLEL                 Parallel build (ON/OFF)"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Run main function
main
