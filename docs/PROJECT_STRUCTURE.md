# Project Structure

This document describes the folder structure of the System Analyser project.

## Directory Layout

```
system_analyser/
├── src/                    # Source code files (.c)
│   ├── main_gui.c         # Main GTK GUI entry point
│   ├── gui.c              # GUI implementation
│   ├── sensors.c          # System sensor reading
│   └── logger.c           # CSV logging functionality
│
├── include/               # Header files (.h)
│   ├── gui.h             # GUI definitions and functions
│   ├── sensors.h         # Sensor reading functions
│   └── logger.h          # Logger interface
│
├── assets/               # Graphics and icons
│   ├── icon.png
│   ├── logo.png
│   ├── logo_single.png
│   ├── logo_single_no_bg.png
│   └── logo_single_w_offset.png
│
├── scripts/              # Build and utility scripts
│   ├── build.sh         # Quick build script
│   └── build_deb.sh     # Debian package build script
│
├── debian/               # Debian packaging files
│   ├── control          # Package metadata
│   ├── rules            # Build rules
│   ├── changelog        # Package changelog
│   └── ...
│
├── docs/                 # Documentation (reserved)
│
├── Makefile             # Build configuration
├── README.md            # Project documentation
├── LICENSE              # MIT License
├── RELEASE_NOTES_v1.0.1.md
└── system-analyser.desktop  # Desktop entry file
```

## Building the Project

### Quick Build
```bash
make
./system-analyser
```

### Using Build Script
```bash
./scripts/build.sh
```

### Building Debian Package
```bash
./scripts/build_deb.sh
```

## Key Changes from Previous Structure

The project has been reorganized for better maintainability:

1. **Source separation**: All `.c` files moved to `src/`
2. **Header organization**: All `.h` files moved to `include/`
3. **Script consolidation**: Build scripts moved to `scripts/`
4. **Clean root**: Root directory now contains only essential configuration files

## Compiler Flags

The Makefile includes `-Iinclude` flag to properly locate header files:
```makefile
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -O2 -Iinclude
```

## Runtime Data

The application creates logs in:
```
~/.local/share/system-analyser/system_metrics.csv
```
