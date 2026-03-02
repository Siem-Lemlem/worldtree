# 🌳 wrldtree

A fast C utility that automatically generates and maintains directory tree visualizations in your README.md files and can even **restructure your filesystem** to match a tree you define.

**Stop manually updating your project structure. Let `wrldtree` do it for you.**

---

## Quick Install

### Linux / Mac / WSL

```bash
git clone https://github.com/Siem-Lemlem/worldtree.git
cd worldtree
make install
```

Or one-liner:

```bash
curl -sSL https://raw.githubusercontent.com/Siem-Lemlem/worldtree/main/install.sh | bash
```

### Windows

```bash
git clone https://github.com/Siem-Lemlem/worldtree.git
cd worldtree
install.bat
```

Or with MinGW:

```bash
gcc -Wall -O2 wrldtree.c -o wrldtree.exe -std=gnu11
```

> ⚠️ **Windows PATH warning:** Do NOT use `setx PATH "%PATH%;..."` — it has a 1024-character limit and will silently truncate your existing PATH, breaking Node, Python, Git, and more.
>
> Instead: create `C:\bin`, copy `wrldtree.exe` there, then add `C:\bin` via **Win + R → sysdm.cpl → Advanced → Environment Variables → Path → Edit → New**.

---

## Usage

```bash
wrldtree [PATH] [FLAGS]

FLAGS:
  --print      Display tree in terminal only (dry run, no file changes)
  --depth N    Limit recursion depth (default: 5)
  --id N       Target a specific WRLDTREE block by ID
  --change     Sync your filesystem to match the INVOLVE tree in README
  --help       Show help menu

EXAMPLES:
  wrldtree                    Update default block in README.md
  wrldtree --print            Preview tree without modifying files
  wrldtree src --depth 3      Show src/ directory, max 3 levels deep
  wrldtree --id 2             Update block marked <!-- WRLDTREE START 2 -->
  wrldtree --change           Sync filesystem to your INVOLVE tree
```

---

## Features

### Auto-updating README trees

Add these markers anywhere in your `README.md`:

```markdown
<!-- WRLDTREE START -->
<!-- WRLDTREE END -->
```

Run `wrldtree` and the tree is injected between them automatically. Run it again anytime to keep it fresh.

If the tags don't exist, `wrldtree` appends a `## Project Structure` section to the end of your README automatically.

**Multiple trees** - use numbered IDs to maintain several independent trees in the same README:

```markdown
<!-- WRLDTREE START 2 -->
<!-- WRLDTREE END 2 -->
```

```bash
wrldtree src --id 2   # updates only block 2
```

---

### Filesystem sync with `--change`

This is the power feature. Define a target structure in your README using INVOLVE tags:

```markdown
<!-- WRLDTREE INVOLVE START -->
```text
. (root/)
├── src/
│   ├── main.c
│   └── utils.c
├── include/
│   └── utils.h
└── README.md
` ` `
<!-- WRLDTREE INVOLVE END -->
```

Run:

```bash
wrldtree --change
```

wrldtree will:

1. **Create** any directories and files that are missing
2. **Move** existing files to new locations if the filename matches using smart relocation
3. **Quarantine** anything that doesn't belong into an `extras/` folder — nothing is ever deleted

This lets you redesign your project structure in markdown and have your filesystem follow.

---

### 🔍 Smart filtering

wrldtree automatically ignores the following so they never appear in your trees:

- Hidden files and folders (anything starting with `.`)
- `node_modules`, `bin`, `obj`, `target`, `dist`
- `.git`, `__pycache__`, `.DS_Store`
- `extras/` (the quarantine folder used by `--change`)
- The `wrldtree` binary itself

---

## Features at a glance

| Feature | Details |

|---|---|
| Zero dependencies | Single C file, compiles in seconds |
| Cross-platform | Windows, Linux, Mac |
| Auto-inject | Writes tree between your markers |
| Auto-append | Creates `## Project Structure` if no markers found |
| Multiple trees | Numbered `--id` blocks |
| Filesystem sync | `--change` restructures your project to match a defined tree |
| Smart relocation | Moves files by filename match, never blind-deletes |
| Safe quarantine | Extras go to `extras/`, not the trash |
| Smart filtering | Ignores build artifacts, hidden files, and common junk |
| Sorted output | Alphabetical for consistency |
| Fast | Written in C |

---

## Known limitations

- `--change` detects files to relocate by **filename only** — if you have two files with the same name in different directories, it will match the first one it finds
- Max tree depth for filesystem sync is 128 levels
- On Linux, if you're compiling manually, use `-std=gnu11` or `-D_GNU_SOURCE`

---

## Contributing

PRs welcome. Current roadmap ideas:

- `.wrldtreeignore` config file support
- Custom ignore patterns via CLI flag
- Colorized terminal output
- JSON / XML output formats
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

## License

MIT — see [LICENSE](LICENSE) for details.

---

Built by a developer who got tired of manually updating README trees.
**If `wrldtree` saves you time, star the repo ⭐**
