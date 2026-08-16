APP_NAME := "Vulcano"

# Print help panel
help:
    just -l

# Configure the CMake build directory
init:
    @if [ ! -d build ]; then cmake --preset default; fi

# Compile/build the project
build: init
    cmake --build --preset default
alias b := build

# Force build the project by cleaning build targets first
force-build: init
    cmake --build --preset default --clean-first
alias fb := force-build

# Run the compiled Vulkan application (syncs assets if changed)
run: build copy-assets
    cd build && ./{{ APP_NAME }}
alias r := run

# Update assets for the current build
copy-assets: build
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

