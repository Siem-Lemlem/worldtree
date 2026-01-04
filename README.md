# 🌳 wrldtree

A fast C utility that automatically generates and maintains directory tree visualizations in your README.md files.

**Stop manually updating your project structure.** Let `wrldtree` do it for you.

---

## Quick Install

### Linux / Mac / WSL

```bash
curl -sSL https://raw.githubusercontent.com/Siem-Lemlem/wrldtree/main/install.sh | bash
```

Or manually:

```bash
git clone https://github.com/Siem-Lemlem/wrldtree.git
cd wrldtree
make install
```

### Windows

```cmd
git clone https://github.com/Siem-Lemlem/wrldtree.git
cd wrldtree
install.bat
```

Or with MinGW:

```bash
gcc -Wall -O2 wrldtree.c -o wrldtree.exe -std=gnu11
```

---

## ⚠️ CRITICAL WARNING FOR WINDOWS USERS

**DO NOT use `setx PATH "%PATH%;..."` to add wrldtree to your PATH!**

This command has a **1024-character limit** and will **silently truncate your existing PATH**, breaking other programs like Python, Node.js, VS Code, Git, and more.

### ✅ Safe Installation Methods:

**Method 1: Use a Dedicated Bin Folder (Recommended)**

1. Create a bin folder:

```cmd
   mkdir C:\bin
```

2. Copy wrldtree.exe there:

```cmd
   copy wrldtree.exe C:\bin\
```

3. Add `C:\bin` to PATH using Windows GUI:

   - Press `Win + R` and type `sysdm.cpl`
   - Go to **Advanced** → **Environment Variables**
   - Under **User variables**, select **Path** and click **Edit**
   - Click **New** and add: `C:\bin`
   - Click **OK** on all dialogs
   - Restart your terminal

**Method 2: Just Use the Full Path**

```cmd
C:\path\to\wrldtree.exe --help
```

---

## 🚀 Quick Start

1. Add markers to your README.md:

```markdown
<!-- WRLDTREE START -->
<!-- WRLDTREE END -->
```

2. Run wrldtree:

```bash
wrldtree
```

3 Your project structure is now auto-documented!

---

## 📖 Usage

```bash
wrldtree [PATH] [FLAGS]

FLAGS:
  --print      Display tree in terminal only (dry run)
  --depth N    Limit recursion depth (default: 5)
  --id N       Target a specific WRLDTREE block by ID
  --help       Show help menu

EXAMPLES:
  wrldtree                    # Update default block in README.md
  wrldtree --print            # Preview tree without modifying files
  wrldtree src --depth 3      # Show src/ directory, max 3 levels deep
  wrldtree --id 2             # Update block marked <!-- WRLDTREE START 2 -->
```

---

## Features

- ✅ **Zero dependencies** - Single C file, compiles in seconds
- ✅ **Cross-platform** - Works on Windows, Linux, and Mac
- ✅ **Surgical injection** - Only updates content between markers
- ✅ **Multiple trees** - Support for numbered ID blocks
- ✅ **Smart filtering** - Automatically ignores `.git`, `node_modules`, etc.
- ✅ **Sorted output** - Alphabetical ordering for consistency
- ✅ **Fast** - Written in C, processes large projects instantly

---

## Test Coverage

All tests passing ✓

| Test | Command | Status |

|------|---------|--------|

| Compilation | `gcc -Wall wrldtree.c -o wrldtree` | ✓ |

| Help Menu | `./wrldtree --help` | ✓ |

| Preview | `./wrldtree --print` | ✓ |

| Depth Limit | `./wrldtree --depth 1` | ✓ |

| Subdirectory | `./wrldtree src --print` | ✓ |

| Missing Tags | `./wrldtree` (no tags) | ✓ |

| Existing Tags | `./wrldtree` (with tags) | ✓ |

| Multiple Blocks | `./wrldtree --id 2` | ✓ |

| Deep Nesting | `./wrldtree --depth 100` | ✓ |

| Invalid Path | `./wrldtree fake_dir` | ✓ |

| Legacy Support | `./wrldtree --id 1` | ✓ |
<<<<<<< HEAD

---
=======
>>>>>>> 291d0c09e94947da73b9b0f842e012ccaf884d4b

## Contributing

PRs welcome! Areas for improvement:

- Configuration file support (`.wrldtreeignore`)
- Custom ignore patterns via CLI
- Colorized output
- JSON/XML output formats
- Git integration (only show tracked files)

---

## Project Structure

<!-- WRLDTREE START -->
```text
. (root/)
├── install.bat
├── install.sh
├── LICENSE
├── Makefile
├── README.md
└── wrldtree.c
```
<!-- WRLDTREE END -->

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

## Acknowledgments

Built by a developer who got tired of manually updating README trees.

**If `wrldtree` saves you time, star the repo ⭐ and share it with other developers!**
