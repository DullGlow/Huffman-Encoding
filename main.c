#include <stdlib.h>
#include <stdio.h>
#include "huffmantypes.h"
#include "pqueue.h"  // Priority queue implementation for Node*

int readBit(FILE *f) {
	static unsigned char byte;
	static int counter;

	if (counter == 0) {
		fread(&byte, sizeof(char), 1, f);
		if (feof(f))
			return -1;
		counter = 8;
	}
	unsigned int result = byte >> 7;  // Least significant bit
	byte = byte << 1;    // Move onto next bit
	counter--;
	return result;
}

void writeBit(int b, FILE *f) {
	static char byte;
	static int counter;

	byte = (byte << 1) | b;
	counter++;

	if (counter == 8) {
		counter = 0;    // Reset counter
		fwrite(&byte, sizeof(char), 1, f);
		byte = 0;       // Clear byte buffer
	}
}

Node* newNode(int character, int weight) {
	Node *node = malloc(sizeof(Node));
	node->character = character;
	node->weight = weight;
	node->one = NULL;
	node->zero = NULL;
	return node;
}

void createTable(FILE* f, int *table) {
	for (int i = 0; i < 258; i++)
		table[i] = 0;
	unsigned char c = getc(f);
	while (!feof(f)) {
		table[(int)c]++;
		c = getc(f);
	}
	/*
	char c = getc(f);
	while (c != EOF) {
		table[(unsigned int)c]++;
		c = getc(f);
	}*/
	table[PSEUDO_EOF]++;
}

Node *combine(Node *n1, Node *n2) {
	Node *parent = newNode(NOT_A_CHAR, n1->weight + n2->weight);
	parent->zero = n1;
	parent->one = n2;
	return parent;
}

Node *createTree(int *table) {
	// Create priority queue
    PQ_Node* pq = NULL;
	for (int i = 0; i < 258; i++) {
		if (table[i] == 0)
			continue;
		Node *node = newNode(i, table[i]);
		pq_push(&pq, node, table[i]);
	}
	// Create the tree
	Node *head;
	while (!pq_isEmpty(&pq)) {
		Node *n1 = pq_pop(&pq);
		if (pq_isEmpty(&pq)) {
			head = n1;
			break;
		}
		Node *n2 = pq_pop(&pq);

		head = combine(n1, n2);
		pq_push(&pq, head, head->weight);
	}
	return head;
}

void freeTree(Node *head) {
	if (head == NULL)
		return;
	freeTree(head->zero);
	freeTree(head->one);
	free(head);
}

void writeFileHeader(FILE *f, int *table) {
	long int size = -1;
	for (int i = 0; i < 258; i++) {
		if (table[i] != 0)
			size++;
	}
	// How many bytes we have encoded
	fprintf(f, "%ld ", size);

	for (int i = 0; i < 256; i++) {
		if (table[i] != 0)
			fprintf(f, "%c%d ", i, table[i]);
	}
}

void readFileHeader(FILE *f, int *table) {
	// Set table frequencies to 0
	for (int i = 0; i < 258; i++) {
		table[i] = 0;
	}

	// Get amount of bytes there are encoded
	char c;
	int size = 0;
	while ((c = getc(f)) != ' ') {
		size = size*10 + c - '0';
	}

	// Read header and fill the table
	for (int i = 0; i < size; i++) {
		unsigned char index = getc(f);
		int freq = 0;
		while ((c = getc(f)) != ' ') {
			freq = freq*10 + c - '0';
		}
		table[(int)index] = freq;
	}
	table[PSEUDO_EOF]++;
}

int getSequence(int character, Node *node, int *soFar, int index) {
	if (node->character != NOT_A_CHAR) {
		if (node->character == character) {
			soFar[index] = -1;   // End with -1
			return 1;
		}
		return 0;
	}
	soFar[index] = 1;
	if (getSequence(character, node->one, soFar, index+1))
		return 1;
	soFar[index] = 0;
	return getSequence(character, node->zero, soFar, index+1);
}

void encode(FILE *input, Node *head, FILE *output) {
	unsigned char c = getc(input);
	while (!feof(input)) {
		int result[258];
		for (int i = 0; i < 258; i++) {
			result[i] = 0;
		}
		if (!getSequence(c, head, result, 0))
			printf("Failed to get sequence for '%c'\n", c);
		for (int i = 0; i < 258; i++) {
			if (result[i] == -1)
				break;
			writeBit(result[i], output);
		}
		c = getc(input);
	}
	// Add EOF as well
	int result[258];
	if (!getSequence(PSEUDO_EOF, head, result, 0))
		printf("Failed to get sequence for PSEUDO_EOF\n");

	for (int i = 0; i < 258; i++) {
		if (result[i] == -1)
			break;
		writeBit(result[i], output);
	}
}

void decode(FILE *input, Node *head, FILE *output) {
	Node *node = head;
	int bit = readBit(input);

	while (bit != -1) {
		// If we reach leaf node
		if (node->character != NOT_A_CHAR && node->character != EOF) {
			putc(node->character, output);
			node = head;
		}
		if (bit == 0)
			node = node->zero;
		else 
			node = node->one;
		bit = readBit(input);
	}
}

void compress(char *inputname, char *outputname) {
	// Open the files
	FILE *input = fopen(inputname, "r");
	if (input == NULL)
		printf("Failed to open the file\n");
	FILE *output = fopen(outputname, "wb");

	// Create frequency table
	int table[258];
	createTable(input, table);

	// Reset input pointer
	fclose(input);
	input = fopen(inputname, "r");

	// Write header to output file, so we 
	// can read frequency on decoding
	writeFileHeader(output, table);

	// Create Huffman tree using the frequency table
	Node *node = createTree(table);

	// Encode the input file
	encode(input, node, output);

	// Free memory allocated for tree
	freeTree(node);

	// Close files
	fclose(input);
	fclose(output);

	printf("Sucessfully compressed %s into %s\n", inputname, outputname);
}

void decompress(char *inputname, char *outputname) {
	FILE *input = fopen(inputname, "r");
	if (input == NULL)
		printf("Failed to open the file\n");
	FILE *output = fopen(outputname, "w");

	// Read frequency table from file
	int table[258];
	readFileHeader(input, table);

	// Create Huffman tree using the frequency table
	Node *node = createTree(table);

	// Decode the input file
	decode(input, node, output);
	
	// Free memory allocated for tree
	freeTree(node);

	// Close files
	fclose(input);
	fclose(output);

	printf("Sucessfully decompressed %s into %s\n", inputname, outputname);
}

void printInstructions(){
	printf("Usage:\n");
	printf("\thuff c <file-to-compress> <output-filename>\n");
	printf("\thuff d <file-to-decompress> <output-filename>\n");
}

int main(int argc, char *argv[]) {
	if (argc <3) 
		printInstructions();
	// Compress
	else if (argv[1][0] == 'c') {
		if (argc == 4)
			compress(argv[2], argv[3]);
		else {
			char filename[] = "compressed.huff";
			compress(argv[2], filename);
		}
	}
	// Decompress
	else if (argv[1][0] == 'd') {
		if (argc == 4)
			decompress(argv[2], argv[3]);
		else {
			char filename[] = "decompressed.huff";
			decompress(argv[2], filename);
		}
	}
	// Incorrect input
	else
		printInstructions();
		
	return 0;
}
