#include "tree.h"
#include "cpu.h"

s32 cpuTreeTake(recomp_node_t**, u32*, size_t);
s32 treeInsertNode(recomp_node_t** root, u32 n64_start_addr, u32 n64_end_addr, recomp_node_t** new_node);
s32 treeAdjustRoot(cpu_class_t* cpu, u32 n64_start_addr, u32 n64_end_addr);
s32 treeBalance(recomp_tree_t* tree);
s32 treeSearchNode(recomp_node_t* node, s32 n64_addr, recomp_node_t** out_node);
s32 treePrintNode(cpu_class_t* cpu, recomp_node_t* node, s32 arg2, s32* start, s32* end);

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeCallerCheck.s")

s32 treeInit(cpu_class_t* cpu, u32 code_boundary) {
    recomp_tree_t* tree = cpu->recomp_tree;
    if (tree == NULL) {
        return 0;
    }
    tree->node_cnt = 0;
    tree->total_size = 0;
    tree->code_boundary = code_boundary;
    tree->n64_start = 0x00000000;
    tree->n64_end = 0x80000000;
    tree->code_root = NULL;
    tree->ovl_root = NULL;
    tree->unk_0x70 = 0;
    tree->unk_0x74 = 0;
    tree->unk_0x78 = 0;
    tree->unk_0x7C = NULL;
    tree->unk_0x80 = 0;
    return 1;
}

s32 treeInitNode(recomp_node_t** out_node, recomp_node_t* parent, u32 n64_start_addr, u32 n64_end_addr) {
    recomp_node_t* new_node;
    u32 unk38;

    if (!cpuTreeTake(&new_node, &unk38, sizeof(recomp_node_t))) {
        return 0;
    }

    new_node->n64_start_addr = n64_start_addr;
    new_node->n64_end_addr = n64_end_addr;
    new_node->ext_calls = NULL;
    new_node->ext_call_cnt = 0;
    new_node->state = 0x21; // todo: create enum
    new_node->unk_0x00 = 0;
    new_node->recompiled_func = NULL;
    new_node->branch_cnt = 0;
    new_node->branches = NULL;
    new_node->checksum = 0;
    new_node->unk_0x28 = 1;
    new_node->size = 0;
    new_node->alloc_type = -1; // todo: create enum
    new_node->unk_0x34 = -1;
    new_node->unk_0x38 = unk38;
    new_node->parent = parent;
    new_node->left = NULL;
    new_node->right = NULL;
    *out_node = new_node;
    return 1;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeKill.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeKillNodes.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeDeleteNode.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/func_8003F330.s")

s32 treeInsert(cpu_class_t* cpu, s32 n64_start_addr, s32 n64_end_addr) {
    recomp_tree_t* tree = cpu->recomp_tree;
    recomp_node_t* new_node;
    s32 node_added;

    if (tree == NULL) {
        return 0;
    }

    if (n64_start_addr < tree->code_boundary && n64_end_addr > tree->code_boundary) {
        treeAdjustRoot(cpu, n64_start_addr, n64_end_addr);
    }

    tree->node_cnt++;
    tree->total_size += sizeof(recomp_node_t);

    if (n64_start_addr != 0x80000180) {
        if (n64_start_addr < tree->n64_start) {
            tree->n64_start = n64_start_addr;
        }

        if (n64_end_addr > tree->n64_end) {
            tree->n64_end = n64_end_addr;
        }
    }

    if (n64_start_addr < tree->code_boundary) {
        node_added = treeInsertNode(&tree->code_root, n64_start_addr, n64_end_addr, &new_node);
    } else if (n64_start_addr > tree->code_boundary) {
        node_added = treeInsertNode(&tree->ovl_root, n64_start_addr, n64_end_addr, &new_node);
    } else {
        return 0;
    }
    if (node_added) {
        return treeBalance(tree);
    } else {
        return 0;
    }
}

#ifdef NON_MATCHING
s32 treeInsertNode(recomp_node_t** root, u32 n64_start_addr, u32 n64_end_addr, recomp_node_t** new_node) {
    recomp_node_t* node = *root;

    if (*root == NULL) {
        if (treeInitNode(root, NULL, n64_start_addr, n64_end_addr)) {
            *new_node = *root;
            return 1;
        } else {
            return 0;
        }
    }

    do {
        node = *root;
        if (n64_start_addr < node->n64_start_addr) {
            root = &node->left;
        } else if (n64_start_addr > node->n64_start_addr) {
            root = &node->right;
        } else {
            return 0;
        }
    } while (*root != NULL);

    if (treeInitNode(root, node, n64_start_addr, n64_end_addr)) {
        *new_node = *root;
        return 1;
    } else {
        return 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeInsertNode.s")
#endif

#ifdef NON_MATCHING
s32 treeBalance(recomp_tree_t* tree) {
    s32 i;
    s32 j;

    for (i = 0; i < 2; i++) {
        recomp_node_t* start = (i == 0) ? tree->code_root : tree->ovl_root;
        recomp_node_t* node;
        s32 depth;

        if (start == NULL) {
            continue;
        }

        for (depth = 0, node = start; node->right != NULL; node = node->right) {
            depth++;
        }

        if (depth >= 12) {
            node = start;
            for (depth = depth / 2; depth != 0; depth--) {
                node = node->right;
            }
            node->parent->right = NULL;
            start->right = node;
            node->parent = start;

            while (node->left != NULL) {
                node = node->left;
            }
            node->left = start->right;
            start->right->parent = node;
        }

        for (depth = 0, node = start->left; node != NULL; node = node->left) {
            depth++;
        }

        if (depth >= 12) {
            node = start;
            for (depth = depth / 2; depth != 0; depth--) {
                node = node->left;
            }

            node->parent->left = NULL;
            start->left = node;
            node->parent = start;

            while (node->right != NULL) {
                node = node->right;
            }
            node->right = start->left;
            start->left->parent = node;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeBalance.s")
#endif

#ifdef NON_MATCHING
s32 treeAdjustRoot(cpu_class_t* cpu, u32 n64_start_addr, u32 n64_end_addr) {
    recomp_tree_t* tree = cpu->recomp_tree;
    s32 orig_boundary = tree->code_boundary;
    s32 n64_addr = orig_boundary + 2;
    s32 addr2 = 0;
    u32 node_cnt = tree->node_cnt;
    u32 size = tree->total_size;
    recomp_node_t* node;
    recomp_node_t* node2;

    while (n64_end_addr >= n64_addr) {
        node = NULL;
        treeSearchNode(tree->ovl_root, n64_addr, &node);
        if (node != NULL) {
            if (addr2 == 0) {
                addr2 = n64_addr;
            }

            tree->code_boundary = n64_end_addr + 2;
            if (!treeInsert(cpu, node->n64_start_addr, node->n64_end_addr)) {
                return 0;
            }

            if (!treeSearchNode(tree->code_root, n64_addr, &node2)) {
                return 0;
            }

            node2->unk_0x28 = node->unk_0x28;
            node2->size = node->size;
            if (node->recompiled_func != NULL) {
                node2->recompiled_func = node->recompiled_func;
                node->recompiled_func = NULL;
            }

            node2->branch_cnt = node->branch_cnt;
            if (node->branches != NULL) {
                node2->branches = node->branches;
                node->branches = NULL;
            }

            node2->checksum = node->checksum;
            node2->state = node->state;
            node2->unk_0x1C = node->unk_0x1C;
            if (node->unk_0x18 != NULL) {
                node2->unk_0x18 = node->unk_0x18;
                node->unk_0x18 = NULL;
            }

            n64_addr = node->n64_end_addr;
            tree->code_boundary = orig_boundary;
            treeKillRange(cpu, tree->ovl_root, node->n64_start_addr, node->n64_end_addr - 4);
        }

        n64_addr += 4;
    }

    tree->code_boundary = n64_end_addr + 2;
    tree->node_cnt = node_cnt;
    tree->total_size = size;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeAdjustRoot.s")
#endif

s32 treeSearchNode(recomp_node_t* node, s32 n64_addr, recomp_node_t** out_node) {
    if (node == NULL) {
        return 0;
    }

    do {
        if (n64_addr >= node->n64_start_addr && n64_addr < node->n64_end_addr) {
            *out_node = node;
            return 1;
        }

        if (n64_addr < node->n64_start_addr) {
            node = node->left;
        } else if (n64_addr > node->n64_start_addr) {
            node = node->right;
        } else {
            node = NULL;
        }
    } while (node != NULL);

    return 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeKillRange.s")

s32 treeTimerCheck(cpu_class_t* cpu) {
    s32 start;
    s32 end;
    recomp_tree_t* tree;

    if (cpu->call_cnt > 0x7FFFF000) {
        tree = cpu->recomp_tree;
        if (tree->unk_0x70 != 0) {
            return 0;
        }

        end = 0x7FFFF000;
        start = 0;
        if (tree->code_root != NULL) {
            treePrintNode(cpu, tree->code_root, 0x100, &start, &end);
        }

        if (tree->ovl_root != NULL) {
            treePrintNode(cpu, tree->ovl_root, 0x100, &start, &end);
        }

        start = end - 3;
        if (tree->code_root != NULL) {
            treePrintNode(cpu, tree->code_root, 0x1000, &start, &end);
        }

        if (tree->ovl_root != NULL) {
            treePrintNode(cpu, tree->ovl_root, 0x1000, &start, &end);
        }

        cpu->call_cnt -= start;
        return 1;
    }

    return 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/func_80040258.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeCleanup.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeCleanNodes.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treeForceCleanNodes.s")

#pragma GLOBAL_ASM("asm/non_matchings/virtual_console/tree/treePrintNode.s")
