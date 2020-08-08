# Huffman-Encoding
Huffman compression/decompression implemented in C

## Compiling:

    git clone https://github.com/DullGlow/Huffman-Encoding.git
    cd Huffman-Encoding
    gcc pqueue.c main.c -o huff
    ./huff c text.txt compressed.huff
    ./huff d compressed.huff output
