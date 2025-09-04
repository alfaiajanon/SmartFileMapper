#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Statistics/Clusterer.h"
#include "CustomDS/CustomDS.h"
#include "Logger/Log.c"
#include "TUI-FancyTerminal/Renderer/FancyTerminal.h"
#include "TUI-FancyTerminal/Keyboard/keyboard.h"


char *path = "";

void showMenuPage();
void showControlPage();
int scan_directory(char ***strs);
char* getShortNameWithContext(const char *fullpath);
FTTreeNode* convertClusterToFTTree(ClusterNode *node, char **filenames);
LogSettings logSettings;


typedef struct {
    void **elems;
    int count;
} Data;






int main(){
    logSettings.logFilePath="../log.txt";
    showMenuPage();
}








//---------------------
#pragma region Handlers
//---------------------

int gotoControlPage(FancyTerminal *ft, FTElement *element, void* data, void* userData){
    FTElement *textBox=(FTElement*)userData;
    FTTextField *textboxRaw=(FTTextField*)(textBox->element);
    path=textboxRaw->textData; 
    if(strcmp(path,"")==0)return 0;

    exitFancyTerminal(ft);
    showControlPage();
}


int gotoMenuPage(FancyTerminal *ft, FTElement *element, void* data, void* userData){
    exitFancyTerminal(ft);
    showMenuPage();
}



int preview(FancyTerminal *ft, FTElement *element, void* data, void* userData){

    Data *ar = (Data*)userData;
    FTElement *preview = (FTElement*)ar->elems[0];
    FTElement *priorityTextBox = (FTElement*)ar->elems[1];
    FTTreeNode **root = (FTTreeNode**)ar->elems[2];

    // char *strs[] = {"anon/test", "anon/assignment-test2", "anon/test3", "file22", "samia/test","samia/assignment-test2"};
    // int file_count = sizeof(strs) / sizeof(char *);

    char **strs = NULL;
    int file_count = scan_directory(&strs);
    if (file_count == 0) {
        return 0;
    }

    StringFeatureSet *sets[file_count];
    for (int i = 0; i < file_count; i++) {
        sets[i] = createStringFeatureSet(strs[i]);
    }

    FTTextField *priorityTextField = (FTTextField*)priorityTextBox->element;
    ArrayList *priorities = parseTextToArrayList(priorityTextField->textData, ",");
    reverseArrayList(priorities);


    float **distance_matrix = createDistanceMatrix(file_count, sets, priorities);

    ClusterNode *tree = hierarchical_clustering(distance_matrix, file_count);
    *root = convertClusterToFTTree(tree, strs);

    //update userdata
    // ar->elems[2] = root;
    ((FTTreeView*)(preview->element))->root=*root;
}




void recursive_rename(FTTreeNode *node, const char *basePath) {
    if (!node) {
        writeLog(&logSettings, "Node is NULL, skipping rename.");
        return;
    }

    // If this is a leaf node (file), move it here
    if (node->metadata) {
        char newFilePath[1024];
        //remove leading slashes from label
        char *label = node->label;
        while (*label != '/' ) label++;  
        
        snprintf(newFilePath, sizeof(newFilePath), "%s/%s", basePath, label);
        // Ensure basePath exists
        mkdir(basePath, 0755);
        rename(node->metadata, newFilePath);
        char msg[256];
        snprintf(msg, sizeof(msg), "Renamed file: %s to %s\n", (char*)node->metadata, newFilePath);
        writeLog(&logSettings, msg);
        return;
    }

    // Otherwise, create directory for this node and process children
    char newPath[1024];
    snprintf(newPath, sizeof(newPath), "%s/%s", basePath, node->label);
    mkdir(newPath, 0755);

    for (int i = 0; i < node->childCount; i++) {
        recursive_rename(node->childs[i], newPath);
    }
}

int performRestructure(FancyTerminal *ft, FTElement *element, void* data, void* userData){
    FTTreeNode **rootNode = (FTTreeNode**)userData;
    if(rootNode==NULL) ft->enabled=0;
    recursive_rename(*rootNode, path);
    return 0;
}














//-------------------
#pragma region Pages
//-------------------

void showMenuPage(){
    FancyTerminal *menuPage = initFancyTerminal();
    setFTLayoutPattern(
        menuPage,
        7,7,
        "0 0 0 0 0 0 0,"
        "0 1 1 2 2 2 0,"
        "0 1 1 2 2 2 0,"
        "0 1 1 2 2 2 0,"
        "0 3 3 3 3 3 0,"
        "0 4 4 4 4 4 0,"
        "0 0 0 0 0 0 0,"
    );

    FTElement* logo = createFTLogo(
        11,20,
        " .----------------. ,"
        "| .--------------. |,"
        "| |  _________   | |,"
        "| | |_   ___  |  | |,"
        "| |   | |_  \\_|  | |,"
        "| |   |  _|      | |,"
        "| |  _| |_       | |,"
        "| | |_____|      | |,"
        "| |              | |,"
        "| '--------------' |,"
        " '----------------' ,"
    );
    addToFancyTerminal(menuPage,logo);
    setElementDecoration(logo,FT_BORDER_COLOR,-1);
    
    FTElement* title = createFTBanner(
        "Welcome to\n Smart File Mapper (SFM)\n"
        "version 1.0.0\n \n \n"
        "Choose folder to create new hierarchy \n"
    );
    addToFancyTerminal(menuPage,title);
    setElementDecoration(title,FT_BORDER_COLOR,-1);
    
    FTElement* textBox=createFTTextField("Enter folder location: ",path?path:"");
    addToFancyTerminal(menuPage,textBox);

    FTElement* button = createFTButton("Start Process");
    addToFancyTerminal(menuPage,button);

    char *path = ((FTTextField*)textBox->element)->textHint;
    printf("%s",path);
    ft_connect(button, FT_EVENT_CLICK, gotoControlPage, (void*)textBox);

    menuPage->selector=2;  // focus on text box
    enterFancyTerminal(menuPage);

    return;
}




void showControlPage(){
    FancyTerminal *controlPage=initFancyTerminal();
    setFTLayoutPattern(
        controlPage,
        7,4,
        "1 1 2 2,"
        "3 3 2 2,"
        "3 3 2 2,"
        "3 3 2 2,"
        "3 3 2 2,"
        "4 4 2 2,"
        "5 6 2 2,"
    );
    

    char text[100] = "Control Panel\n";
    if(path!=NULL){
        strcat(text,path);
        strcat(text,"\n");
    }
    printf("%s",text);
    FTElement* title2 = createFTBanner(                                 //1
        text
    );
    addToFancyTerminal(controlPage,title2);
    
    FTElement* previewBox = createFTTreeView(                           //2
        NULL
    );
    addToFancyTerminal(controlPage,previewBox); 

    
    FTElement* priorityTextBox=createFTTextField("Priority wise keywords (comma separated): ","");    //3
    addToFancyTerminal(controlPage,priorityTextBox);
    
    FTElement* previewBtn = createFTButton("Check Output");             //4
    addToFancyTerminal(controlPage,previewBtn);

    FTElement* backBtn = createFTButton("Back");                        //5
    addToFancyTerminal(controlPage,backBtn);
    
    FTElement* performBtn = createFTButton("Continue");                 //6
    addToFancyTerminal(controlPage,performBtn);


    FTTreeNode *rootNode = NULL;
    FTTreeNode **p=&rootNode;

    Data arr;
    arr.count = 3;
    arr.elems = malloc(sizeof(void*) * arr.count);
    arr.elems[0] = previewBox;
    arr.elems[1] = priorityTextBox;
    arr.elems[2] = p;

    ft_connect(previewBtn, FT_EVENT_CLICK, preview, (void*)&arr);
    ft_connect(backBtn, FT_EVENT_CLICK, gotoMenuPage, NULL);
    ft_connect(performBtn, FT_EVENT_CLICK, performRestructure, (void*)p);

    controlPage->selector=2;  // focus on text box
    enterFancyTerminal(controlPage);
    return;
}



























//-------------------------------
#pragma region Main Functionality
//-------------------------------

char **getCommonFTTreeChildNames(FTTreeNode *parent, int *count) {
    if (!parent || parent->childCount == 0) {
        *count = 0;
        return NULL;
    }
    StringFeatureSet *common = createStringFeatureSet(parent->childs[0]->label);
    for (int i = 1; i < parent->childCount; i++) {
        StringFeatureSet *next = createStringFeatureSet(parent->childs[i]->label);
        int tmpCount = 0;
        char **tmp = getCommonStringFeatureSet(common, next, &tmpCount);
        // Update common set to tmp
        common->count = tmpCount;
        for (int j = 0; j < tmpCount; j++) {
            free(common->strings[j]);
            common->strings[j] = strdup(tmp[j]);
            free(tmp[j]);
        }
        free(tmp);
        // If no common, break early
        if (tmpCount == 0) break;
    }
    if (common->count > 0) {
        char **result = malloc(sizeof(char*) * common->count);
        for (int i = 0; i < common->count; i++)
            result[i] = strdup(common->strings[i]);
        *count = common->count;
        return result;
    }
    *count = 0;
    return NULL;
}

FTTreeNode* convertClusterToFTTree(ClusterNode *node, char **filenames) {
    if (node == NULL) return NULL;

    FTTreeNode *ftNode = malloc(sizeof(FTTreeNode));
    ftNode->childCount = 0;
    ftNode->childs = NULL;
    ftNode->metadata = NULL; 

    // Leaf node (has file indices)
    if (node->left == NULL && node->right == NULL && node->size > 0) {
        // Only leaf node (which can be only one) counts as a file
        // if (node->size == 1) {
        //     ftNode->label = getShortNameWithContext(filenames[node->indices[0]]);
        // } else {
            // // create intermediate cluster label
            // char labelBuf[64];
            // snprintf(labelBuf, sizeof(labelBuf), "Cluster (dist = %.2f, id : %d)", node->distance, node->id);
            // ftNode->label = strdup(labelBuf);
            // ftNode->childCount = node->size;
            // ftNode->childs = malloc(sizeof(FTTreeNode*) * node->size);
            // for (int i = 0; i < node->size; i++) {
            //     FTTreeNode *child = malloc(sizeof(FTTreeNode));
            //     child->label = getShortNameWithContext(filenames[node->indices[i]]);
            //     child->label = strdup(filenames[node->indices[i]]);
            //     child->childCount = 0;
            //     child->childs = NULL;
            //     ftNode->childs[i] = child;
            // }
        // }
        // return ftNode;

        ftNode->label = getShortNameWithContext(filenames[node->indices[0]]);
        ftNode->metadata = filenames[node->indices[0]]; // Store full path as metadata
        return ftNode;
    }
    

    int maxChildren = 2;
    ftNode->childs = malloc(sizeof(FTTreeNode*) * maxChildren);
    
    // Internal node: left and right children
    if (node->left)
        ftNode->childs[ftNode->childCount++] = convertClusterToFTTree(node->left, filenames);
    if (node->right)
        ftNode->childs[ftNode->childCount++] = convertClusterToFTTree(node->right, filenames);

        

    //---- Common Folder Name Generation ----
    char labelBuf[64];
    int commonCount = 0;
    char **common = getCommonFTTreeChildNames(
        ftNode, &commonCount
    );
    char folderName[64];
    if(commonCount==0) snprintf(folderName, sizeof(folderName), "folder");
    else if(commonCount==1) snprintf(folderName, sizeof(folderName), "%s", common[0]);
    else{
        for(int i = 0; i < commonCount; i++) {
            if (i == 0) {
                snprintf(folderName, sizeof(folderName), "%s", common[i]);
            } else {
                strncat(folderName, "_", sizeof(folderName) - strlen(folderName) - 1);
                strncat(folderName, common[i], sizeof(folderName) - strlen(folderName) - 1);
            }
        }
    }
    snprintf(labelBuf, sizeof(labelBuf), folderName, node->distance, node->id);
    ftNode->label = strdup(labelBuf);
    //-------

    return ftNode;
}






char* getShortNameWithContext(const char *fullpath) {

    int lastSlashIndex = -1;
    int prevOfLastSlashIndex = -1;
    for(int i=0; i<strlen(fullpath); i++){
        if(fullpath[i]=='/'){
            prevOfLastSlashIndex=lastSlashIndex;
            lastSlashIndex=i;
        }
    }

    if(lastSlashIndex==-1) return strdup(fullpath);
    else return strndup(fullpath+prevOfLastSlashIndex+1, strlen(fullpath)-prevOfLastSlashIndex-1);
}








// Recursive helper
static void scan_directory_recursive(const char *dir_path, char ***list, int *count, int *capacity) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Build full path
        char *fullpath;
        asprintf(&fullpath, "%s/%s", dir_path, entry->d_name);  // GNU extension, works on Linux

        struct stat st;
        if (stat(fullpath, &st) == -1) {
            free(fullpath);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            scan_directory_recursive(fullpath, list, count, capacity);
        } else if (S_ISREG(st.st_mode)) {
            // Grow list if needed
            if (*count >= *capacity) {
                *capacity *= 2;
                *list = realloc(*list, (*capacity) * sizeof(char *));
                if (!*list) {
                    perror("realloc failed");
                    exit(1);
                }
            }
            (*list)[*count] = fullpath;  // already strdup-ed via asprintf
            (*count)++;
        } else {
            free(fullpath);  // skip non-files
        }
    }

    closedir(dir);
}




// main interface for scanning directory
int scan_directory(char ***strs) {
    *strs = malloc(16 * sizeof(char *));  // initial capacity
    if (!*strs) {
        perror("malloc failed");
        return 0;
    }

    int count = 0;
    int capacity = 16;

    if (!path || strlen(path) == 0) {
        fprintf(stderr, "Invalid global path\n");
        return 0;
    }

    scan_directory_recursive(path, strs, &count, &capacity);
    return count;
}



