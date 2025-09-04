#ifndef FANCY_TERMINAL_H
#define FANCY_TERMINAL_H



enum FTElemType{
    FT_EMPTY        = 0,        //done
    FT_LOGO         = 1,        //done
    FT_BANNER       = 2,        //done
    FT_BUTTON       = 3,        //done
    FT_TEXTFIELD    = 4,        //done
    FT_CHECKBOX     = 5,        //done
    FT_RADIOBOX     = 6,        //done
    FT_MENU_BAR     = 7,
    FT_LISTBOX      = 8,
    FT_TREEVIEW     = 9,        //done
    FT_TABLE        = 10,
    FT_SLIDER       = 11,
    FT_PROGRESSBAR  = 12,
};

enum FTEvents{
    FT_EVENT_SELECTED,
    FT_EVENT_UNSELECTED,
    FT_EVENT_CLICK,
    FT_EVENT_STATE_CHANGED,
    FT_EVENT_INPUT_STREAM,
};

enum FTConstants{
    TOTAL_EVENTS=5,
    TOTAL_ELEM_TYPES=13,
    TOTAL_DECORATIONS=4
};

enum FTDecorations{
    FT_COLOR=0,
    FT_BORDER_COLOR=1,
    FT_SELECTED_BORDER_COLOR=2,
    FT_SELECTOR_BORDER_COLOR=3
};


typedef struct _FTElement FTElement;

typedef struct {
    int width;
    int height;
    int posX;
    int posY;
} FTElemData;

typedef struct{
    int needsRedraw;
} FTAuxStatus;




typedef struct{
    char *pattern;
    int height;
    int width;
}FTLogo;

typedef struct{
    char *text;
}FTBanner;

typedef struct{
    char *text;
    int (*callable)(void*, void*);
    void *callableData; 
}FTButton;

typedef struct{
    char *textHint;
    char *textData;
}FTTextField;

typedef struct{
    int elemCount;
    int localPointer;
    int *checked;
    char **texts;
}FTCheckBox;

typedef struct{
    int elemCount;
    int localPointer;
    int selected;
    char **texts;
}FTRadioBox;

typedef struct FTTreeNode {
    char *label;
    int childCount;
    char *metadata;
    struct FTTreeNode **childs;
} FTTreeNode;

typedef struct FTTreeView{
    FTTreeNode *root;
    int scrollPos;
} FTTreeView;






typedef struct{
    int *patternDims;
    int **pattern;
    int elemCount;
    int loadedElemCount;
    FTElement **elemPointers;
    FTElemData *elemDatas; 
} FTLayout;

typedef struct{
    FTLayout *layout;
    int selector;
    int selected;
    int height;
    int width;
    int enabled;
    FTAuxStatus auxStatus;
} FancyTerminal;

typedef struct _FTElement{
    int type;
    void *element;
    void (*renderer)(void*, int*, int, int, int, int, int, int);                        //elem, decorations, width, height, x, y, hovered, selected
    int (*eventsCallable[TOTAL_EVENTS])(FancyTerminal*, FTElement*, void*, void*);      //parent, self, data, userData
    void* eventsCallableData[TOTAL_EVENTS];
    int* decorations;
}FTElement;



FancyTerminal* initFancyTerminal();
FTElement* createFTLogo(int height, int width, char *pattern);
FTElement* createFTBanner(char *text);
FTElement* createFTButton(char *text);
FTElement* createFTTextField(char *textHint, char *textData);
FTElement* createFTCheckBox(int count, char **text);
FTElement* createFTRadioBox(int count, char **text);
FTElement* createFTTreeView(FTTreeNode *root);

void enterFancyTerminal(FancyTerminal *ft);
void exitFancyTerminal(FancyTerminal *ft);
void addToFancyTerminal(FancyTerminal *ft, FTElement *element);
void setElementDecoration(FTElement *element, int decoration, int value);
void setFTLayoutPattern(FancyTerminal *ft, int rowCount, int colCount, char *pattern);
void ft_connect(FTElement* ft, int event, int (*func)(FancyTerminal*, FTElement*,void*,void*), void* data);


#endif