#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <stdbool.h>
#include "Clusterer.h"
// #include "../Logger/Log.c"


// LogSettings logSettings;

bool __is_token_digit(const char *token) {
    for (int i = 0; i < strlen(token); i++) {
        if (!isdigit(token[i])) return false;
    }
    return true;
}










//--------------------------------------------------------
#pragma region StringFeatureSet
//--------------------------------------------------------


StringFeatureSet* createStringFeatureSet(char *str) {
    int size = strlen(str);
    int MAX_TOKEN_LENGTH = 20;

    StringFeatureSet *set = malloc(sizeof(StringFeatureSet));
    set->count = 1;
    set->strings = (char**) malloc(sizeof(char*) * 1);
    set->strings[0] = (char*) calloc(MAX_TOKEN_LENGTH, sizeof(char)); 

    int j = 0;
    for (int i = 0; i < size; i++) {
        char c = str[i];
        if (isalnum(c)) {
            if (isdigit(c)) {
                char *ptr = set->strings[set->count - 1];
                if (strlen(ptr) > 0) { //new if already somthing is written
                    set->count++;
                    set->strings = realloc(set->strings, sizeof(char*) * (set->count));
                    ptr = (char*) calloc(MAX_TOKEN_LENGTH, sizeof(char)); 
                    set->strings[set->count - 1] = ptr;
                }

                j = 0;
                do {
                    c = str[i];
                    set->strings[set->count - 1][j++] = c;
                    i++;
                } while (i < size && isdigit(str[i]));

                i--; // for loop increment
                set->strings[set->count - 1][j] = '\0';

                // next token/block
                set->count++;
                set->strings = realloc(set->strings, sizeof(char*) * (set->count));
                set->strings[set->count - 1] = (char*) calloc(MAX_TOKEN_LENGTH, sizeof(char)); 
                j = 0; 
            } else {
                set->strings[set->count - 1][j++] = tolower(c);
                set->strings[set->count - 1][j] = '\0';
            }
        } else {
            if (j > 0) {
                j = 0;
                set->count++;
                set->strings = realloc(set->strings, sizeof(char*) * (set->count));
                set->strings[set->count - 1] = (char*) calloc(MAX_TOKEN_LENGTH, sizeof(char));
            }
            while (!isalnum(str[i + 1]) && i + 1 < size) {
                i++;
            }
        }
    }

    // remove last empty token if empty
    if (set->strings[set->count - 1][0] == '\0') {
        free(set->strings[set->count - 1]);
        set->count--;
    }

    return set;
}



void freeStringFeatureSet(StringFeatureSet *set) {
    for (int i = 0; i < set->count; i++) {
        free(set->strings[i]);
    }
    free(set->strings);
    free(set);
}


char** getCommonStringFeatureSet(StringFeatureSet *s1, StringFeatureSet *s2, int *commonCount) {
    int maxCommon = (s1->count < s2->count) ? s1->count : s2->count;
    char **result = malloc(sizeof(char*) * maxCommon);
    int count = 0;
    for (int i = 0; i < s1->count; i++) {
        for (int j = 0; j < s2->count; j++) {
            if (strcmp(s1->strings[i], s2->strings[j]) == 0) {
                // Check for duplicates in result
                int duplicate = 0;
                for (int k = 0; k < count; k++) {
                    if (strcmp(result[k], s1->strings[i]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    result[count++] = strdup(s1->strings[i]);
                }
            }
        }
    }
    *commonCount = count;
    return result;
}





















//--------------------------------------------------------
#pragma region Jaccard Similarity
//--------------------------------------------------------


float __jaccard_similarity_modified(
    StringFeatureSet *a, StringFeatureSet *b, ArrayList *priorities
) {
    float intersection = 0.0f;
    float union_count = 0.0f;
    bool matched_b[b->count];
    for (int i = 0; i < b->count; i++) matched_b[i] = false;

    // Count intersection and union
    for (int i = 0; i < a->count; i++) {
        char *tokenA = a->strings[i];
        float weight = (!__is_token_digit(tokenA) && strlen(tokenA) >= 3) ? 1.0f : 0.5f;

        // Check if tokenA is in priorities charracter array
        for (int p = 0; p < priorities->count; p++) {
            if (strcmp(tokenA, priorities->data[p]) == 0) {
                weight += (p+1.0f);
                break;
            }
        }

        // Check if tokenA matches any token in B
        bool found = false;
        for (int j = 0; j < b->count; j++) {
            if (matched_b[j]) continue;

            if (strcmp(tokenA, b->strings[j]) == 0) {
                // Found match
                intersection += weight;
                matched_b[j] = true;
                found = true;
                break;
            }
        }
        union_count += weight;
    }

    // Add unmatched tokens from B to union
    for (int j = 0; j < b->count; j++) {
        if (!matched_b[j]) {
            float weight = (!__is_token_digit(b->strings[j]) && strlen(b->strings[j]) >= 3) ? 1.0f : 0.5f;
            // Check if tokenB is in priorities charracter array
            for (int p = 0; p < priorities->count; p++) {
                if (strcmp(b->strings[j], priorities->data[p]) == 0) {
                    weight += (p+1.0f);
                    break;
                }
            }
            union_count += weight;
        }
    }
    return (union_count > 0.0f) ? (intersection / union_count) : 0.0f;
}



















//--------------------------------------------------------
#pragma region Distance Matrix
//--------------------------------------------------------

float** createDistanceMatrix(int file_count, StringFeatureSet *fileUnits[], ArrayList *priorities) {
    float **distance_matrix = (float**)malloc(file_count * sizeof(float*));
    for (int i = 0; i < file_count; i++) {
        distance_matrix[i] = (float*)malloc(file_count * sizeof(float));
    }
    for (int i = 0; i < file_count; i++) {
        for (int j = 0; j < file_count; j++) {
            float sim = __jaccard_similarity_modified(fileUnits[i], fileUnits[j], priorities);
            distance_matrix[i][j] = 1.0f - sim;
        }
    }
    return distance_matrix;
}















//--------------------------------------------------------
#pragma region Hierarchical Clustering
//--------------------------------------------------------





float average_linkage_distance(float **distance_matrix, ClusterNode *a, ClusterNode *b) {
    float sum = 0.0f;
    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            sum += distance_matrix[a->indices[i]][b->indices[j]];
        }
    }
    return sum / (a->size * b->size);
}

ClusterNode* create_leaf_node(int id) {
    ClusterNode *node = malloc(sizeof(ClusterNode));
    node->id = id;
    node->distance = 0;
    node->left = node->right = NULL;
    node->size = 1;
    node->indices = malloc(sizeof(int));
    node->indices[0] = id;
    return node;
}

void free_cluster_node(ClusterNode *node) {
    if (node->left) free_cluster_node(node->left);
    if (node->right) free_cluster_node(node->right);
    if (node->indices) free(node->indices);
    free(node);
}

ClusterNode* hierarchical_clustering(float **distance_matrix, int file_count) {
    int cluster_count = file_count;
    ClusterNode **clusters = malloc(sizeof(ClusterNode*) * file_count);
    int *active = malloc(sizeof(int) * file_count);

    for (int i = 0; i < file_count; i++) {
        clusters[i] = create_leaf_node(i);
        active[i] = 1;
    }

    while (cluster_count > 1) {
        float min_dist = FLT_MAX;
        int idx_a = -1, idx_b = -1;
        for (int i = 0; i < file_count; i++) {
            if (!active[i]) continue;
            for (int j = i + 1; j < file_count; j++) {
                if (!active[j]) continue;
                float d = average_linkage_distance(distance_matrix, clusters[i], clusters[j]);
                if (d < min_dist) {
                    min_dist = d;
                    idx_a = i;
                    idx_b = j;
                }
            }
        }
        if (idx_a == -1 || idx_b == -1) break;
        {

            ClusterNode *merged = malloc(sizeof(ClusterNode));
            merged->id = -1;
            merged->distance = min_dist;
            merged->left = clusters[idx_a];
            merged->right = clusters[idx_b];
            merged->size = clusters[idx_a]->size + clusters[idx_b]->size;
            merged->indices = malloc(sizeof(int) * merged->size);
    
            memcpy(merged->indices, clusters[idx_a]->indices, sizeof(int) * clusters[idx_a]->size);
            memcpy(merged->indices + clusters[idx_a]->size, clusters[idx_b]->indices, sizeof(int) * clusters[idx_b]->size);
    
            clusters[idx_a] = merged;
            active[idx_b] = 0;
            // Reminder : Don't free clusters[idx_b] here! It's used in merged.
            cluster_count--;
        }
    }

    ClusterNode *root = NULL;
    for (int i = 0; i < file_count; i++) {
        if (active[i]) {
            root = clusters[i];
            break;
        }
    }
    free(clusters);
    free(active);
    return root;
}








void print_tree(ClusterNode *node, char **filenames, int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
    if (node->left == NULL && node->right == NULL) {
        printf("%s\n", filenames[node->indices[0]]);
    } else {
        printf("Cluster (dist = %.2f):\n", node->distance);
        if (node->left) print_tree(node->left, filenames, depth + 1);
        if (node->right) print_tree(node->right, filenames, depth + 1);
    }
}




int* getCommonClusterNodeNames(ClusterNode *n1, ClusterNode *n2, int *commonCount) {
    int maxCommon = (n1->size < n2->size) ? n1->size : n2->size;
    int *result = malloc(sizeof(int) * maxCommon);
    int count = 0;

    for (int i = 0; i < n1->size; i++) {
        for (int j = 0; j < n2->size; j++) {
            if (n1->indices[i] == n2->indices[j]) {
                result[count++] = n1->indices[i];
                break;
            }
        }
    }

    *commonCount = count;
    return result;
}
