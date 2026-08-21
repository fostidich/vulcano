APP_NAME := "Vulcano"

# Print help panel
help:
    @just -l

# Configure build directory
init:
    @if [ ! -d build ]; then cmake --preset default; fi

# Compile/build the project
build: init
    cmake --build --preset default
alias b := build

# Force build the project by cleaning build targets first
clean-build: init
    cmake --build --preset default --clean-first
alias cb := clean-build

# Run the compiled application (syncs assets if changed)
run *ARGS: build copy-assets
    cd build && ./{{ APP_NAME }} {{ ARGS }}
alias r := run

# Run with debug enabled
debug-run *ARGS: copy-assets
    @if [ ! -d build ]; then cmake --preset debug; fi
    cmake --build --preset debug
    cd build && DEBUG=1 ./{{ APP_NAME }} {{ ARGS }}

# Update assets for the current build
copy-assets:
    cmake -E copy_directory_if_different assets build/assets
alias ca := copy-assets

# Run static analysis and shader linting
lint: init
    # Linting C++ files with clang-tidy
    @git ls-files --others --cached --exclude-standard | grep -E '^src/.*\.cpp$' | xargs \
        clang-tidy -p build --quiet -header-filter="^include/.*\.hpp" || true
    # Validating GLSL shaders with glslc
    @git ls-files --others --cached --exclude-standard | grep -E '^shaders/.*\.(vert|frag)$' | xargs -n 1 \
        glslc -c --target-env=vulkan -o /dev/null || true

# Format C++ files
format:
    # Formatting C++ files with clang-format
    @git ls-files --others --cached --exclude-standard | grep -E '^(src|include)/.*\.(cpp|hpp)$' | xargs \
        clang-format -i || true
alias fmt := format

# Clean build artifacts
clean:
    rm -rf build .cache

# Open scene JSON editor
scene-builder:
    open .scene-builder/index.html
alias sb := scene-builder

