#ifndef HUFFMAN_TYPES_INCLUDE
#define HUFFMAN_TYPES_INCLUDE

#define PSEUDO_EOF 256
#define NOT_A_CHAR 257

typedef struct Node {
    int character;
    struct Node *zero;
    struct Node *one;

    int weight;
} Node;


#endif