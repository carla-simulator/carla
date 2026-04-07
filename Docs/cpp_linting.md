# C++ Linting Setup for CARLA

## Prerequisites

Install clang-tidy:

### macOS
```bash
brew install llvm
```

### Ubuntu/Debian
```bash
sudo apt-get install clang-tidy
```

### Windows (with Visual Studio)
clang-tidy is included with Visual Studio 2022+ with "C++ Clang tools for Windows"

## Configuration

The project uses `.clang-tidy` in the root directory with tight linting rules including:
- bugprone-*
- cert-*
- clang-analyzer-*
- cppcoreguidelines-*
- misc-*
- modernize-*
- performance-*
- portability-*
- readability-*

## Usage

### Basic Usage
```bash
# Run on a single file
clang-tidy -p build Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Sensor/Sensor.cpp

# Run with automatic fixes
clang-tidy -p build <source-file> --fix

# Run with header filtering
clang-tidy -header-filter='.*' -p build <source-file>
```

### Using CMake Presets
```bash
# Configure with CMake
cmake --preset=linux-base

# Run clang-tidy on the project
cmake --build --preset=linux-base --target clang-tidy
```

### Running on All Files
```bash
# Find all C++ files and run clang-tidy
find Unreal -name "*.cpp" -o -name "*.h" | xargs -I {} clang-tidy -p build {}
```

## Integration with IDEs

### VS Code
Install the C++ extension and add to `.vscode/settings.json`:
```json
{
    "C_Cpp.clang_format_fallbackStyle": "file",
    "C_Cpp.errorSquiggles": "IfEnabled",
    "C_Cpp.clangTidy.enabled": true
}
```

### CLion
CLion automatically detects `.clang-tidy` configuration.

## Rules Explanation

### Enabled Categories
- **bugprone**: Detects bug-prone code patterns
- **cert**: CERT C++ Secure Coding Standard
- **clang-analyzer**: Static analysis
- **cppcoreguidelines**: C++ Core Guidelines
- **misc**: Miscellaneous checks
- **modernize**: Modern C++ best practices
- **performance**: Performance optimizations
- **portability**: Portability issues
- **readability**: Code readability

### Disabled Rules
Some rules are disabled to reduce noise:
- `modernize-use-trailing-return-type`: Style preference
- `readability-magic-numbers`: Too strict for game code
- `cppcoreguidelines-avoid-magic-numbers`: Same as above
- `readability-identifier-length`: Unreal Engine uses short names
- And others specific to Unreal Engine constraints

## Warnings as Errors
The configuration treats all warnings as errors (`WarningsAsErrors: '*'`) to ensure code quality.

## Header Filtering
The `HeaderFilterRegex: '.*'` ensures headers are also checked.

## Format Style
Uses project's `.clang-format` for automatic formatting fixes.
