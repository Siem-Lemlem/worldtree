#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define MAX_PATH_LEN 8192

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <Windows.h>
    #include <direct.h>
    #define PATH_SEP "\\"
#else
    #include <unistd.h>
    #define PATH_SEP "/"
#endif

#define TEMP_TREE_FILE "tree_temp.txt"
#define EXTRAS_DIR "extras"
#define INVOLVE_START "<!-- WRLDTREE INVOLVE START -->"
#define INVOLVE_END "<!-- WRLDTREE INVOLVE END -->"

/* =========================
   Utility
   ========================= */

void join_path(char *dest, const char *a, const char *b) {
    snprintf(dest, MAX_PATH_LEN, "%s%s%s", a, PATH_SEP, b);
}

void mkdir_recursive(const char *path) {
    char tmp[4096];
    snprintf(tmp, sizeof(tmp), "%s", path);

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
#ifdef _WIN32
            _mkdir(tmp);
#else
            mkdir(tmp, 0755);
#endif
            *p = PATH_SEP[0];
        }
    }

#ifdef _WIN32
    _mkdir(tmp);
#else
    mkdir(tmp, 0755);
#endif
}

/* =========================
   Ignore Filters
   ========================= */

int should_skip(const char *name) {
    if (!name || name[0] == '.') return 1;

    const char *ignore[] = {
        "node_modules", "bin", "obj", "target", "dist", ".git",
        "__pycache__", ".DS_Store",
        TEMP_TREE_FILE, "wrldtree.exe", "wrldtree",
        EXTRAS_DIR,
        NULL
    };

    for (int i = 0; ignore[i]; i++)
        if (strcmp(name, ignore[i]) == 0)
            return 1;

    return 0;
}

int compare_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* =========================
   Tree Generation
   ========================= */

typedef struct {
    int max_depth;
    FILE *out;
} Config;

void walk_tree(const char *path, int depth, Config *cfg, int *pipe_mask) {
    if (cfg->max_depth != -1 && depth > cfg->max_depth)
        return;

    DIR *dir = opendir(path);
    if (!dir) return;

    char **entries = NULL;
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir))) {
        if (should_skip(entry->d_name)) continue;

        char **tmp = realloc(entries, sizeof(char *) * (count + 1));
        if (!tmp) break;
        entries = tmp;
        entries[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(entries, count, sizeof(char *), compare_names);

    for (int i = 0; i < count; i++) {
        int is_last = (i == count - 1);

        for (int d = 0; d < depth; d++)
            fprintf(cfg->out, pipe_mask[d] ? "│   " : "    ");

        char full_path[4096];
        join_path(full_path, path, entries[i]);

        struct stat st;
        int is_dir = (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode));

        fprintf(cfg->out, "%s %s%s\n",
                is_last ? "└──" : "├──",
                entries[i],
                is_dir ? "/" : "");

        if (is_dir) {
            pipe_mask[depth] = !is_last;
            walk_tree(full_path, depth + 1, cfg, pipe_mask);
        }
    }

    for (int i = 0; i < count; i++) free(entries[i]);
    free(entries);
}

char *generate_tree(const char *root, int depth) {
    FILE *temp = fopen(TEMP_TREE_FILE, "w");
    if (!temp) return NULL;

    Config cfg = { depth, temp };
    int pipe_mask[128] = {0};

    fprintf(temp, ". (%s/)\n", strcmp(root, ".") == 0 ? "root" : root);
    walk_tree(root, 0, &cfg, pipe_mask);
    fclose(temp);

    FILE *f = fopen(TEMP_TREE_FILE, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    remove(TEMP_TREE_FILE);
    return buf;
}

/* =========================
   README Handling
   ========================= */

char *extract_tree_from_readme(const char *readme, int id) {
    FILE *f = fopen(readme, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);

    char start[64], end[64];
    if (id == 1) {
        strcpy(start, "<!-- WRLDTREE START -->");
        strcpy(end, "<!-- WRLDTREE END -->");
    } else {
        snprintf(start, sizeof(start), "<!-- WRLDTREE START %d -->", id);
        snprintf(end, sizeof(end), "<!-- WRLDTREE END %d -->", id);
    }

    char *s = strstr(content, start);
    char *e = strstr(content, end);
    if (!s || !e || e <= s) {
        free(content);
        return NULL;
    }

    char *code = strstr(s, "```");
    if (!code) { free(content); return NULL; }
    code = strchr(code + 3, '\n');
    if (!code) { free(content); return NULL; }
    code++;

    char *code_end = strstr(code, "```");
    if (!code_end) { free(content); return NULL; }

    *code_end = '\0';
    char *tree = strdup(code);
    free(content);
    return tree;
}

char *extract_involve_tree(const char *readme) {
    FILE *f = fopen(readme, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);

    char *s = strstr(content, INVOLVE_START);
    char *e = strstr(content, INVOLVE_END);
    if (!s || !e || e <= s) {
        free(content);
        return NULL;
    }

    char *code = strstr(s, "```");
    if (!code) { free(content); return NULL; }
    code = strchr(code + 3, '\n');
    if (!code) { free(content); return NULL; }
    code++;

    char *code_end = strstr(code, "```");
    if (!code_end) { free(content); return NULL; }

    *code_end = '\0';
    char *tree = strdup(code);
    free(content);
    return tree;
}

void update_readme(const char *readme, const char *tree, int id) {
    FILE *f = fopen(readme, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);

    char start[64], end[64];
    if (id == 1) {
        strcpy(start, "<!-- WRLDTREE START -->");
        strcpy(end, "<!-- WRLDTREE END -->");
    } else {
        snprintf(start, sizeof(start), "<!-- WRLDTREE START %d -->", id);
        snprintf(end, sizeof(end), "<!-- WRLDTREE END %d -->", id);
    }

    char *s = strstr(content, start);
    char *e = strstr(content, end);

    FILE *out = fopen(readme, "wb");
    if (!out) { free(content); return; }

    if (s && e && e > s) {
        fwrite(content, 1, s - content, out);
        fprintf(out, "%s\n```text\n%s```\n%s", start, tree, end);
        fprintf(out, "%s", e + strlen(end));
        printf("✓ README updated\n");
    } else {
        fprintf(out, "%s\n\n%s\n```text\n%s```\n%s\n",
                content, start, tree, end);
        printf("⚠ Tags not found. Appended structure to end of README.\n");
    }

    fclose(out);
    free(content);
}

/* =========================
   README → Filesystem Sync
   ========================= */

void collect_existing(const char *root, char ***list, int *count) {
    DIR *dir = opendir(root);
    if (!dir) return;

    struct dirent *e;
    while ((e = readdir(dir))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        if (should_skip(e->d_name)) continue;

        char full[4096];
        join_path(full, root, e->d_name);

        (*list) = realloc(*list, sizeof(char*) * (*count + 1));
        (*list)[(*count)++] = strdup(full);

        struct stat st;
        if (stat(full, &st) == 0 && S_ISDIR(st.st_mode))
            collect_existing(full, list, count);
    }
    closedir(dir);
}

int path_in_list(const char *p, char **list, int count) {
    for (int i = 0; i < count; i++)
        if (!strcmp(p, list[i])) return 1;
    return 0;
}

char **parse_tree_block(const char *text, int *count) {
    char **list = NULL;
    *count = 0;

    char *copy = strdup(text);
    char *line = strtok(copy, "\n");

    char *path_at_depth[128] = {0};

    while (line) {
        char *p = line;

        while (*p) {
            if (*p == ' ') {
                p++;
            } else if ((unsigned char)*p == 0xE2 && p[1] && p[2]) {
                p += 3;
            } else if (*p == '-') {
                p++;
            } else {
                break;
            }
        }

        int chars_skipped = p - line;
        int depth = chars_skipped / 4;

        if (strstr(p, ". (root/)") || strstr(p, ". (")) {
            line = strtok(NULL, "\n");
            continue;
        }

        if (!*p || *p == '\r' || *p == '\n') {
            line = strtok(NULL, "\n");
            continue;
        }

        char name[256] = "";
        int i = 0;
        while (p[i] && p[i] != '\r' && p[i] != '\n' && i < 255) {
            name[i] = p[i];
            i++;
        }
        name[i] = '\0';

        while (i > 0 && (name[i-1] == '/' || name[i-1] == ' ' ||
                         name[i-1] == '\r' || name[i-1] == '\n')) {
            name[--i] = '\0';
        }

        if (name[0] == '\0') {
            line = strtok(NULL, "\n");
            continue;
        }

        char full_path[4096] = "";
        for (int d = 0; d < depth; d++) {
            if (path_at_depth[d]) {
                if (full_path[0]) strcat(full_path, PATH_SEP);
                strcat(full_path, path_at_depth[d]);
            }
        }
        if (full_path[0]) strcat(full_path, PATH_SEP);
        strcat(full_path, name);

        list = realloc(list, sizeof(char*) * (*count + 1));
        list[(*count)++] = strdup(full_path);

        if (path_at_depth[depth]) free(path_at_depth[depth]);
        path_at_depth[depth] = strdup(name);

        for (int d = depth + 1; d < 128; d++) {
            if (path_at_depth[d]) {
                free(path_at_depth[d]);
                path_at_depth[d] = NULL;
            }
        }

        line = strtok(NULL, "\n");
    }

    for (int d = 0; d < 128; d++) {
        if (path_at_depth[d]) free(path_at_depth[d]);
    }
    free(copy);

    return list;
}

void normalize_path_sep(char *path) {
    for (char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = PATH_SEP[0];
        }
    }
}

void apply_change(const char *root, const char *tree_text) {
    int wanted_count;
    char **wanted = parse_tree_block(tree_text, &wanted_count);

    char **existing = NULL;
    int existing_count = 0;
    collect_existing(root, &existing, &existing_count);

    char extras[4096];
    join_path(extras, root, EXTRAS_DIR);
    mkdir_recursive(extras);

    printf("→ Syncing filesystem to README tree\n");

    for (int i = 0; i < wanted_count; i++) {
        char full[4096];
        join_path(full, root, wanted[i]);

        struct stat st;
        if (stat(full, &st) != 0) {
            if (strchr(wanted[i], '.')) {
                char *sep = strrchr(full, PATH_SEP[0]);
                if (sep) {
                    *sep = '\0';
                    mkdir_recursive(full);
                    *sep = PATH_SEP[0];
                }

                FILE *f = fopen(full, "w");
                if (f) {
                    fclose(f);
                    printf("✓ Created file: %s\n", wanted[i]);
                }
            } else {
                mkdir_recursive(full);
                printf("✓ Created dir: %s\n", wanted[i]);
            }
        }
    }

    for (int i = 0; i < existing_count; i++) {
        char rel[4096];
        snprintf(rel, sizeof(rel), "%s", existing[i] + strlen(root) + 1);

        if (!path_in_list(rel, wanted, wanted_count)) {
            char dest[4096];
            join_path(dest, extras, rel);

            char *sep = strrchr(dest, PATH_SEP[0]);
            if (sep) {
                *sep = 0;
                mkdir_recursive(dest);
                *sep = PATH_SEP[0];
            }

            if (rename(existing[i], dest) == 0)
                printf("⚠ Moved extra: %s\n", rel);
        }
    }

    for (int i = 0; i < wanted_count; i++) free(wanted[i]);
    for (int i = 0; i < existing_count; i++) free(existing[i]);
    free(wanted);
    free(existing);
}

void apply_involve_change(const char *root, const char *tree_text) {
    int wanted_count;
    char **wanted = parse_tree_block(tree_text, &wanted_count);

    printf("→ Syncing filesystem to INVOLVE tree structure\n");
    printf("  Found %d items in target structure\n\n", wanted_count);

    // Determine which paths are directories (have children)
    int *is_dir = calloc(wanted_count, sizeof(int));
    for (int i = 0; i < wanted_count; i++) {
        for (int j = 0; j < wanted_count; j++) {
            if (i == j) continue;
            int len = strlen(wanted[i]);
            if (strncmp(wanted[j], wanted[i], len) == 0 &&
                (wanted[j][len] == '/' || wanted[j][len] == '\\')) {
                is_dir[i] = 1;
                break;
            }
        }
    }

    // Pass 1: Create directories
    for (int i = 0; i < wanted_count; i++) {
        if (!is_dir[i]) continue;
        char full[4096];
        join_path(full, root, wanted[i]);
        struct stat st;
        if (stat(full, &st) != 0) {
            mkdir_recursive(full);
            printf("✓ Created dir: %s\n", wanted[i]);
        }
    }

    // Pass 2: Move existing files to new locations by filename match
    for (int i = 0; i < wanted_count; i++) {
        if (is_dir[i]) continue;

        char wanted_path[4096];
        join_path(wanted_path, root, wanted[i]);

        struct stat st;
        if (stat(wanted_path, &st) == 0) continue;

        char *filename = strrchr(wanted[i], PATH_SEP[0]);
        filename = filename ? filename + 1 : wanted[i];

        char **existing = NULL;
        int existing_count = 0;
        collect_existing(root, &existing, &existing_count);

        for (int j = 0; j < existing_count; j++) {
            char *existing_name = strrchr(existing[j], PATH_SEP[0]);
            existing_name = existing_name ? existing_name + 1 : existing[j];

            if (strcmp(filename, existing_name) == 0) {
                if (rename(existing[j], wanted_path) == 0) {
                    char rel_from[4096];
                    snprintf(rel_from, sizeof(rel_from), "%s", existing[j] + strlen(root) + 1);
                    printf("→ Moved: %s → %s\n", rel_from, wanted[i]);
                }
                break;
            }
        }

        for (int j = 0; j < existing_count; j++) free(existing[j]);
        free(existing);
    }

    // Pass 3: Create any still-missing files
    for (int i = 0; i < wanted_count; i++) {
        if (is_dir[i]) continue;
        char full[4096];
        join_path(full, root, wanted[i]);
        struct stat st;
        if (stat(full, &st) != 0) {
            FILE *f = fopen(full, "w");
            if (f) {
                fclose(f);
                printf("✓ Created file: %s\n", wanted[i]);
            } else {
                fprintf(stderr, "✗ Failed to create: %s\n", wanted[i]);
            }
        }
    }

    // Pass 4: Move anything not in wanted to extras/
    char **existing = NULL;
    int existing_count = 0;
    collect_existing(root, &existing, &existing_count);

    char extras[4096];
    join_path(extras, root, EXTRAS_DIR);
    int extras_created = 0;

    for (int i = 0; i < existing_count; i++) {
        const char *start = existing[i] + strlen(root);
        if (*start == '/' || *start == '\\') start++;

        char rel[4096];
        snprintf(rel, sizeof(rel), "%s", start);

        if (!path_in_list(rel, wanted, wanted_count)) {
            if (!extras_created) {
                mkdir_recursive(extras);
                extras_created = 1;
            }

            char dest[4096];
            join_path(dest, extras, rel);

            char *sep = strrchr(dest, PATH_SEP[0]);
            if (sep) {
                *sep = '\0';
                mkdir_recursive(dest);
                *sep = PATH_SEP[0];
            }

            if (rename(existing[i], dest) == 0)
                printf("⚠ Moved to extras: %s\n", rel);
        }
    }

    // FIX: normalize before free, not after
    for (int i = 0; i < wanted_count; i++) normalize_path_sep(wanted[i]);
    for (int i = 0; i < wanted_count; i++) free(wanted[i]);
    for (int i = 0; i < existing_count; i++) free(existing[i]);
    free(is_dir);
    free(wanted);
    free(existing);
}

/* =========================
   Main
   ========================= */

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    int depth = 5, id = 1;
    int dry = 0, change = 0;
    char *root = ".", *readme = "README.md";

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--print"))       dry = 1;
        else if (!strcmp(argv[i], "--change")) change = 1;
        else if (!strcmp(argv[i], "--depth") && i + 1 < argc)
            depth = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--id") && i + 1 < argc)
            id = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--help")) {
            printf("wrldtree: Keep your project documentation in sync\n\n");
            printf("Usage: wrldtree [PATH] [FLAGS]\n\n");
            printf("Flags:\n");
            printf("  --print      Display tree in terminal only (no file changes)\n");
            printf("  --depth N    Limit recursion depth (default: 5)\n");
            printf("  --id N       Target specific WRLDTREE block by ID\n");
            printf("  --change     Sync filesystem to match INVOLVE tree in README\n");
            printf("  --help       Show this help menu\n\n");
            printf("Examples:\n");
            printf("  wrldtree                    Update default block in README.md\n");
            printf("  wrldtree --print            Preview without modifying files\n");
            printf("  wrldtree src --depth 3      Show src/ directory, 3 levels deep\n");
            printf("  wrldtree --id 2             Update block <!-- WRLDTREE START 2 -->\n");
            printf("  wrldtree --change           Sync filesystem to INVOLVE tree\n\n");
            printf("INVOLVE tags:\n");
            printf("  <!-- WRLDTREE INVOLVE START -->\n");
            printf("  ```text\n");
            printf("  . (root/)\n");
            printf("  ├── src/\n");
            printf("  │   └── main.c\n");
            printf("  └── README.md\n");
            printf("  ```\n");
            printf("  <!-- WRLDTREE INVOLVE END -->\n");
            return 0;
        }
        else if (argv[i][0] != '-') root = argv[i];
    }

    printf("→ Scanning %s (depth: %d)...\n", root, depth);

    char *tree = generate_tree(root, depth);
    if (!tree) return 1;

    if (change) {
        char *involve_tree = extract_involve_tree(readme);
        if (!involve_tree) {
            fprintf(stderr, "Error: No INVOLVE tree found in README between tags:\n");
            fprintf(stderr, "  %s\n  %s\n", INVOLVE_START, INVOLVE_END);
            free(tree);
            return 1;
        }
        apply_involve_change(root, involve_tree);
        free(involve_tree);
    }
    else if (dry) {
        puts(tree);
        printf("→ Dry run complete. README.md was NOT modified.\n");
    }
    else {
        update_readme(readme, tree, id);
    }

    free(tree);
    return 0;
}