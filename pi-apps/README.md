# Pi-Apps Submission for System Analyser

This directory contains all necessary files to submit System Analyser to Pi-Apps.

## 📁 Directory Structure

```
pi-apps/
├── install          # Installation script (executable)
├── uninstall        # Uninstallation script (executable)
├── description      # Detailed app description
├── credits          # Developer and contributor credits
├── website          # Project website URL
├── icon-64.png      # App icon (64x64px or larger, square)
└── README.md        # This file
```

## ✅ Pi-Apps Submission Checklist

### Requirements Met:

- [x] **App is useful for Raspberry Pi desktop users**
  - System monitoring is essential for desktop Pi users
  - Real-time performance tracking
  - Temperature monitoring to prevent throttling

- [x] **App has GUI interface** (not command-line only)
  - Native GTK3 graphical interface
  - Integrates with desktop environment

- [x] **App is legal and appropriate**
  - Open source (MIT License)
  - No piracy or malicious content
  - Appropriate for all ages

- [x] **Multi-architecture support**
  - ✅ ARM32 (armhf) support
  - ✅ ARM64 (aarch64) support
  - Detects architecture automatically

- [x] **Installation method**
  - Uses .deb packages from GitHub Releases
  - Dependencies managed via `install_packages`
  - Clean uninstallation

### Files Checklist:

- [x] `install` script (executable, tested)
- [x] `uninstall` script (executable, tested)
- [x] `description` (comprehensive, user-friendly)
- [x] `credits` (acknowledges contributors)
- [x] `website` (GitHub repository URL)
- [x] `icon-64.png` (731x731px, suitable for scaling)

## 🚀 How to Submit

### Method 1: Official Submission (Recommended)

1. **Create GitHub Release** (if not done):
   ```bash
   # Tag and push
   git tag -a v1.0.1 -m "Release v1.0.1 for Pi-Apps"
   git push origin v1.0.1
   
   # GitHub Actions will create release with .deb files
   ```

2. **Verify release contains**:
   - `system-analyser_1.0.1-1_armhf.deb`
   - `system-analyser_1.0.1-1_arm64.deb`

3. **Create submission issue**:
   - Go to: https://github.com/Botspot/pi-apps/issues/new
   - Choose template: "App submission"
   - Title: `[App Submission] System Analyser - Real-time system monitor`
   - Include:
     - Link to this repository
     - Link to pi-apps/ directory
     - Screenshots from README
     - Explanation of usefulness

4. **Attach files**:
   - Create a ZIP of the pi-apps/ directory:
     ```bash
     cd /home/bthts/Projects/system_analyser
     zip -r system-analyser-pi-apps.zip pi-apps/
     ```
   - Attach to the issue

### Method 2: Pull Request

1. **Fork Pi-Apps repository**:
   ```bash
   git clone https://github.com/Botspot/pi-apps
   cd pi-apps
   ```

2. **Create app directory**:
   ```bash
   mkdir -p apps/"System Analyser"
   cp /home/bthts/Projects/system_analyser/pi-apps/* apps/"System Analyser"/
   ```

3. **Test locally**:
   ```bash
   ~/pi-apps/createapp # Test the app structure
   ```

4. **Create Pull Request**:
   - Branch: `add-system-analyser`
   - Title: `Add System Analyser - Real-time system monitoring for Raspberry Pi`
   - Description: Reference the app features and compatibility

## 🧪 Testing Before Submission

### Test Installation:
```bash
cd /home/bthts/Projects/system_analyser/pi-apps
./install
```

### Verify:
- [ ] App installs without errors
- [ ] Icon appears in application menu
- [ ] App launches correctly
- [ ] No missing dependencies
- [ ] Works on both 32-bit and 64-bit systems (if possible)

### Test Uninstallation:
```bash
./uninstall
```

### Verify:
- [ ] App uninstalls cleanly
- [ ] No errors during removal
- [ ] User data preserved (~/.local/share/system-analyser/)
- [ ] Menu entry removed

## 📝 Submission Template

Use this template when creating the submission issue:

```markdown
## App Name
System Analyser

## Category
System / Monitor

## Description
A lightweight GTK3-based real-time system monitoring application for Raspberry Pi with CPU, temperature, memory tracking, and CSV logging.

## Why is this useful?
- Helps users monitor their Pi's performance in real-time
- Prevents thermal throttling by tracking temperature
- Perfect for desktop Pi users who need system visibility
- Lightweight alternative to heavyweight monitoring tools
- Useful for both beginners and advanced users

## Architecture Support
- ✅ 32-bit (armhf)
- ✅ 64-bit (arm64)

## Installation Method
Downloads latest .deb package from GitHub Releases based on detected architecture.

## Links
- Repository: https://github.com/bathiatuss/system_analyser
- Releases: https://github.com/bathiatuss/system_analyser/releases
- Issue Tracker: https://github.com/bathiatuss/system_analyser/issues

## Screenshots
[Include screenshot from README]

## App Files
[Attach system-analyser-pi-apps.zip]

## Testing
- ✅ Tested on Raspberry Pi OS (armhf)
- ✅ Tested on Raspberry Pi OS (arm64)
- ✅ Install script works correctly
- ✅ Uninstall script works correctly
- ✅ No conflicts with existing packages

## License
MIT License

## Developer
Atakan B. (@bathiatuss)
```

## 📋 Pre-Submission Checklist

Before submitting, ensure:

- [ ] GitHub repository is public
- [ ] Latest release (v1.0.1+) is published with .deb files
- [ ] README has screenshots and clear description
- [ ] LICENSE file is present
- [ ] All pi-apps/ scripts are tested
- [ ] App works on actual Raspberry Pi hardware
- [ ] No console errors or warnings during usage
- [ ] Dependencies are properly declared
- [ ] Icon displays correctly
- [ ] Desktop entry file works

## 🔍 Expected Review Process

1. **Initial Review** (1-7 days)
   - Pi-Apps maintainers review the submission
   - May ask questions or request changes

2. **Testing Phase**
   - Maintainers test on various Pi models
   - Check for compatibility issues

3. **Approval & Merge**
   - Once approved, added to Pi-Apps repository
   - Appears in next Pi-Apps update

4. **User Availability**
   - Users can install via Pi-Apps GUI
   - Automatic updates via Pi-Apps updater

## 🛠️ Maintenance

After acceptance:

- Keep GitHub releases updated
- Respond to Pi-Apps issues related to your app
- Update scripts if installation method changes
- Monitor Pi-Apps discussions for feedback

## 📞 Support

For Pi-Apps submission questions:
- Discord: https://discord.gg/RXSTvaUvuu
- GitHub: https://github.com/Botspot/pi-apps/issues

For System Analyser issues:
- GitHub Issues: https://github.com/bathiatuss/system_analyser/issues
