
#pragma once

#include "tree.h"

/* 递归型的前序、中序、后序树遍历 */

void recursive_traverse_preorder(Node *tree);

void recursive_traverse_inorder(Node *tree);

void recursive_traverse_postorder(Node *tree);

/* 非递归型的前序、中序、后序树遍历 */

void nonrecur_traverse_preorder(Node *tree);

void nonrecur_traverse_inorder(Node *tree);

void nonrecur_traverse_postorder(Node *tree);

/* 层次遍历 */

void level_traverse(Node *tree);