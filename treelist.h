// treelist.h

#ifndef TREELIST_H
#define TREELIST_H

#define MAX_FILENAME 256
#define UTF8_LENGTH 256

struct node_t;

// парсит символы из файла и собирает их в список
unsigned long long * parse_symbols(const char *input_file);

// создает упорядоченный по убыванию список из массива
struct node_t * list_from_freq_array(unsigned long long *frequencies);

// создает новый элемент дерева из двух элементов дерева
struct node_t * make_new_node(struct node_t *left, struct node_t *right);

// вставляет узлы в список по их возрастанию
static void sorted_insert_node(struct node_t **head, struct node_t *new_node);

// создает дерево из упорядоченного списка
struct node_t * make_tree(struct node_t *root);

// делает рекурсивный обход по дереву
static void build_codes_recursive(struct node_t *node, char *buffer, int depth, unsigned char **table);

// заполняет таблицу кодов символов
void make_code_table(struct node_t *root, unsigned char *codes);

// возвращает код символа по дереву
unsigned char get_char_code(unsigned char symbol, struct node_t *root);

// тестовый вывод списка
void print_list(struct node_t *head);

// создают имя для выходного файла
char* make_output_name(const char *input, const char *suffix);

// объединяет функции treelist в один поток
int utite_fuctions_to_encode(const char *input_file);

// кодирует данные
void encode_file(const char *input_file, const char *output_file, unsigned long long freq[UTF8_LENGTH], unsigned char **code_table);

// декодировка данных
int decode(const char *input_file);

#endif