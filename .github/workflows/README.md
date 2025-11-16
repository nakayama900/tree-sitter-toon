# GitHub Actions Workflows

This directory contains automated workflows for CI/CD.

## Workflows

### 1. CI (`ci.yml`)

**Triggers**: Push to main/master/develop, Pull Requests

**What it does**:
- Runs tests on multiple platforms (Ubuntu, macOS, Windows)
- Tests with Node.js 18 and 20
- Builds Python package
- Lints code and checks grammar

**Status Badge**:
```markdown
![CI](https://github.com/3swordman/tree-sitter-toon/workflows/CI/badge.svg)
```

### 2. Publish to PyPI (`publish.yml`)

**Triggers**:
- Automatically on GitHub Release (publishes to PyPI)
- Manually via workflow dispatch (choose Test PyPI or PyPI)

**What it does**:
- Builds Python package
- Checks package integrity
- Publishes to Test PyPI or PyPI
- Uploads artifacts

**Manual Trigger**:
1. Go to Actions tab
2. Select "Publish to PyPI"
3. Click "Run workflow"
4. Choose destination (testpypi/pypi)

### 3. Create Release (`release.yml`)

**Triggers**:
- Automatically when you push a version tag (e.g., `v0.1.0`)
- Manually via workflow dispatch

**What it does**:
- Generates changelog from git commits
- Creates GitHub Release
- Marks as pre-release if version contains alpha/beta/rc

## Setup Required

### 1. PyPI API Tokens

Add these secrets to your repository:

1. Go to: **Repository Settings → Secrets and variables → Actions**
2. Add secrets:
   - `TEST_PYPI_API_TOKEN` - From https://test.pypi.org/manage/account/token/
   - `PYPI_API_TOKEN` - From https://pypi.org/manage/account/token/

To generate tokens:
- **Test PyPI**: https://test.pypi.org/manage/account/token/
- **PyPI**: https://pypi.org/manage/account/token/

### 2. Repository Permissions

Ensure the repository has these permissions:
- **Settings → Actions → General → Workflow permissions**
- Select: "Read and write permissions"
- Check: "Allow GitHub Actions to create and approve pull requests"

## Usage Examples

### Publishing a Release

**Method 1: Tag-based (Automatic)**
```bash
# Update version in pyproject.toml to 0.1.0

git add pyproject.toml
git commit -m "Bump version to 0.1.0"
git tag v0.1.0
git push origin main
git push origin v0.1.0

# This triggers:
# 1. create-release.yml → Creates GitHub Release
# 2. publish.yml → Publishes to PyPI
```

**Method 2: Manual Release**
1. Go to Actions → "Create Release"
2. Click "Run workflow"
3. Enter version (e.g., `v0.1.0`)
4. Wait for release creation
5. This will trigger publish workflow

**Method 3: Test Publishing**
1. Go to Actions → "Publish to PyPI"
2. Click "Run workflow"
3. Select "testpypi"
4. Run workflow
5. Test installation from Test PyPI

### Checking CI Status

All pull requests and pushes automatically run CI:
- Tests on multiple platforms
- Python package build
- Code linting

View status:
- Pull Request → Checks tab
- Actions tab → CI workflow

### Monitoring Workflows

1. **Actions Tab**: See all workflow runs
2. **Commit Status**: Green checkmark or red X on commits
3. **Email Notifications**: Configure in GitHub settings

## Workflow Files

```
.github/workflows/
├── ci.yml          # Continuous Integration
├── publish.yml     # PyPI Publishing
├── release.yml     # GitHub Release Creation
└── README.md       # This file
```

## Customization

### Modify Test Matrix

Edit `ci.yml`:
```yaml
strategy:
  matrix:
    os: [ubuntu-latest, macos-latest, windows-latest]
    node-version: [18, 20]
    # Add more versions or platforms
```

### Change Publish Trigger

Edit `publish.yml`:
```yaml
on:
  release:
    types: [published]  # Change to [created, edited, published]
```

### Disable Workflow

To temporarily disable:
1. Go to Actions tab
2. Select workflow
3. Click "..." → "Disable workflow"

Or add to workflow file:
```yaml
on:
  workflow_dispatch:  # Manual only
```

## Troubleshooting

### "Resource not accessible by integration"
- Fix: Enable write permissions in Settings → Actions → General

### "Secret not found"
- Fix: Add PYPI_API_TOKEN and TEST_PYPI_API_TOKEN in Settings → Secrets

### Build fails on Windows
- Common: Line ending issues (CRLF vs LF)
- Fix: Add `.gitattributes`:
  ```
  * text=auto
  *.sh text eol=lf
  ```

### Package upload fails
- Check: Version in `pyproject.toml` isn't already published
- Fix: Increment version number

## Testing Workflows Locally

Use [act](https://github.com/nektos/act) to test workflows locally:

```bash
# Install act
brew install act  # macOS
# or
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Run CI workflow
act -j test

# Run with specific event
act release -e event.json
```

## Status Badges

Add to README.md:

```markdown
[![CI](https://github.com/3swordman/tree-sitter-toon/workflows/CI/badge.svg)](https://github.com/3swordman/tree-sitter-toon/actions/workflows/ci.yml)
[![Publish](https://github.com/3swordman/tree-sitter-toon/workflows/Publish%20to%20PyPI/badge.svg)](https://github.com/3swordman/tree-sitter-toon/actions/workflows/publish.yml)
[![PyPI version](https://badge.fury.io/py/tree-sitter-toon.svg)](https://badge.fury.io/py/tree-sitter-toon)
```

## Security

- Never commit API tokens
- Use repository secrets for sensitive data
- Review workflow runs for exposed credentials
- Limit workflow permissions to minimum required

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Publishing to PyPI](https://packaging.python.org/guides/publishing-package-distribution-releases-using-github-actions-ci-cd-workflows/)
