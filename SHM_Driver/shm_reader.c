//This is client only reads from SM and is connected to shm_server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define PORT 8080
#define FILE_PATH "output.txt" //filename
#define MAX_FILE_SIZE 10 * 1024 * 1024 // 10 MB
#define BACKUP_DIR "backup

pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;

// Structure for Huffman tree nodes
typedef struct TreeNode {
    unsigned char character;
    int frequency;
    struct TreeNode *leftChild;
    struct TreeNode *rightChild;
} TreeNode;

// Helper structure for priority queue
typedef struct {
    TreeNode **nodes;
    int size;
    int capacity;
} PriorityQueue;

// Priority queue comparator for sorting by frequency
int compareNodes(const void *a, const void *b) {
    return (*(TreeNode **)a)->frequency - (*(TreeNode **)b)->frequency;
}

// Initialize a priority queue
PriorityQueue *createPriorityQueue(int capacity) {
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    pq->nodes = (TreeNode **)malloc(capacity * sizeof(TreeNode *));
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

// Add a node to the priority queue
void enqueue(PriorityQueue *pq, TreeNode *node) {
    if (pq->size == pq->capacity) {
        pq->capacity *= 2;
        pq->nodes = (TreeNode **)realloc(pq->nodes, pq->capacity * sizeof(TreeNode *));
    }
    pq->nodes[pq->size++] = node;
    qsort(pq->nodes, pq->size, sizeof(TreeNode *), compareNodes);
}

// Remove the smallest node (by frequency) from the priority queue
TreeNode *dequeue(PriorityQueue *pq) {
    if (pq->size == 0) return NULL;
    TreeNode *node = pq->nodes[0];
    memmove(pq->nodes, pq->nodes + 1, (--pq->size) * sizeof(TreeNode *));
    return node;
}

// Create a new TreeNode
TreeNode *createTreeNode(unsigned char character, int frequency) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->character = character;
    node->frequency = frequency;
    node->leftChild = node->rightChild = NULL;
    return node;
}

// Free memory allocated for the Huffman tree
void freeTree(TreeNode *root) {
    if (!root) return;
    freeTree(root->leftChild);
    freeTree(root->rightChild);
    free(root);
}

// Calculate frequencies of characters in the content
void calculateFrequencies(const char *content, int *freqTable, int *uniqueChars) {
    for (int i = 0; i < 256; i++) freqTable[i] = 0;
    while (*content) {
        if (freqTable[(unsigned char)*content] == 0) (*uniqueChars)++;
        freqTable[(unsigned char)(*content++)]++;
    }
}

// Build a Huffman tree from frequency table
TreeNode *buildHuffmanTree(const int *freqTable) {
    PriorityQueue *pq = createPriorityQueue(256);
    for (int i = 0; i < 256; i++) {
        if (freqTable[i] > 0) enqueue(pq, createTreeNode(i, freqTable[i]));
    }

    while (pq->size > 1) {
        TreeNode *left = dequeue(pq);
        TreeNode *right = dequeue(pq);
        TreeNode *merged = createTreeNode(0, left->frequency + right->frequency);
        merged->leftChild = left;
        merged->rightChild = right;
        enqueue(pq, merged);
    }

    TreeNode *root = dequeue(pq);
    free(pq->nodes);
    free(pq);
    return root;
}

// Generate Huffman codes recursively
void generateCodes(TreeNode *root, char **codes, char *currentCode, int depth) {
    if (!root) return;
    if (!root->leftChild && !root->rightChild) {
        currentCode[depth] = '\0';
        codes[root->character] = strdup(currentCode);
        return;
    }
    currentCode[depth] = '0';
    generateCodes(root->leftChild, codes, currentCode, depth + 1);
    currentCode[depth] = '1';
    generateCodes(root->rightChild, codes, currentCode, depth + 1);
}

// Encode a string using Huffman codes
char *encodeString(const char *content, char **codes) {
    size_t bufferSize = 1024;
    char *encoded = (char *)malloc(bufferSize * sizeof(char));
    size_t length = 0;

    while (*content) {
        const char *code = codes[(unsigned char)*content++];
        size_t codeLength = strlen(code);
        if (length + codeLength >= bufferSize) {
            bufferSize *= 2;
            encoded = (char *)realloc(encoded, bufferSize);
        }
        strcpy(encoded + length, code);
        length += codeLength;
    }
    return encoded;
}

// Save the encoded content and tree to a file
void writeCompressedFile(TreeNode *root, const char *encodedText, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    // Write tree
    void writeTree(TreeNode *node) {
        if (!node) {
            fputc(0, file);
            return;
        }
        fputc(1, file);
        fputc(node->character, file);
        writeTree(node->leftChild);
        writeTree(node->rightChild);
    }
    writeTree(root);

    // Write encoded text
    uint8_t buffer = 0;
    int bitCount = 0;
    while (*encodedText) {
        buffer = (buffer << 1) | (*encodedText++ - '0');
        if (++bitCount == 8) {
            fputc(buffer, file);
            buffer = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        fputc(buffer, file);
    }
    fclose(file);
}

// Compress file using Huffman coding
void compressFile(const char* file_path) {
    char compressed_file[256];
    char backup_file[256];
    snprintf(compressed_file, sizeof(compressed_file), "%s/%s.bin", BACKUP_DIR, file_path);
    snprintf(backup_file, sizeof(backup_file), "%s/%s_backup", BACKUP_DIR, file_path);

    // Read file content
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(fileSize + 1);
    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';
    fclose(file);

    // Calculate frequencies
    int freqTable[256];
    int uniqueChars = 0;
    calculateFrequencies(content, freqTable, &uniqueChars);

    // Build Huffman tree
    TreeNode *root = buildHuffmanTree(freqTable);

    // Generate Huffman codes
    char *codes[256] = {NULL};
    char currentCode[256];
    generateCodes(root, codes, currentCode, 0);

    // Encode content
    char *encodedText = encodeString(content, codes);

    // Save compressed file
    writeCompressedFile(root, encodedText, compressed_file);

    // Free resources
    free(content);
    free(encodedText);
    for (int i = 0; i < 256; i++) {
        if (codes[i]) free(codes[i]);
    }
    freeTree(root);

    printf("File compressed to: %s\n", compressed_file);

    // Fast file copy using sendfile
    int read_fd, write_fd;
    struct stat stat_buf;

    // Open the compressed file for reading
    read_fd = open(compressed_file, O_RDONLY);
    if (read_fd < 0) {
        perror("Failed to open compressed file for reading");
        return;
    }

    // Get file stats
    if (fstat(read_fd, &stat_buf) < 0) {
        perror("Failed to get file stats");
        close(read_fd);
        return;
    }

    // Open backup file for writing
    write_fd = open(backup_file, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode);
    if (write_fd < 0) {
        perror("Failed to open backup file for writing");
        close(read_fd);
        return;
    }

    // Use sendfile for fast file copy
    off_t offset = 0;
    ssize_t sent_bytes = sendfile(write_fd, read_fd, &offset, stat_buf.st_size);
    if (sent_bytes < 0) {
        perror("sendfile failed");
    } else {
        printf("Fast copy created: %s\n", backup_file);
    }

    // Close file descriptors
    close(read_fd);
    close(write_fd);
}

void writeToFile(const char* data) {
    printf("entered the file creation process\n");
    int fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        perror("Failed to open file");
        return;
    }
    write(fd, data, strlen(data));
    close(fd);

    struct stat file_stat;
    if (stat(FILE_PATH, &file_stat) == 0 && file_stat.st_size > MAX_FILE_SIZE) {
        printf("File size exceeds 10 MB, compressing...\n");
        compressFile(FILE_PATH);
        remove(FILE_PATH); // Delete the original file after compression
    }
}

void sendDevBusIds(int sock) {
    int dev_id, bus_id;
    printf("Enter dev_id: ");
    scanf("%d", &dev_id);
    printf("Enter bus_id: ");
    scanf("%d", &bus_id);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "dev_id:%d bus_id:%d\n", dev_id, bus_id);
    send(sock, buffer, strlen(buffer), 0);
    printf("Sent to server: %s", buffer);
}

void receiveFromServer(int sock) {
    char buffer[1024];
    int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Data received from server: %s\n", buffer);
        writeToFile(buffer);
    }
}

void ensureBackupDirectory() {
    struct stat st = {0};
    if (stat(BACKUP_DIR, &st) == -1) {
        mkdir(BACKUP_DIR, 0700);
    }
}

int main() {
    ensureBackupDirectory();
    int sock;
    struct sockaddr_in server_address;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    while (1) {
        sendDevBusIds(sock);
        receiveFromServer(sock);
    }

    close(sock);
    return 0;
}
