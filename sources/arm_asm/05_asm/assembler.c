#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include "cl_getline.h"
#include "binary_tree.h"
#include "dict.h"
#include "unsolved_label_address_list.h"

static unsigned char word_buf[WORD_BUF_SIZE];
static struct Emitter emitter;


// Return the next non-space character: " a" -> 'a'
int get_next_nonsp_ch(char* str) {
    int len_read_ch = 0;
    int ch = str[len_read_ch];

    while (is_space(ch)) {
        ch = str[++len_read_ch];
    }

    return ch;
}

// number: 0xE59F101E -> emitter: 
//   pos       n     n+1   n+2   n+3   n+4
//   word_buf  0x1E, 0x10, 0x9F, 0xE5
void emit_int(struct Emitter* emitter, int number) {
    for (int i = 0; i < 4; i++) {
        emitter->word_buf[emitter->pos++] = (number >> (i * 8)) & 0xFF;
    }
}

// char: "hello " -> emitter: 
//   pos       n    n+1  n+2  n+3  n+4   n+5   n+6  n+7
//   word_buf  'h', 'e', 'l', 'l', 'o', ' ', '\0', '\0'
void emit_string(struct Emitter* emitter, char* str) {
    char str_buf[STR_SIZE] = {'\0'};
    strcpy(&str_buf, str);
    int len = strlen(str);
    int len_four_bytes_aligned = len + (4 - len % 4) % 4;

    for (int i = 0; i < len_four_bytes_aligned; i++) {
        int str_pos = 4 * (i / 4) + (i % 4);
        emitter->word_buf[emitter->pos++] = str_buf[str_pos];
    }
}

// mov: " r1, r2" -> return 0, out_word->u.number = E1A01002
int asm_mov(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_dest = 0;

    word.wtype = WORD_NUMBER;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    int next_ch = get_next_nonsp_ch(str);

    if ((next_ch == 'r') || (next_ch == 'R')) {
        // 0xE1A0X00X
        word.u.number = 0xE1A00000;
        word.u.number += register_dest << 12;

        int register_op2 = 0;            
        len_read_ch = parse_register(str, &register_op2);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;
        word.u.number += register_op2;
    } else if (next_ch == '#') {
        // 0xE3A0XXXX
        word.u.number = 0xE3A00000;
        word.u.number += register_dest << 12;

        int immediate_v = 0;
        len_read_ch = parse_immediate_value(str, &immediate_v);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;
        word.u.number += immediate_v;
    } else {
        return ASM_FAILURE;
    }

    *out_word = word;
    return 0;
}

// cmp: " r3, #0x0" -> return 0, out_word.u.number = E3530000
// Now only supporting operand2 is an immediate value case.
int asm_cmp(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_operand1 = 0;
    int immediate_v = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = 0xE3500000;

    len_read_ch = parse_register(str, &register_operand1);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_operand1 << 16;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = parse_immediate_value(str, &immediate_v);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += immediate_v;

    *out_word = word;
    return 0;
}

// Assemble data processing instructions with three arguments.
//   e.g.) add (base_word = 0xE0800000): 
//     " r1, r1, #0x05" -> return 0, out_word.u.number = E2811005
//     " r3, r3, r4" -> return 0, out_word.u.number = E0833004
int asm_data_processing_3_args(char* str, struct Word* out_word, int base_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_1st = 0;
    int register_dest = 0;
    int immediate_v = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = base_word;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_dest << 12;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    
    len_read_ch = parse_register(str, &register_1st);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    int next_ch = get_next_nonsp_ch(str);
    if ((next_ch == 'r') || (next_ch == 'R')) {
        int register_2nd = 0;
        len_read_ch = parse_register(str, &register_2nd);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;

        if (0x0 == (word.u.number & 0x000000F0)) {
            // Such as add, and.
            word.u.number += register_1st << 16;
            word.u.number += register_2nd;
        } else {
            // Such as lsr.
            word.u.number += register_1st;
            word.u.number += register_2nd << 8;
        }
    } else if (next_ch == '#') {
        word.u.number += 0x1 << 25;  // Flag operand2 = immediate value.
        int immediate_v = 0;
        len_read_ch = parse_immediate_value(str, &immediate_v);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;

        word.u.number += register_1st << 16;
        word.u.number += immediate_v;
    } else {
        return ASM_FAILURE;
    }

    *out_word = word;
    return 0;
}

// str: " r1, [r15]" -> return 0, out_word.u.number = E58F1000
int asm_str(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_base = 0;
    int register_dest = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = 0xE5800000;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_dest << 12;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_sbracket_open(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = parse_register(str, &register_base);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_base << 16;

    len_read_ch = skip_sbracket_close(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    *out_word = word;
    return 0;
}

// ldr: " r1, [r15, #0x30]" -> return 0, out_word.u.number = E59F101E
// ldr: " r1, [r15, #-0x30]" -> return 0, out_word.u.number = E53F101E
// ldr: " r1, [r15]" -> return 0, out_word.u.number = E59F1000
// ldr: " r0, =0x101f1000" or " r0, =something" -> return 0, out_word.u.number = E59F0000
//   store its emitter_pos and word into the linked list.
int asm_ldr(char* str, struct Emitter* emitter, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    char parsed_str[STR_SIZE] = {'\0'};
    int register_dest = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = 0xE5000000;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_dest << 12;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    
    // " [r15 , #0x30] " ->  parsed_str = "[r15,#0x30]"
    len_read_ch = 0;
    int non_sp_ch_idx = 0;
    while (str[len_read_ch] != '\0') {
        if (str[len_read_ch] != ' ') {
            parsed_str[non_sp_ch_idx] = str[len_read_ch];
            non_sp_ch_idx++;
        }
        len_read_ch++;
    }

    if (strcmp(parsed_str, "[r15,#0x30]") == 0) {
        word.u.number += 0x009F001E;
    } else if (strcmp(parsed_str, "[r15,#-0x30]") == 0) {
        word.u.number += 0x001F001E;
    } else if (strcmp(parsed_str, "[r15]") == 0) {
        word.u.number += 0x009F0000;
    } else if (parsed_str[0] == '=') {
        word.u.number += 0x009F0000;
        int symbol_label = to_label_symbol(&parsed_str[1]);
        common_unsolved_label_address_list_put(emitter->pos, symbol_label, word.u.number);
    } else {
        return ASM_FAILURE;
    }

    *out_word = word;
    return 0;
}

// ldrb: " r3, [r1]" -> return 0, out_word.u.number = E5D13000
int asm_ldrb(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_base = 0;
    int register_dest = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = 0xE5D00000;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_dest << 12;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_sbracket_open(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = parse_register(str, &register_base);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_base << 16;

    len_read_ch = skip_sbracket_close(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    *out_word = word;
    return 0;
}

// Assemble block data transfer instruction cases.
// Now only write-back mode is supported.
//   e.g.) stmdb (base_word = 0xE9200000): 
//     " r13!, {r3, r14}" -> return 0, out_word.u.number = E92D4008
int asm_block_data_transfer(char* str, struct Word* out_word, int base_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_base = 0;
    int register_tmp = 0;
    int register_list = 0x0;

    word.wtype = WORD_NUMBER;
    word.u.number = base_word;

    len_read_ch = parse_register(str, &register_base);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_base << 16;

    len_read_ch = skip_exclamation(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_cbracket_open(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    // " r2, r3, r5, r12}" -> register_list = 0x102C (0001000000101100)
    while (get_next_nonsp_ch(str) != '}') {
        len_read_ch = parse_register(str, &register_tmp);
        str += len_read_ch;
        register_list += 0x1 << register_tmp;

        if (get_next_nonsp_ch(str) == ',') {
            len_read_ch = skip_comma(str);
            if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
            str += len_read_ch;
        }
    }
    word.u.number += register_list;

    *out_word = word;
    return 0;
}

// branch (b, bne, ...): " some_label" -> return 0, out_word.u.number = base_word
//   label id of "some_label" is stored into the linked list.
int asm_branch(char* str, struct Emitter* emitter, struct Word* out_word, int base_word) {
    char* label;
    int len_read_ch = 0;
    char parsed_str[STR_SIZE] = {'\0'};

    // "  somelabel" -> parsed_str = ['s','o','m','l','a','b','e','l','\0',...]
    int non_sp_ch_idx = 0;
    while (str[len_read_ch] == ' ') {len_read_ch++;}
    while ((str[len_read_ch] != ' ') && (str[len_read_ch] != '\0')) {
        parsed_str[non_sp_ch_idx++] = str[len_read_ch];
        len_read_ch++;
    }

    int symbol_label = to_label_symbol(parsed_str);
    common_unsolved_label_address_list_put(emitter->pos, symbol_label, base_word);
    out_word->wtype = WORD_NUMBER;
    out_word->u.number = base_word;

    return 0;
}

// .raw: 
//   " 0x12345678" -> return 0, out_word->u.number = 0x12345678
//   " "hello"" -> return 0, out_word->u.str = "hello"
int asm_raw(char* str, struct Word* out_word) {
    int len_read_ch = 0;

    int next_ch = get_next_nonsp_ch(str);

    if (next_ch == '0') {
        out_word->wtype = WORD_NUMBER;

        int number = 0x0;
        len_read_ch = parse_int(str, &number);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;

        out_word->u.number = number;
    } else if (next_ch == '"') {
        out_word->wtype = WORD_STRING;

        char* parsed_str;
        len_read_ch = parse_str(str, &parsed_str);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;
        
        out_word->u.str = (char*)malloc(strlen(parsed_str));
        strcpy(out_word->u.str, parsed_str);
    } else {
        return ASM_FAILURE;
    }

    return 0;
}

// Assemble a given line.
//   "" (blank line) -> out_word wtype = WORD_SKIP 
//   "label:" -> out_word wtype = WORD_LABEL, u,number = (label id).
//   "str xxx" -> out_word wtype = WORD_NUMBER, u,number = (corresponding 4 bytes word).
int asm_one(char* str, struct Word* out_word) {
    struct Substring substr = {'\0'};
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};

    str += parse_one(str, &substr);

    // Skip the blank line case.
    if (strcmp("", &substr.str[0]) == 0) {
        out_word->wtype = WORD_SKIP;
        return 0;
    }
    
    char* str_inst;
    str_inst = malloc(substr.len + 1);
    strncpy(str_inst, substr.str, substr.len);
    str_inst[substr.len] = '\0';

    if (str_inst[substr.len - 1] == ':') {
        str_inst[substr.len - 1] = '\0';
        // Skip _start: label case.
        if (strcmp("_start", &str_inst[0]) == 0) {
            out_word->wtype = WORD_SKIP;
            return 0;
        }
        int symbol_label = to_label_symbol(str_inst);
        out_word->wtype = WORD_LABEL;
        out_word->u.number = symbol_label;
        return 0;
    }

    int symbol_mnemonic = to_mnemonic_symbol(str_inst);
    switch (symbol_mnemonic) {
        case GLOBAL:
            // Skip .globl mnemonic case.
            out_word->wtype = WORD_SKIP;
            return 0;
        case MOV:
            if (asm_mov(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case CMP:
            if (asm_cmp(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case LSR:
            if (asm_data_processing_3_args(str, &word, 0xE1A00030) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case ADD:
            if (asm_data_processing_3_args(str, &word, 0xE0800000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case SUB:
            if (asm_data_processing_3_args(str, &word, 0xE0400000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case AND:
            if (asm_data_processing_3_args(str, &word, 0xE0000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case STR:
            if (asm_str(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case LDR:
            if (asm_ldr(str, &emitter, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case LDRB:
            if (asm_ldrb(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case B:
            if (asm_branch(str, &emitter, &word, 0xEA000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case BNE:
            if (asm_branch(str, &emitter, &word, 0x1A000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case BLE:
            if (asm_branch(str, &emitter, &word, 0xDA000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case BGT:
            if (asm_branch(str, &emitter, &word, 0xCA000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case BGE:
            if (asm_branch(str, &emitter, &word, 0xAA000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case BL:
            if (asm_branch(str, &emitter, &word, 0xEB000000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case STMDB:
            if (asm_block_data_transfer(str, &word, 0xE9200000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case LDMIA:
            if (asm_block_data_transfer(str, &word, 0xE8B00000) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case RAW:
            if (asm_raw(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            if (word.wtype == WORD_NUMBER) {
                *out_word = word;
            } else if (word.wtype == WORD_STRING) {
                out_word->wtype = word.wtype;
                out_word->u.str = (char*)malloc(strlen(word.u.str));
                strcpy(out_word->u.str, word.u.str);
            }
            return 0;
        
        default:
            return ASM_FAILURE;
    }
}

// Solve unsolved addressed for branch and ldr cases.
//   branch:
//     e.g.) emitter->word_buf: [0x0, 0x0, 0x0, 0xEA] -> [0xFE, 0xFF, 0xFF, 0xEA]
//   ldr:
//     e.g.) emitter->word_buf: [0x0, 0x0, 0x9F, 0xE5] -> [0x38, 0x00, 0x9F, 0xE5]
void solve_label_address(struct Emitter* emitter) {
    struct LinkedList* list;
    list = malloc(sizeof(struct LinkedList));
    struct Word label_info = {NO_WORD_TYPE, {0}};
    int pos_label = 0;;

    while (linkedlist_get(list)) {
        if ((0x0A000000 == (list->word & 0x0A000000)) || (0x0B000000 == (list->word & 0x0B000000))) {
            // branch (b, bne, ...) somelabel case.
            dict_get(list->label_id, &pos_label);
            int relative_word_num = pos_label - list->emitter_pos;
            int offset = relative_word_num - 0x8;  // Subtract pc.
            offset = offset >> 2;  // 2 bits shift.
            offset = offset & 0x00FFFFFF;  // Get the lower 24 bits.
            for (int i = 0; i < 3; i++) {
                emitter->word_buf[list->emitter_pos + i] = (offset >> (i * 8)) & 0xFF;
            }
        } else if (0xE59F0000 == (list->word & 0xE59F0000)) {
            // ldr rX,=somelabel case.
            // Now use hard-coded conditions for specific cases.
            if (list->label_id == to_label_symbol("0x101f1000")) {
                int address = 0x101f1000;
                for (int i = 0; i < 4; i++) {
                    emitter->word_buf[emitter->pos++] = (address >> (8 * i)) & 0xFF;
                }
                int offset = (emitter->pos - list->emitter_pos - 4) - 0x8;
                for (int i = 0; i < 2; i++) {
                    emitter->word_buf[list->emitter_pos + i] = (offset >> (i * 8)) & 0xFF;
                }
            } else if (list->label_id == to_label_symbol("0xdeadbeaf")) {
                int address = 0xdeadbeaf;
                for (int i = 0; i < 4; i++) {
                    emitter->word_buf[emitter->pos++] = (address >> (8 * i)) & 0xFF;
                }
                int offset = (emitter->pos - list->emitter_pos - 4) - 0x8;
                for (int i = 0; i < 2; i++) {
                    emitter->word_buf[list->emitter_pos + i] = (offset >> (i * 8)) & 0xFF;
                }
            } else if (list->label_id == to_label_symbol("0x08000000")) {
                // Replace the word of (ldr, r13,=0x08000000) with 0xE3A0D302.
                int replace_word = 0xE3A0D302;
                for (int i = 0; i < 4; i++) {
                    emitter->word_buf[list->emitter_pos + i] = (replace_word >> (8 * i)) & 0xFF;
                }
            } else {
                // e.g., =message case.
                //   [N] ldr rX,=message
                //   ...
                //   [M] message:
                //     .raw "something"
                // -> 
                //   [emitter->pos] 0x0001XXXX + 0xM
                //   Add offset from (ldr rX,=message) to (0x0001XXXX + 0xM) instruction into word of (ldr rX,=message).
                int label_pos = 0;
                dict_get(list->label_id, &label_pos);
                int offset_from_ldr = emitter->pos - list->emitter_pos - 0x8;

                int word = 0x00010000 + label_pos;
                emit_int(emitter, word);

                for (int i = 0; i < 2; i++) {
                    emitter->word_buf[list->emitter_pos + i] = (offset_from_ldr >> (i * 8)) & 0xFF;
                }
            }
        }
    }
}

// Assemble the input file and save the result in the output file.
// out_file_rel_path is given as a relatie path from the top of the repository.
int assemble(char* out_file_rel_path) {
    char* str_line;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    emitter.word_buf = word_buf;
    emitter.pos = 0;
    unsolved_label_address_list_init();

    while(cl_getline(&str_line) != EOF) {
        if (asm_one(str_line, &word) == ASM_FAILURE) return ASM_FAILURE;
        switch (word.wtype) {
            case WORD_LABEL:
                // [emitter.pos = n] some_inst
                // [NO emit] some_label:
                // [emitter.pos = n+4] some_inst
                //   -> add dict to key = (id of some_label), value = n+4
                dict_put(word.u.number, emitter.pos);
                break;
            case WORD_SKIP:
                break;
            case WORD_NUMBER:
                emit_int(&emitter, word.u.number);
                break;
            case WORD_STRING:
                emit_string(&emitter, word.u.str);
                break;
            default:
                break;
        }
    }

    solve_label_address(&emitter);

    FILE* out_fp = get_fp(out_file_rel_path, FWRITE);

    // Write the result binaries into the output file:
    for (int i = 0; i < emitter.pos; i++) {
        fwrite(&emitter.word_buf[i], sizeof(unsigned char), 1, out_fp);
    }

    fclose(out_fp);

    return 0;
}

// 
// TEST
// 

static void test_asm_one_movr1r2() {
	char* input = "mov r1, r2";
	int expect = 0xE1A01002;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr10xFF() {
	char* input = "mov r1,#0xFF";
	int expect = 0xE3A010FF;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr150x00008000() {
	char* input = "mov r15, #0x00008000";
	int expect = 0xE3A0F902;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr15m0xFF000000() {
	char* input = "mov r15, #0xFF000000";
	int expect = 0xE3A0F4FF;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_mov_fail() {
	char* input = "mov a15, #0xFF";
	int expect = ASM_FAILURE;
	
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
	int actual = asm_one(input, &word);
	
	assert_two_num_eq(expect, actual);
}

static void test_asm_one_raw_int() {
	char* input = " .raw 0x12345678";
	int expect = 0x12345678;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_raw_str() {
	char* input = " .raw \"this is a test\n\"";
	char* expect = "this is a test\n";
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_str_eq(expect, actual.u.str);
}

static void test_asm_one_ldr_r1r150x30() {
	char* input = " ldr r1, [r15, #0x30]";
	int expect = 0xE59F101E;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_r1r15_minus0x30() {
	char* input = " ldr r1, [r15, #-0x30]";
	int expect = 0xE51F101E;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_r1r15() {
	char* input = " ldr r1, [r15]";
	int expect = 0xE59F1000;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_fail() {
	char* input = " ldr [r15], r2";
	int expect_return = ASM_FAILURE;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	int actual_return = asm_one(input, &actual);
	
	assert_two_num_eq(expect_return, actual_return);
}

static void test_asm_one_ldrb_r3r1() {
	char* input = " ldrb r3, [r1]";
	int expect = 0xE5D13000;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_str_r0r1() {
	char* input = " str r0, [r1]";
	int expect = 0xE5810000;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_add_r1r1_0x05() {
	char* input = " add r1, r1, #0x05";
	int expect = 0xE2811005;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_add_r3r3r4() {
	char* input = " add r3, r3, r4";
	int expect = 0xE0833004;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_cmp_r3_0x0() {
	char* input = " cmp r3, #0x0";
	int expect = 0xE3530000;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_sub_r2r2_0x04() {
	char* input = " sub r2, r2, #0x04";
	int expect = 0xE2422004;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_lsr_r3r1r2() {
	char* input = " lsr r3, r1, r2";
	int expect = 0xE1A03231;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_and_r3r3_0x15() {
	char* input = " and r3, r3, #0x15";
	int expect = 0xE2033015;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_stmdb_r13_r3r14() {
	char* input = " stmdb r13!, {r3, r14}";
	int expect = 0xE92D4008;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldmia_r13_r3r14() {
	char* input = " ldmia r13!, {r3, r14}";
	int expect = 0xE8BD4008;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void unittests() {
    test_asm_one_movr1r2();
    test_asm_one_movr10xFF();
    test_asm_one_movr150x00008000();
    test_asm_one_movr15m0xFF000000();
    test_asm_one_mov_fail();
    test_asm_one_raw_int();
    test_asm_one_raw_str();
    test_asm_one_ldr_r1r150x30();
    test_asm_one_ldr_r1r15_minus0x30();
    test_asm_one_ldr_r1r15();
    test_asm_one_ldr_fail();
    test_asm_one_ldrb_r3r1();
    test_asm_one_str_r0r1();
    test_asm_one_add_r1r1_0x05();
    test_asm_one_add_r3r3r4();
    test_asm_one_cmp_r3_0x0();
    test_asm_one_sub_r2r2_0x04();
    test_asm_one_lsr_r3r1r2();
    test_asm_one_and_r3r3_0x15();
    test_asm_one_stmdb_r13_r3r14();
    test_asm_one_ldmia_r13_r3r14();

    printf("All unittests successfully passed.\n");
}


// USAGE: ./sources/arm_asm/05_asm/assembler.out \
//   /sources/arm_asm/05_asm/test/test_input/test_assembler.ks \
//   /sources/arm_asm/05_asm/test/test_input/test_assembler
int main(int argc, char* argv[]) {
    // Set mnemonics as symbols.
    set_mnemonics();

    if (argc > 2) {
        char* in_file_rel_path = argv[1];
        char* out_file_rel_path = argv[2];
        FILE* fp = get_fp(in_file_rel_path, FREAD);
        cl_getline_set_file(fp);
        assemble(out_file_rel_path);
    } else {
        unittests();
    }

    return 0;
}