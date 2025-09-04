#ifndef CLUSTERER_H
#define CLUSTERER_H

#include "../CustomDS/CustomDS.h"


typedef struct StringFeatureSet {
    int count;
    char **strings;
    char *extension; // unused for now
} StringFeatureSet;

typedef struct ClusterNode {
    int id;
    float distance;
    struct ClusterNode *left;
    struct ClusterNode *right;
    int size;
    int *indices;  // which files are in this cluster
} ClusterNode;




StringFeatureSet* createStringFeatureSet(char *str);
ClusterNode* hierarchical_clustering(float **distance_matrix, int file_count);
float** createDistanceMatrix(int file_count, StringFeatureSet *fileUnits[], ArrayList *priorities);
void free_cluster_node(ClusterNode *node);
void print_tree(ClusterNode *node, char **filenames, int depth);
char** getCommonStringFeatureSet(StringFeatureSet *s1, StringFeatureSet *s2, int *commonCount);
int* getCommonClusterNodeNames(ClusterNode *n1, ClusterNode *n2, int *commonCount);

#endif