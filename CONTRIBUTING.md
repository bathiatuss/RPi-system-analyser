# Contributing to System Analyser

Thank you for your interest in contributing to System Analyser! We welcome contributions from the community.

## How to Contribute

### Reporting Bugs

If you find a bug, please open an issue with:
- A clear title and description
- Steps to reproduce the issue
- Expected vs actual behavior
- Your environment (OS, Raspberry Pi model, GTK version)
- Screenshots if applicable

### Suggesting Features

We welcome feature suggestions! Please:
- Check if the feature already exists or has been requested
- Clearly describe the feature and its use case
- Explain why this feature would be useful

### Pull Requests

1. **Fork the repository**
   ```bash
   git clone https://github.com/bathiatuss/system_analyser.git
   cd system_analyser
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow the existing code style
   - Add comments for complex logic
   - Test your changes thoroughly

4. **Build and test**
   ```bash
   make clean
   make
   ./system-analyser
   ```

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add: Brief description of your changes"
   ```
   
   Commit message format:
   - `Add:` for new features
   - `Fix:` for bug fixes
   - `Update:` for improvements
   - `Refactor:` for code restructuring
   - `Docs:` for documentation

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Go to the original repository
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template

## Development Setup

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install build-essential libgtk-3-dev pkg-config git
```

### Project Structure

```
system_analyser/
├── src/           # Source code (.c files)
├── include/       # Header files (.h files)
├── scripts/       # Build scripts
├── assets/        # Icons and graphics
└── debian/        # Debian packaging
```

### Building

```bash
# Quick build
make

# Clean build
make clean && make

# Install locally
sudo make install

# Build Debian package
./scripts/build_deb.sh
```

## Code Style Guidelines

- Use **4 spaces** for indentation (no tabs)
- Use descriptive variable names
- Add comments for non-obvious code
- Keep functions focused and small
- Follow existing naming conventions:
  - Functions: `snake_case`
  - Structs: `PascalCase`
  - Constants: `UPPER_CASE`

## Testing

Before submitting a PR:
- [ ] Code compiles without errors
- [ ] No new compiler warnings
- [ ] Application runs without crashes
- [ ] All features work as expected
- [ ] Tested on Raspberry Pi (if possible)

## Getting Help

- Open an issue for questions
- Check existing issues and PRs
- Read the documentation in `/docs`

## Code of Conduct

Please be respectful and constructive. We aim to maintain a welcoming community.

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

**Thank you for contributing to System Analyser!** 🎉
