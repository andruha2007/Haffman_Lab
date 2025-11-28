// treelist.c

#include "treelist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node_t {
    unsigned char symbol;
    unsigned long long frequency;
    struct node_t *next;
    struct node_t *left;
    struct node_t *right;
};

unsigned long long * parse_symbols(const char *input_file){
    FILE *file = fopen(input_file, "rb");
    if (!file){
        printf("ERROR");
        return NULL; 
    }

    int c;
    unsigned long long *frequencies = calloc(UTF8_LENGTH, sizeof(unsigned long long));
    if (!frequencies){
        printf("MEMORY ERROR");
        fclose(file);
        return NULL;
    }

    while ((c = fgetc(file)) != EOF) {
        frequencies[(unsigned char)c]++;
    }

    fclose(file);
    return frequencies;
}

struct node_t * list_from_freq_array(unsigned long long *frequencies) {
    struct node_t *head = NULL;
    for (int i = 0; i < UTF8_LENGTH; i++) {
        if (frequencies[i] == 0) continue;

        struct node_t *new_node = malloc(sizeof(struct node_t));
        if (!new_node) {
            return NULL;
        }
        new_node->symbol = (unsigned char)i;
        new_node->frequency = frequencies[i];
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->next = NULL;

        sorted_insert_node(&head, new_node);
    }

    return head;
}

struct node_t * make_new_node(struct node_t *left, struct node_t *right){
    struct node_t *node = malloc(sizeof(struct node_t));
    if (!node) return NULL;

    node->symbol = 0;
    node->frequency = left->frequency + right->frequency;
    node->left = left;
    node->right = right;
    node->next = NULL;
    return node;
}

static void sorted_insert_node(struct node_t **head, struct node_t *new_node) {
    if (*head == NULL || (*head)->frequency > new_node->frequency) {
        new_node->next = *head;
        *head = new_node;
    } else {
        struct node_t *curr = *head;
        while (curr->next != NULL && curr->next->frequency <= new_node->frequency) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

struct node_t * make_tree(struct node_t *root){
    if (!root) return NULL;
    if (root->next == NULL) return root;

    while (root->next != NULL){
        struct node_t *left = root;
        struct node_t *right = root->next;
        root = root->next->next;

        struct node_t *parent = make_new_node(left, right);
        if (!parent) return NULL;

        sorted_insert_node(&root, parent);
    }

    return root;
}

static void build_codes_recursive(struct node_t *node, char *buffer, int depth, unsigned char **table) {
    if (!node) return;
    if (!node->left && !node->right) {
        table[node->symbol] = malloc(depth + 1);
        strncpy((char*)table[node->symbol], buffer, depth);
        table[node->symbol][depth] = '\0';

        return;
    }

    buffer[depth] = '0';
    build_codes_recursive(node->left, buffer, depth + 1, table);

    buffer[depth] = '1';
    build_codes_recursive(node->right, buffer, depth + 1, table);
}


void make_code_table(struct node_t *root, unsigned char *codes) {
    if (!codes) return;
    if (!root) return;
    unsigned char **code_table = (unsigned char **)codes;

    for (int i = 0; i < UTF8_LENGTH; i++) {
        free(code_table[i]);
        code_table[i] = NULL;
    }

    char path[256];
    build_codes_recursive(root, path, 0, code_table);
}


void print_list(struct node_t *head) {
    if (!head) {
        printf("List is empty.\n");
        return;
    }

    printf("Symbol\tFreq\tNext\n");
    printf("------\t----\t----\n");

    struct node_t *curr = head;
    int i = 0;
    while (curr != NULL) {
        // Отображаем символ в читаемом виде (для непечатных — десятичный код)
        if (curr->symbol >= 32 && curr->symbol <= 126) {
            printf("'%c'\t%llu\t%p\n", curr->symbol, curr->frequency, (void*)curr->next);
        } else {
            printf("[%d]\t%llu\t%p\n", curr->symbol, curr->frequency, (void*)curr->next);
        }
        curr = curr->next;
        i++;
        // Защита от зацикливания (максимум 300 узлов)
        if (i > 300) {
            printf("... possible infinite loop, stopping.\n");
            break;
        }
    }
    printf("Total nodes: %d\n", i);
}

char* make_output_name(const char *input, const char *suffix) {
    size_t len = strlen(input) + strlen(suffix) + 1;
    char *output = malloc(len);
    if (!output) return NULL;
    strcpy(output, input);
    strcat(output, suffix);
    return output;
}

//////////////////____________________________КОДИРОВКА_И_ДЕКОДИРОВКА____________________________//////////////////


int utite_fuctions_to_encode(const char *input_file) {
    // 1. Генерация имени выходного файла
    char *output_file = make_output_name(input_file, ".huf");
    if (!output_file) {
        printf("Failed to generate output filename");
        return 1;
    }

    // 2. Подсчёт частот
    unsigned long long *freq = parse_symbols(input_file);
    if (!freq) {
        printf("Error: cannot read input file '%s'\n", input_file);
        free(output_file);
        return 1;
    }

    // 3. Построение списка и дерева
    struct node_t *list = list_from_freq_array(freq);
    

    //print_list(list);
    if (!list) {
        printf("Error: empty input\n");
        free(output_file);
        return 1;
    }

    struct node_t *tree = make_tree(list);
    if (!tree) {
        printf("Error: failed to build Huffman tree\n");
        free(output_file);
        return 1;
    }

    // 4. Формирование таблицы кодов
    unsigned char **code_table = calloc(UTF8_LENGTH, sizeof(unsigned char *));
    if (!code_table) {
        printf("Memory allocation failed");
        free(output_file);
        return 1;
    }
    make_code_table(tree, (unsigned char *)code_table);

    // 5. Кодирование и запись
    encode_file(input_file, output_file, freq, code_table);

    // 6. Очистка
    for (int i = 0; i < UTF8_LENGTH; i++) free(code_table[i]);
    free(code_table);
    free(output_file);
    free(freq);

    printf("Encoded: %s -> %s.huf\n", input_file, input_file);
    return 0;
}


void encode_file(const char *input_file, const char *output_file,
                 unsigned long long freq[UTF8_LENGTH], unsigned char **code_table) {
    // 1. Открывание входной файл
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        printf("ERROR: Cannot open input file '%s'\n", input_file);
        return;
    }

    // 2. Определение размера файла
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 3. Открывание выходной файл (создаётся автоматически)
    FILE *outp_file = fopen(output_file, "wb");
    if (!outp_file) {
        fprintf(stderr, "ERROR: Cannot create output file '%s'\n", output_file);
        fclose(file);
        return;
    }

    // 4. Запись заголовка: частоты + размер
    fwrite(freq, sizeof(unsigned long long), UTF8_LENGTH, outp_file);
    fwrite(&file_size, sizeof(long), 1, outp_file);

    // 5. Побитовое кодирование
    unsigned char byte = 0;
    int bit_count = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
        unsigned char symbol = (unsigned char)c;
        char *code = (char *)code_table[symbol];
        if (!code) continue; // такого не будет, зуб даю, просто на случай, если символ не встречался при анализе

        for (int i = 0; code[i] != '\0'; i++) {
            byte <<= 1;
            if (code[i] == '1') {
                byte |= 1;
            }
            bit_count++;

            if (bit_count == 8) {
                fputc(byte, outp_file);
                bit_count = 0;
            }
        }
    }

    // 6. Запись остатка (если есть)
    if (bit_count > 0) {
        byte <<= (8 - bit_count); // дополняем нулями справа
        fputc(byte, outp_file);
    }

    // 7. Закрытие файлов
    fclose(file);
    fclose(outp_file);

    printf("Encoding complete: '%s' -> '%s'\n", input_file, output_file);
}

int decode(const char *input_file){
    // 1. Генерация имени выходного файла
    char *output_file = make_output_name(input_file, ".txt");
    if (!output_file) {
        printf("Failed to generate output filename");
        return 1;
    }

    // 2. Открытие входной файл
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        printf("ERROR: Cannot open input file '%s'\n", input_file);
        return 1;
    }

    // 3. Чтение таблицы частот
    unsigned long long freq[256];
    if (fread(freq, sizeof(unsigned long long), UTF8_LENGTH, file) != UTF8_LENGTH) {
        printf("ERROR: Corrupted header (frequencies)\n");
        fclose(file);
        free(output_file);
        return 1;
    }

    // 4. Чтение размера исходного файла
    long original_size;
    if (fread(&original_size, sizeof(long), 1, file) != 1) {
        printf("ERROR: Missing file size\n");
        fclose(file);
        free(output_file);
        return 1;
    }

    // 5. Построение списка и дерева
    struct node_t *list = list_from_freq_array(freq);
    if (!list) {
        printf("ERROR: Cannot build node list\n");
        fclose(file);
        free(output_file);
        return 1;
    }

    struct node_t *tree = make_tree(list);
    if (!tree) {
        printf("ERROR: Cannot rebuild Huffman tree\n");
        fclose(file);
        free(output_file);
        return 1;
    }

    // 6. Открытие выходного файла
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("ERROR: Cannot create output file");
        fclose(file);
        free(output_file);
        return 1;
    }

    // 7. Декодирование битов
    long decoded_chars = 0;
    struct node_t *current = tree;
    int byte;
     
    while (decoded_chars < original_size && (byte = fgetc(file)) != EOF) {
        for (int i = 7; i >= 0 && decoded_chars < original_size; i--){
            int bit = (byte >> i) & 1;
            current = bit ? current->right : current->left;

            if (current && !current->left && !current->right) {
                fputc(current->symbol, out);
                decoded_chars++;
                current = tree;
            }
        }
    }

    fclose(out);
    fclose(file);
    free(output_file);

    printf("Decoded: %s -> %s.txt\n", input_file, input_file);
    return 0;

}