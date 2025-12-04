# 🚀 Quick Git & GitHub Guide

## 📝 Daily Workflow

### Making Changes
```bash
# 1. Make your code changes
# 2. Stage all changes
git add .

# 3. Commit with a clear message
git commit -m "Add: New feature description"

# 4. Push to GitHub
git push origin main
```

**What happens:** GitHub Actions runs build test (1-2 min) ✅ or ❌

---

## 🔄 What Triggers What?

| Action | Trigger | What Happens | Auto Deploy? |
|--------|---------|--------------|--------------|
| `git push` to main | Build workflow | Tests if code compiles | ❌ No |
| Open Pull Request | Build workflow | Tests PR code | ❌ No |
| `git push` tag (v*) | Release workflow | Creates .deb package + GitHub Release | ✅ Yes |

---

## 📦 Creating a Release

```bash
# 1. Update CHANGELOG.md with new version
# 2. Commit and push changes
git add CHANGELOG.md
git commit -m "Update: Changelog for v1.0.2"
git push origin main

# 3. Create and push tag
git tag -a v1.0.2 -m "Release v1.0.2"
git push origin v1.0.2
```

**Result:** 
- GitHub automatically creates a release
- .deb package is built and attached
- Go to GitHub → Releases to see it

---

## 🔍 Pull Request Review

When someone submits a PR:

1. **Automatic:** Build workflow tests their code
2. **You review:** Check code in GitHub
3. **You decide:** 
   - Request changes, OR
   - Approve and click "Merge"

**No one can merge without your approval!** 🔒

---

## 🏗️ Project Structure

```
system_analyser/
├── src/              # All .c source files
├── include/          # All .h header files
├── scripts/          # build.sh, build_deb.sh
├── .github/          # GitHub workflows & templates
│   ├── workflows/    # build.yml, release.yml
│   └── ISSUE_TEMPLATE/
├── CONTRIBUTING.md   # How to contribute
├── CHANGELOG.md      # Version history
└── README.md         # Main documentation
```

---

## 🛠️ Local Development

```bash
# Build and test locally
make clean
make
./system-analyser

# Or use the script
./scripts/build.sh

# Build .deb package
./scripts/build_deb.sh
```

---

## ⚡ Quick Reference

### Commit Message Format
```
Add: New feature
Fix: Bug fix
Update: Improvements
Refactor: Code restructure
Docs: Documentation changes
```

### Check Build Status
- Go to GitHub → Actions tab
- See ✅ green checkmark or ❌ red X

### GitHub Settings (One-time setup)
1. **Settings → Branches**
   - Add protection rule for `main`
   - Require PR reviews before merging

2. **Settings → General**
   - Add description
   - Add topics: `raspberry-pi` `gtk3` `system-monitor`

---

## 🎯 Key Points

✅ **Build tests run automatically** on every push  
✅ **You control all merges** - nothing merges without you  
✅ **Releases are semi-automatic** - you push tag, GitHub builds .deb  
✅ **All contributor guidelines** are documented in CONTRIBUTING.md

**Questions?** Check CONTRIBUTING.md or open an issue!
