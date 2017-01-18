/*
  Author: Chao Gao
  file: tree.h
  This program is for constructing the abstract syntax tree.
*/

#ifndef FILE_TREE_H
#define FILE_TREE_H

#include "node.h"
#include "def.h"

TreeNode* create_node(TreeNodeType, char*, int, ...);
TreeNode* merge_node(TreeNode*, TreeNode*);
void double_space(TreeNode*);

void print_tree(TreeNode*, int);

TreeNode* treeroot;

void double_space(TreeNode* p) { //check if need to extend
    if (p->size == p->capacity) {
        assert(p->capacity > 0); //these tree nodes need to double space cannot be the leaf
        TreeNode** children = (TreeNode**)malloc(sizeof(TreeNode*) * (p->capacity << 1));
        int i;
        for (i = 0; i < p->size; i++) {
            children[i] = p->children[i];
        }
        free(p->children);
        p->children = children;
        p->capacity <<= 1;
    }
}

TreeNode* create_node(int lineno,TreeNodeType type, char* data, int cnt, ...) {
    TreeNode* ptr = (TreeNode*)malloc(sizeof(TreeNode));
    ptr->line_num = lineno;
    ptr->type = type;
    ptr->data = strdup(data);
    ptr->size = ptr->capacity = cnt;
    ptr->children = (TreeNode**)malloc(sizeof(TreeNode*) * cnt);
    va_list ap;
    va_start(ap, cnt);
    int i;
    for (i = 0; i < cnt; i++) {
        ptr->children[i] = va_arg(ap, TreeNode*);
    }
    va_end(ap);
    return ptr;
}

TreeNode* merge_node(TreeNode* a, TreeNode* b) {
    double_space(a);
    a->children[a->size++] = b;
    return a;
}

#define MAX_LENGTH 10000000

char buffer[MAX_LENGTH];

void print_tree(TreeNode* ptr, int depth) {
    int i;
    int n = (depth - 1) * 2;
    
    for (i = 0; i < n; i++) {
        putchar(buffer[i]);
    }
    if (depth > 0) {
        putchar('|');
        putchar('_');
    }
    n = depth * 2;
    buffer[n] = '|';
    buffer[n + 1] = ' ';
    if (ptr->size > 0) {
        printf("%s\n", ptr->data);
    } else {
        printf("\033[31;1m%s\033[0m\n", ptr->data);
    }
    for (i = 0; i < ptr->size; i++) {
        if (i == ptr->size - 1) {
            buffer[n] = ' ';
        }
        print_tree(ptr->children[i], depth + 1);
    }
}

#endif
