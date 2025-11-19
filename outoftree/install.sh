#!/usr/bin/env bash
set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[*]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# ===========================
# Configuration
# ===========================
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO_ROOT/build"
SRC_DIR="$REPO_ROOT/src"
INC_DIR="$REPO_ROOT/include"
INSTALL_DIR="$HOME/.local/myclang"
BIN_DIR="$HOME/.local/bin"

print_status "Starting myclang installation..."
echo ""

# ===========================
# Step 1: Verify build directory exists
# ===========================
print_status "Checking for build directory..."
if [[ ! -d "$BUILD_DIR" ]]; then
    print_error "Build directory not found!"
    print_status "Creating build directory and building passes..."
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_status "Running cmake..."
    cmake .. || {
        print_error "CMake failed! Please check your CMakeLists.txt"
        exit 1
    }
    
    print_status "Running make..."
    make -j$(nproc  || echo 4) || {
        print_error "Make failed! Please check your build configuration"
        exit 1
    }
    
    cd "$REPO_ROOT"
    print_success "Build completed"
else
    print_success "Build directory found"
fi

# ===========================
# Step 2: Verify all required files exist
# ===========================
print_status "Verifying required files..."

REQUIRED_PASSES=(
    "libInitPtrPass.so"
    "libAsanPass.so"
    "libGEP.so"
    "libMemcpymv.so"
    "libFreePass.so"
)

# Check for passes in build directory
MISSING_FILES=0
for pass in "${REQUIRED_PASSES[@]}"; do
    if [[ ! -f "$BUILD_DIR/$pass" ]]; then
        print_error "Missing pass: $pass"
        MISSING_FILES=1
    fi
done

# Check for logger.cpp in src directory
if [[ ! -f "$SRC_DIR/logger.cpp" ]]; then
    print_error "Missing runtime file: logger.cpp in $SRC_DIR"
    MISSING_FILES=1
fi

if [[ $MISSING_FILES -eq 1 ]]; then
    print_error "Some required files are missing. Please ensure your build completed successfully."
    exit 1
fi

print_success "All required files found"

# ===========================
# Step 3: Create installation directory
# ===========================
print_status "Creating installation directory..."

mkdir -p "$INSTALL_DIR/passes"
mkdir -p "$INSTALL_DIR/runtime"
mkdir -p "$INSTALL_DIR/include"
mkdir -p "$BIN_DIR"

print_success "Installation directory created: $INSTALL_DIR"

# ===========================
# Step 4: Copy files to installation directory
# ===========================
print_status "Copying passes..."

for pass in "${REQUIRED_PASSES[@]}"; do
    cp "$BUILD_DIR/$pass" "$INSTALL_DIR/passes/"
    print_success "Copied $pass"
done

print_status "Copying runtime files..."
cp "$SRC_DIR/logger.cpp" "$INSTALL_DIR/runtime/"
cp "$INC_DIR/logger.h" "$INSTALL_DIR/include/"
print_success "Copied logger.cpp"

# ===========================
# Step 5: Create wrapper script
# ===========================
print_status "Creating myclang wrapper script..."

cat > "$INSTALL_DIR/myclang" << 'WRAPPER_EOF'
#!/usr/bin/env bash
set -euo pipefail

# Auto-detect script location
# Auto-detect script location (resolve symlinks)
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    DIR="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
SCRIPT_DIR="$(cd -P "$(dirname "$SOURCE")" && pwd)"

# Set paths relative to script location
InitPtr_PASS="$SCRIPT_DIR/passes/libInitPtrPass.so"
ASAN_PASS="$SCRIPT_DIR/passes/libAsanPass.so"
GEP_PASS="$SCRIPT_DIR/passes/libGEP.so"
MEMCPYmv_PASS="$SCRIPT_DIR/passes/libMemcpymv.so"
FREE_PASS="$SCRIPT_DIR/passes/libFreePass.so"
LOGGER_SRC="$SCRIPT_DIR/runtime/logger.cpp"

# Verify required files exist
for file in "$InitPtr_PASS" "$ASAN_PASS" "$GEP_PASS" "$MEMCPYmv_PASS" "$FREE_PASS" "$LOGGER_SRC"; do
    if [[ ! -f "$file" ]]; then
        echo "Error: Required file not found: $file" >&2
        exit 1
    fi
done

# Directory for helper objects
HELPER_DIR="${TMPDIR:-/tmp}/myclang_helpers_$$"
mkdir -p "$HELPER_DIR"

# Cleanup on exit
trap 'rm -rf "$HELPER_DIR"' EXIT

# Argument parsing
DO_COMPILE_ONLY=0
DO_IR_ONLY=0
DO_EMIT_LLVM=0
OUTPUT=""
OUTPUT_SET=0
SRC_FILES=()
OBJ_INPUT_FILES=()
OTHER_FLAGS=()
LINK_FLAGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c)
            DO_COMPILE_ONLY=1
            OTHER_FLAGS+=("$1")
            shift
            ;;
        -S)
            DO_IR_ONLY=1
            OTHER_FLAGS+=("$1")
            shift
            ;;
        -emit-llvm)
            DO_EMIT_LLVM=1
            OTHER_FLAGS+=("$1")
            shift
            ;;
        -o)
            shift
            if [[ $# -eq 0 ]]; then
                echo "Error: -o requires an argument" >&2
                exit 1
            fi
            OUTPUT="$1"
            OUTPUT_SET=1
            shift
            ;;
        -l*|-L*|-Wl,*)
            LINK_FLAGS+=("$1")
            shift
            ;;
        -O*|-g|-I*|-D*|-W*|-std=*|-f*)
            OTHER_FLAGS+=("$1")
            shift
            ;;
        *.c|*.cpp|*.cc|*.cxx|*.C)
            SRC_FILES+=("$1")
            shift
            ;;
        *.o|*.a|*.so)
            OBJ_INPUT_FILES+=("$1")
            shift
            ;;
        --)
            shift
            SRC_FILES+=("$@")
            break
            ;;
        -*)
            OTHER_FLAGS+=("$1")
            shift
            ;;
        *)
            if [[ -f "$1" ]]; then
                if file "$1"  | grep -q "ELF.*relocatable"; then
                    OBJ_INPUT_FILES+=("$1")
                else
                    SRC_FILES+=("$1")
                fi
            else
                SRC_FILES+=("$1")
            fi
            shift
            ;;
    esac
done

if [[ ${#SRC_FILES[@]} -eq 0 && ${#OBJ_INPUT_FILES[@]} -eq 0 ]]; then
    echo "Usage: $(basename "$0") [options] source_files..." >&2
    exit 1
fi

# Set default output
if [[ $OUTPUT_SET -eq 0 ]]; then
    if [[ $DO_COMPILE_ONLY -eq 1 ]]; then
        bn=$(basename "${SRC_FILES[0]}")
        OUTPUT="${bn%.*}.o"
    elif [[ $DO_IR_ONLY -eq 1 && $DO_EMIT_LLVM -eq 1 ]]; then
        bn=$(basename "${SRC_FILES[0]}")
        OUTPUT="${bn%.*}.ll"
    elif [[ $DO_IR_ONLY -eq 1 ]]; then
        bn=$(basename "${SRC_FILES[0]}")
        OUTPUT="${bn%.*}.s"
    else
        OUTPUT="a.out"
    fi
fi

# Prepare helper object (logger only)
LOGGER_OBJ="$HELPER_DIR/logger.o"

if [[ ! -f "$LOGGER_OBJ" ]]; then
    clang++ -c -fsanitize=address -fPIC "$LOGGER_SRC" -o "$LOGGER_OBJ" 
fi

# Process source files through instrumentation pipeline
OBJ_FILES=()
FINAL_LL_FILES=()

if [[ ${#SRC_FILES[@]} -gt 0 ]]; then
    for SRC in "${SRC_FILES[@]}"; do
        bn=$(basename "$SRC")
        base="${bn%.*}"
        
        WORK_DIR="$HELPER_DIR/$base"
        mkdir -p "$WORK_DIR"
        
        clang -O0 -g -S -emit-llvm -fsanitize=address "${OTHER_FLAGS[@]}" \
            "$SRC" -o "$WORK_DIR/$base.orig.ll" 

        opt -load-pass-plugin="$InitPtr_PASS" -passes="InitPtrPass" \
            "$WORK_DIR/$base.orig.ll" -S -o "$WORK_DIR/$base.initptr.ll" 
        
        opt -load-pass-plugin="$ASAN_PASS" -passes="AsanPass" \
            "$WORK_DIR/$base.initptr.ll" -S -o "$WORK_DIR/$base.asan.ll" 
        
        opt -load-pass-plugin="$GEP_PASS" -passes="GEP" \
            "$WORK_DIR/$base.asan.ll" -S -o "$WORK_DIR/$base.gep.ll" 
        
        opt -load-pass-plugin="$MEMCPYmv_PASS" -passes="Memcpymv" \
            "$WORK_DIR/$base.gep.ll" -S -o "$WORK_DIR/$base.memcpymv.ll" 
        
        opt -load-pass-plugin="$FREE_PASS" -passes="FreePass" \
            "$WORK_DIR/$base.memcpymv.ll" -S -o "$WORK_DIR/$base.final.ll" 
        
        FINAL_LL_FILES+=("$WORK_DIR/$base.final.ll")
        
        if [[ $DO_IR_ONLY -eq 1 && $DO_EMIT_LLVM -eq 1 ]]; then
            if [[ ${#SRC_FILES[@]} -eq 1 ]]; then
                cp "$WORK_DIR/$base.final.ll" "$OUTPUT"
            else
                cp "$WORK_DIR/$base.final.ll" "$base.ll"
            fi
        else
            OBJ_FILE="$WORK_DIR/$base.o"
            clang -c "$WORK_DIR/$base.final.ll" -o "$OBJ_FILE" 
            OBJ_FILES+=("$OBJ_FILE")
        fi
    done
fi

# Handle different output modes
if [[ $DO_IR_ONLY -eq 1 && $DO_EMIT_LLVM -eq 1 ]]; then
    exit 0
fi

if [[ $DO_IR_ONLY -eq 1 ]]; then
    for ll in "${FINAL_LL_FILES[@]}"; do
        bn=$(basename "$ll" .final.ll)
        llc "$ll" -o "$bn.s" 
    done
    exit 0
fi

if [[ $DO_COMPILE_ONLY -eq 1 ]]; then
    if [[ ${#OBJ_FILES[@]} -eq 1 ]]; then
        mv "${OBJ_FILES[0]}" "$OUTPUT"
    else
        for obj in "${OBJ_FILES[@]}"; do
            bn=$(basename "$obj")
            cp "$obj" "$bn"
        done
    fi
    exit 0
fi

# Full linking
ALL_OBJS=("${OBJ_FILES[@]}" "${OBJ_INPUT_FILES[@]}")

clang "$LOGGER_OBJ" "${ALL_OBJS[@]}" \
    -fsanitize=address -lstdc++ "${LINK_FLAGS[@]}" -o "$OUTPUT" 
WRAPPER_EOF

chmod +x "$INSTALL_DIR/myclang"
print_success "Wrapper script created"

# ===========================
# Step 6: Create symlink in ~/.local/bin
# ===========================
print_status "Creating symlink in $BIN_DIR..."

ln -sf "$INSTALL_DIR/myclang" "$BIN_DIR/myclang"
print_success "Symlink created: $BIN_DIR/myclang -> $INSTALL_DIR/myclang"

# ===========================
# Step 7: Detect shell and update PATH
# ===========================
print_status "Checking PATH configuration..."

SHELL_CONFIG=""
if [[ -n "${BASH_VERSION:-}" ]]; then
    SHELL_CONFIG="$HOME/.bashrc"
elif [[ -n "${ZSH_VERSION:-}" ]]; then
    SHELL_CONFIG="$HOME/.zshrc"
elif [[ -f "$HOME/.bashrc" ]]; then
    SHELL_CONFIG="$HOME/.bashrc"
elif [[ -f "$HOME/.zshrc" ]]; then
    SHELL_CONFIG="$HOME/.zshrc"
fi

PATH_EXPORT='export PATH="$HOME/.local/bin:$PATH"'

if [[ -n "$SHELL_CONFIG" ]]; then
    if grep -q "\.local/bin" "$SHELL_CONFIG" ; then
        print_success "PATH already configured in $SHELL_CONFIG"
    else
        print_status "Adding $BIN_DIR to PATH in $SHELL_CONFIG..."
        echo "" >> "$SHELL_CONFIG"
        echo "# Added by myclang installer" >> "$SHELL_CONFIG"
        echo "$PATH_EXPORT" >> "$SHELL_CONFIG"
        print_success "PATH configured"
    fi
else
    print_warning "Could not detect shell config file"
    print_warning "Please manually add to your shell config:"
    echo "    $PATH_EXPORT"
fi

# ===========================
# Step 8: Test installation
# ===========================
print_status "Testing installation..."

# Create a temporary test directory
TEST_DIR=$(mktemp -d)
trap 'rm -rf "$TEST_DIR"' EXIT

TEST_FILE="$TEST_DIR/test.c"
TEST_OUTPUT="$TEST_DIR/test_output"

cat > "$TEST_FILE" << 'TEST_EOF'
#include <stdio.h>
int main() {
    printf("Hello from myclang!\n");
    return 0;
}
TEST_EOF

if "$INSTALL_DIR/myclang" "$TEST_FILE" -o "$TEST_OUTPUT" ; then
    if [[ -x "$TEST_OUTPUT" ]]; then
        print_success "Installation test passed"
    else
        print_warning "Compilation succeeded but output is not executable"
    fi
else
    print_warning "Installation test failed (this might be normal if dependencies are missing)"
fi

# ===========================
# Installation Complete
# ===========================
echo ""
print_success "Installation completed successfully!"
echo ""
echo -e "${GREEN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║${NC}  Installation Summary                                          ${GREEN}║${NC}"
echo -e "${GREEN}╠════════════════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║${NC}  Installed to: $INSTALL_DIR"
echo -e "${GREEN}║${NC}  Symlink: $BIN_DIR/myclang"
echo -e "${GREEN}║${NC}"
echo -e "${GREEN}║${NC}  ${YELLOW}Next steps:${NC}"
echo -e "${GREEN}║${NC}    1. Reload your shell configuration:"
if [[ -n "$SHELL_CONFIG" ]]; then
echo -e "${GREEN}║${NC}       ${BLUE}source $SHELL_CONFIG${NC}"
else
echo -e "${GREEN}║${NC}       ${BLUE}source ~/.bashrc${NC}  ${GREEN}# or ~/.zshrc${NC}"
fi
echo -e "${GREEN}║${NC}"
echo -e "${GREEN}║${NC}    2. Verify installation:"
echo -e "${GREEN}║${NC}       ${BLUE}which myclang${NC}"
echo -e "${GREEN}║${NC}"
echo -e "${GREEN}║${NC}    3. Test compilation:"
echo -e "${GREEN}║${NC}       ${BLUE}myclang test.c -o test${NC}"
echo -e "${GREEN}║${NC}"
echo -e "${GREEN}║${NC}  ${YELLOW}Usage with build systems:${NC}"
echo -e "${GREEN}║${NC}    CMake: ${BLUE}cmake -DCMAKE_C_COMPILER=myclang ..${NC}"
echo -e "${GREEN}║${NC}    Make:  ${BLUE}make CC=myclang${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""