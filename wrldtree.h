#ifndef WRLDTREE_H
#define WRLDTREE_H

#include <stdio.h>

typedef struct {
    int max_depth;
    int current_max;
    FILE *out;
    int stats_only;
    const char *root_path;
} WrldConfig;

void tree_walk(const char *path, int depth, WrldConfig *cfg);
void tree_replace(const char *readme_path, const char *new_tree, int id);

#endif