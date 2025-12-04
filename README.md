# System Analyser

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi-red.svg)](https://www.raspberrypi.org/)
[![GTK](https://img.shields.io/badge/GTK-3.0-blue.svg)](https://www.gtk.org/)

A lightweight GTK3-based real-time system monitoring application for Raspberry Pi. Displays CPU usage, temperature, memory, and other system metrics with CSV logging capabilities.

![System Analyser Screenshot](assets/logo_single_no_bg.png)

## ✨ Features

- Real-time CPU usage and temperature monitoring
- Memory and swap usage tracking
- System uptime and load average display
- CPU clock speed and voltage monitoring
- Data logging to CSV files
- Native GTK3 interface

## 📋 Requirements

- Raspberry Pi (all models supported)
- Raspbian/Raspberry Pi OS
- GTK+ 3.0
- GCC compiler

## 🚀 Quick Start

### Option 1: Build from Source

1. **Install dependencies:**
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential libgtk-3-dev pkg-config
   ```

2. **Clone and build:**
   ```bash
   git clone https://github.com/bathiatuss/system_analyser.git
   cd system_analyser
   make
   ```

3. **Run:**
   ```bash
   ./system-analyser
   ```

### Option 2: Install System-wide

```bash
make
sudo make install
```

### Option 3: Build Debian Package

```bash
./scripts/build_deb.sh
sudo dpkg -i ../system-analyser_*.deb
```

## Installation

### From .deb Package

```bash
sudo dpkg -i system-analyser_1.0.0-1_armhf.deb
sudo apt-get install -f  # Fix any dependency issues
```

### From Source

```bash
make
sudo make install
```

## Usage

Launch from the application menu or run from terminal:

```bash
system-analyser
```

Data is logged to `~/.local/share/system-analyser/system_metrics.csv`

## Uninstallation

```bash
# If installed via .deb
sudo apt-get remove system-analyser

# If installed via make
sudo make uninstall
```

## Publishing to Raspberry Pi Repository

To submit this package to the official Raspberry Pi repository:

1. **Prepare your package:**
   - Ensure all licensing is clear (GPL-3+)
   - Test on multiple Raspberry Pi models
   - Update maintainer information in `debian/control` and `debian/changelog`

2. **Contact Raspberry Pi:**
   - Visit: https://www.raspberrypi.com/
   - Contact their package maintainers
   - Or submit via GitHub: https://github.com/RPi-Distro

3. **Alternative: PPA or Third-party Repository**
   - Create a Launchpad PPA
   - Host your own APT repository
   - Publish on GitHub Releases

## License

MIT License - Open for community contributions

## Author

Atakan B. <atakanbaltac-1@hotmail.com>

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details on:
- How to report bugs
- How to suggest features
- How to submit pull requests
- Code style guidelines

## 📝 Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and changes.

## 🐛 Known Issues

- Some sensor readings may not be available on all Raspberry Pi models
- Requires read access to `/sys/class/thermal/` for temperature monitoring

## 📚 Documentation

- [Project Structure](PROJECT_STRUCTURE.md) - Folder organization
- [Contributing Guide](CONTRIBUTING.md) - How to contribute

## 🔗 Links

- **Repository**: https://github.com/bathiatuss/system_analyser
- **Issues**: https://github.com/bathiatuss/system_analyser/issues
- **Pull Requests**: https://github.com/bathiatuss/system_analyser/pulls

## 💬 Support

- Open an [issue](https://github.com/bathiatuss/system_analyser/issues) for bugs
- Start a [discussion](https://github.com/bathiatuss/system_analyser/discussions) for questions

## ⭐ Star History

If you find this project useful, please consider giving it a star!
