#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Cross-platform compatibility
#ifdef _WIN32
    #include <Windows.h>
    #define PATH_SEP "\\"
#else
    #define PATH_SEP "/"
#endif

typedef struct {
    int max_depth;
    FILE *out; 
} Config;

int should_skip(const char *name) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 1;
    if (name[0] == '.') return 1;

    const char *ignore[] = {
        "node_modules", "bin", "obj", "target", "dist", ".git",
        "__pycache__", ".DS_Store",
        "tree_temp.txt", "wrldtree.exe", "wrldtree",
        NULL
    };

    for (int i = 0; ignore[i] != NULL; i++) {
        if (strcmp(name, ignore[i]) == 0) return 1;
    }
    return 0;
}

int compare_names(const void *a, const void *b) {
    const char *name_a = *(const char **)a;
    const char *name_b = *(const char **)b;
    return strcmp(name_a, name_b);
}

void walk(const char *path, int depth, Config *cfg, int *pipe_mask) {
    if (cfg->max_depth != -1 && depth > cfg->max_depth) return;

    DIR *dir = opendir(path);
    if (!dir) {
        if (depth == 0) {
            fprintf(stderr, "Error: Cannot open directory '%s'\n", path);
            perror("Reason");
        }
        return;
    }

    char **names = NULL;
    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (should_skip(entry->d_name)) continue;
        
        char **temp = realloc(names, sizeof(char *) * (count + 1));
        if (!temp) {
            for (int j = 0; j < count; j++) free(names[j]);
            free(names);
            closedir(dir);
            fprintf(stderr, "Error: Memory allocation failed\n");
            return;
        }
        names = temp;
        
        names[count] = strdup(entry->d_name);
        if (!names[count]) {
            for (int j = 0; j < count; j++) free(names[j]);
            free(names);
            closedir(dir);
            fprintf(stderr, "Error: Memory allocation failed\n");
            return;
        }
        count++;
    }
    closedir(dir);

    if (names && count > 0) qsort(names, count, sizeof(char *), compare_names);

    for (int i = 0; i < count; i++) {
        int is_last = (i == count - 1);

        for (int d = 0; d < depth; d++) {
            fprintf(cfg->out, pipe_mask[d] ? "│   " : "    ");
        }

        char full_path[4096];
        int path_len = snprintf(full_path, sizeof(full_path), "%s%s%s", 
                                path, PATH_SEP, names[i]);
        
        if (path_len >= sizeof(full_path)) {
            fprintf(stderr, "Warning: Path too long, skipping: %s%s%s\n", 
                    path, PATH_SEP, names[i]);
            free(names[i]);
            continue;
        }

        struct stat st;
        int is_dir = (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode));

        fprintf(cfg->out, "%s %s%s\n", is_last ? "└──" : "├──", 
                names[i], is_dir ? "/" : "");

        if (is_dir) {
            pipe_mask[depth] = is_last ? 0 : 1;
            walk(full_path, depth + 1, cfg, pipe_mask);
        }
        free(names[i]);
    }
    free(names);
}

int find_max_id(const char *content) {
    int max_id = 1;
    const char *ptr = content;
    
    while ((ptr = strstr(ptr, "<!-- WRLDTREE START")) != NULL) {
        ptr += 19;
        while (*ptr == ' ') ptr++;
        
        if (*ptr >= '0' && *ptr <= '9') {
            int id = atoi(ptr);
            if (id > max_id) max_id = id;
        }
        
        ptr = strchr(ptr, '>');
        if (!ptr) break;
        ptr++;
    }
    
    return max_id;
}

void update_readme(const char *readme_path, const char *tree_text, int id) {
    struct stat st;
    if (stat(readme_path, &st) != 0) {
        printf("Error: %s not found. Create it or use --print.\n", readme_path);
        return;
    }

    char start_tag[64], end_tag[64];
    if (id == 1) {
        strcpy(start_tag, "<!-- WRLDTREE START -->");
        strcpy(end_tag, "<!-- WRLDTREE END -->");
    } else {
        snprintf(start_tag, sizeof(start_tag), "<!-- WRLDTREE START %d -->", id);
        snprintf(end_tag, sizeof(end_tag), "<!-- WRLDTREE END %d -->", id);
    }

    FILE *f = fopen(readme_path, "rb");
    if (!f) {
        perror("Error opening README");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    
    char *content = malloc(size + 1);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(f);
        return;
    }
    
    size_t bytes_read = fread(content, 1, size, f);
    content[bytes_read] = '\0';
    fclose(f);

    char *start_pos = strstr(content, start_tag);
    char *end_pos = strstr(content, end_tag);

    FILE *out = fopen(readme_path, "wb");
    if (!out) {
        perror("Error writing README");
        free(content);
        return;
    }

    if (start_pos && end_pos && (end_pos > start_pos)) {
        fwrite(content, 1, start_pos - content, out);
        fprintf(out, "%s\n```text\n%s```\n%s", start_tag, tree_text, end_tag);
        fprintf(out, "%s", end_pos + strlen(end_tag));
        printf("✓ README updated at %s\n", start_tag);
    } else {
        fprintf(out, "%s\n\n## Project Structure\n%s\n```text\n%s```\n%s\n", 
                content, start_tag, tree_text, end_tag);
        printf("⚠ Tags not found. Appended structure to end of README.\n");
    }

    fclose(out);
    free(content);
}

int main(int argc, char *argv[]) {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif

    int target_id = 1;
    int target_depth = 5;
    int dry_run = 0;
    char *start_node = ".";
    char *readme_path = "README.md";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print") == 0) {
            dry_run = 1;
        }
        else if (strcmp(argv[i], "--depth") == 0 && i + 1 < argc) {
            target_depth = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--id") == 0 && i + 1 < argc) {
            target_id = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--help") == 0) {
            printf("wrldtree: Keep your project documentation in sync\n\n");
            printf("Usage: wrldtree [PATH] [FLAGS]\n\n");
            printf("Flags:\n");
            printf("  --print      Display tree in terminal only (no file changes)\n");
            printf("  --depth N    Limit recursion depth (default: 5)\n");
            printf("  --id N       Target specific WRLDTREE block by ID\n");
            printf("  --help       Show this help menu\n\n");
            printf("Examples:\n");
            printf("  wrldtree                  Update default block in README.md\n");
            printf("  wrldtree --print          Preview without modifying files\n");
            printf("  wrldtree src --depth 3    Show src/ directory, 3 levels deep\n");
            printf("  wrldtree --id 2           Update block <!-- WRLDTREE START 2 -->\n\n");
            return 0;
        }
        else if (argv[i][0] != '-') {
            start_node = argv[i];
        }
    }

    printf("→ Scanning %s (depth: %d, id: %d)...\n", start_node, target_depth, target_id);

    FILE *temp = fopen("tree_temp.txt", "w");
    if (!temp) { 
        perror("Error: Could not create temporary file"); 
        return 1; 
    }

    Config cfg;
    cfg.max_depth = target_depth;
    cfg.out = temp;

    fprintf(temp, ". (%s/)\n", (strcmp(start_node, ".") == 0) ? "root" : start_node);

    int pipe_mask[100] = {0};
    walk(start_node, 0, &cfg, pipe_mask);
    fclose(temp);

    FILE *read_temp = fopen("tree_temp.txt", "rb");
    if (!read_temp) {
        perror("Error reading temporary file");
        return 1;
    }

    fseek(read_temp, 0, SEEK_END);
    long tree_size = ftell(read_temp);
    rewind(read_temp);

    char *tree_str = malloc(tree_size + 1);
    if (tree_str) {
        size_t bytes_read = fread(tree_str, 1, tree_size, read_temp);
        tree_str[bytes_read] = '\0';
        fclose(read_temp);

        if (dry_run) {
            printf("\n%s\n", tree_str);
            printf("→ Dry run complete. README.md was NOT modified.\n");
        } else {
            update_readme(readme_path, tree_str, target_id);
        }
        free(tree_str);
    } else {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(read_temp);
    }

    remove("tree_temp.txt");
    return 0;
}