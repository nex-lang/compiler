#include "gen/stack.h"

#include <stdlib.h>
#include <string.h>


unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

unsigned long gen_str_symb(ASM_StringSymbol** head, const char* str, int* counter, bool is_def, uint32_t dest_hash) {
    unsigned long hash;

    char label[32];
    
    if (is_def) {
        hash = dest_hash;
    } else if (dest_hash == 0 && str != NULL) {
        hash = hash_string(str);
    }

    snprintf(label, sizeof(label), "str_%lu_%d", hash, (*counter)++);

    size_t str_len = strlen(str);
    char *new_value = malloc(str_len + 1);
    if (new_value == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(new_value, str);
    new_value[str_len] = '\0';

    ASM_StringSymbol *new_node = malloc(sizeof(ASM_StringSymbol));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        perror("String duplication failed");
        exit(EXIT_FAILURE);
    }
    new_node->value = new_value;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ASM_StringSymbol *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    return hash;
}

unsigned long gen_char_symb(ASM_CharSymbol** head, char ch, int* counter, bool is_def, uint32_t dest_hash) {
    unsigned long hash;
    char label[32];

    if (is_def) {
        hash = dest_hash;
    } else {
        hash = hash_string(&ch);
    }

    snprintf(label, sizeof(label), "char_%lu_%d", hash, (*counter)++);

    ASM_CharSymbol* new_node = malloc(sizeof(ASM_CharSymbol));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_node->label = strdup(label);
    if (new_node->label == NULL) {
        perror("String duplication failed");
        exit(EXIT_FAILURE);
    }
    new_node->value = ch;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        ASM_CharSymbol* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    return hash;
}

void gen_string_lits(FILE* fp, ASM_StringSymbol* head) {
    ASM_StringSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    db \"%s\", 10, 0\n", current->value);
        current = current->next;
    }
}

void gen_char_lits(FILE* fp, ASM_CharSymbol* head) {
    ASM_CharSymbol* current = head;
    while (current != NULL) {
        fprintf(fp, "%s:\n", current->label);
        fprintf(fp, "    db '%c'\n", current->value);
        current = current->next;
    }
}

void stackvar_push(StackVars* cur_variables, size_t offset, uint32_t id, size_t size) {
    StackVar* var = malloc(sizeof(StackVar));

    var->offset = offset;
    var->id = id;
    var->size = size;

    cur_variables->ac_size += 1;
    cur_variables->vars = realloc(cur_variables->vars, sizeof(StackVar) * (cur_variables->ac_size));
    cur_variables->vars[cur_variables->ac_size - 1] = var;
}

void csstackvar_push(CS_StackVars* cur_csvariables, char* str, bool is_char, uint32_t iden) {
    if (is_char == true) {
        cur_csvariables->char_size += 1;
        cur_csvariables->char_vars = realloc(cur_csvariables->char_vars, sizeof(char*) * (cur_csvariables->char_size));
        cur_csvariables->char_id = realloc(cur_csvariables->char_id, sizeof(uint32_t) * (cur_csvariables->char_size));
        cur_csvariables->char_vars[cur_csvariables->char_size - 1] = strdup(str);
        cur_csvariables->char_id[cur_csvariables->char_size - 1] = iden;
    } else {
        cur_csvariables->size += 1;
        cur_csvariables->vars = realloc(cur_csvariables->vars, sizeof(char*) * (cur_csvariables->size));
        cur_csvariables->id = realloc(cur_csvariables->id, sizeof(uint32_t) * (cur_csvariables->size));
        cur_csvariables->vars[cur_csvariables->size - 1] = strdup(str);
        cur_csvariables->id[cur_csvariables->size - 1] = iden;
    }
}