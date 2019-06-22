#include "parser.h"
#include "cl_util.h"
#include <stdio.h>
#include <string.h>


int is_space(int c) {
    return c == ' ';
}

int is_underscore(int c) {
    return c == '_';
}

int is_dot(int c) {
    return c == '.';
}

int is_digit(int c) {
    return ('0' <= c) && (c <= '9');
}

int is_colon(int c) {
    return c == ':';
}

int is_alphabet(int c) {
    return (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z'));
}

int is_hex(int c) {
    return is_digit(c) || (('A' <= c) && (c <= 'F')) || (('a' <= c) && (c <= 'f'));
}

int is_inst_character(int c) {
    return is_underscore(c) || is_dot(c) || is_digit(c) || is_alphabet(c);
}

int is_register(int c) {
    return (c == 'r') || (c == 'R');
}

int hex2int(int c) {
    int v = 0;
    if (is_digit(c)) {
        v = c - '0';
    } else if (('a' <= c) && (c <= 'f')) {
        v = c - 'a' + 10;
    } else if (('A' <= c) && (c <= 'F')) {
        v = c - 'A' + 10;
    }
    return v;
}

// Convert 32 bits int to 12 bits immediate value:
//   (ex1) 0xC8, -1 -> return -0xC8
//   (ex2) 0xFF00000, 1 -> return 0x000004FF
int convert_32bits_to_immediate12bits(int c, int sign) {
    // Return input itself if 0 <= c <= 0xFF.
    if (0 <= c && c <= 0xFF) return sign * c;

    // 0000 0000 0000 0010 1000 0000 0000 0000 -> 15
    int lowest_bit_pos = 0;
    for (int i = 0; i < 32; i++) {
        if ((c >> i & 0x1) == 0x1) {
            lowest_bit_pos = i;
            break;
        }
    }

    // Use unsigned int to handle logical bit shift.
    unsigned int unsigned_c = (unsigned int)c;

    // 0000 0000 0000 0010 1000 0000 0000 0000 ->
    //   bit_shift_num_left = 17 ->
    //   bit_shift_num_left = 18 (only 2 * n is allowed as ARM bit shift)
    int bit_shift_num_left = 32 - lowest_bit_pos;
    bit_shift_num_left += bit_shift_num_left % 2;

    // 0000 0000 0000 0010 1000 0000 0000 0000 ->
    //   F F F F F 0 0 0 (to keep bit information shifted out)
    int num_F = (bit_shift_num_left + (bit_shift_num_left % 4)) / 4;
    unsigned int keep_bits = 0x00000000;
    for (int i = 0; i < num_F; i++) {
        keep_bits += (0x0000000F << (4 * (7 - i)));
    }

    // 0000 0000 0000 0010 1000 0000 0000 0000 ->
    //   target_bits = 0000 0000 0000 0000 0000 0000 0000 1010 ->
    //   target_bits = 0000 0000 0000 0000 0000 1001 0000 1010 (lower 12 bits = immeadiate value)
    int target_bits = ((unsigned_c & keep_bits) >> (lowest_bit_pos - lowest_bit_pos % 2));
    target_bits += ((0x00000000 + bit_shift_num_left / 2) << 8);

    return sign * target_bits;
}

// "  mov r1, r2" ->
//   return 5 (len including spaces)
//   out_subs->str = ['m','o','v','\0', ...]
//   out_subs->len = 3 (len not including spaces) 
int parse_one(char* str, struct Substring* out_subs) {    
    int len_read_ch = 0;
    int len_inst_ch = 0;
    int head_ch = str[len_read_ch];

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    out_subs->str = &str[len_read_ch];

    do {
        if (is_inst_character(head_ch)) {
            len_inst_ch++;
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_inst_character(head_ch));

    // Read one more char if the next char is ':'.
    char next_ch = str[len_read_ch];
    if (is_colon(next_ch)) {
        len_read_ch++;
        len_inst_ch++;
    }

    out_subs->len = len_inst_ch;

    return len_read_ch;
}

// " r12, r2" ->
//   return 4 (len including spaces)
//   out_register = 12
int parse_register(char* str, int* out_register) {
    int len_read_ch = 0;
    int register_num = 0;
    int head_ch = str[len_read_ch];

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is 'r' or 'R', and read the next character.
    if (!is_register(head_ch)) return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    do {
        if (is_digit(head_ch)) {
            register_num = 10 * register_num + (head_ch - '0');
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_digit(head_ch));

    // Valid register number is from 0 to 15.
    if (register_num <= 15) {
        *out_register = register_num;
        return len_read_ch;
    } else {
        return PARSE_FAILURE;
    }
}

// "  #0xC8 " ->
//   return 7 (len including spaces)
//   out_immediate_v = 0xC8
int parse_immediate_value(char* str, int* out_immediate_v) {
    int len_read_ch = 0;
    int immediate_v = 0;
    int head_ch = str[len_read_ch];
    int sign = 1;

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the prefix is "#0x".
    char prefix_ch[3] = {'#', '0', 'x'};
    for (int i = 0; i < 3; i++) {
        if(str[len_read_ch++] != prefix_ch[i]) return PARSE_FAILURE;
        if(str[len_read_ch] == '-') {
            sign = -1;
            len_read_ch++;
        }
    }
    head_ch = str[len_read_ch];

    do {
        if (is_hex(head_ch)) {
            immediate_v = 16 * immediate_v + hex2int(head_ch);
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_hex(head_ch));

    // Valid immediate value is <= 0x7FFFFFFF (since it is a signed int).
    if (immediate_v <= 0x7FFFFFFF) {
        immediate_v = convert_32bits_to_immediate12bits(immediate_v, sign);
        *out_immediate_v = immediate_v;
        return len_read_ch;
    } else {
        return PARSE_FAILURE;
    }
}

// " , r2" -> return 2 (len including spaces)
int skip_comma(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is ',', and read the next character.
    if (head_ch != ',') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// "!, r2" -> return 1
int skip_exclamation(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is '!', and read the next character.
    if (head_ch != '!') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}


// " [r2]" -> return 2 (len including spaces)
int skip_sbracket_open(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is '[', and read the next character.
    if (head_ch != '[') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// " ]" -> return 2 (len including spaces)
int skip_sbracket_close(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is ']', and read the next character.
    if (head_ch != ']') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// " {r2}" -> return 2 (len including spaces)
int skip_cbracket_open(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is '{', and read the next character.
    if (head_ch != '{') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// " }" -> return 2 (len including spaces)
int skip_cbracket_close(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is '}', and read the next character.
    if (head_ch != '}') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// "  0x1234F678 " (32 bits fully specified int) ->
//   return 12 (len including spaces)
//   out_int = 0x1234F678
int parse_int(char* str, int* out_int) {
    int len_read_ch = 0;
    int hex_int = 0x00000000;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the prefix is "0x".
    char prefix_ch[2] = {'0', 'x'};
    for (int i = 0; i < 2; i++) {
        if(str[len_read_ch++] != prefix_ch[i]) return PARSE_FAILURE;
    }
    head_ch = str[len_read_ch];

    // Convert each char to hex and make hex int.
    int pos = 7;
    do {
        if (is_hex(head_ch)) {
            hex_int += hex2int(head_ch) << (4 * pos--);
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_hex(head_ch));

    if (pos >= 0) return PARSE_FAILURE;

    *out_int = hex_int;
    return len_read_ch;
}

// "  "hello\n" " -> return 10 (len including spaces), out_str = "hello\n"
int parse_str(char* str, char** out_str) {
    char tmpbuf[PARSE_STR_SIZE] = {'\0'};
    int len_read_ch = 0;
    int copy_pos = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the double quotation exists.
    if(str[len_read_ch] != '"') return PARSE_FAILURE;

    enum ParseState state = S_READING;
    while (state != S_FINAL) {
        head_ch = str[++len_read_ch];
        switch(state) {
            case S_READING:
                if (head_ch == '\\') {
                    state = S_ESCAPING;
                    break;
                } else if (head_ch == '"') {
                    ++len_read_ch;  // Include the last double quote.
                    state = S_FINAL;
                    break;
                } else {
                    tmpbuf[copy_pos++] = head_ch;
                    break;
                }
            case S_ESCAPING:
                if (head_ch == '"' || head_ch == '\\') {
                    tmpbuf[copy_pos++] = head_ch;
                    state = S_READING;
                    break;
                } else if (head_ch == 'n') {
                    tmpbuf[copy_pos++] = '\n';
                    state = S_READING;
                    break;
                } else {
                    return PARSE_FAILURE;
                }
            case S_FINAL:
                break;
        }
    }

    char* res = (char*)malloc(copy_pos + 1);
    memcpy(res, tmpbuf, copy_pos + 1);
    res[copy_pos] = '\0';
    *out_str = res;

    return len_read_ch;
}

// 
// TEST
// 

static void test_parse_one_movr1r2_mov() {
	char* input = "mov r1, r2";
	char* expect = "mov";
	int expect_len_read = 3;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);

	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_movr1r2_mov_with_sp() {
	char* input = "  mov r1, r2";
	char* expect = "mov";
	int expect_len_read = 5;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_intermediate() {
	char* input = "  mov[";
	char* expect = "mov";
	int expect_len_read = 5;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_failure() {
	char* input = "  [";
	char* expect_len_read = PARSE_FAILURE;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_one_only_sp() {
	char* input = "   ";
	int expect_len_read = EOF;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_one_raw() {
	char* input = "  .raw";
	char* expect = ".raw";
	int expect_len_read = 6;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_movlabel() {
	char* input = "  mov:";
	char* expect = "mov:";
	int expect_len_read = 6;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_register_r1() {
	char* input = "  r1, r2";
	int expect = 1;
	int expect_len_read = 4;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
	assert_two_num_eq(expect, actual);
}

static void test_parse_register_fail() {
	char* input = "  r, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_register_fail_other_ch() {
	char* input = "  ar1, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_comma() {
	char* input = " , r2";
	int expect_len_read = 2;
	
	int actual_len_read = skip_comma(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_comma_fail() {
	char* input = " a, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual_len_read = skip_comma(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_bracket_open() {
	char* input = " [r2";
	int expect_len_read = 2;

	int actual_len_read = skip_sbracket_open(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_bracket_close() {
	char* input = " ]";
	int expect_len_read = 2;

	int actual_len_read = skip_sbracket_close(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_immediate_value() {
    char* input = "  #0xC8 ";
    int expect = 0xC8;
    int expect_len_read = 7;

    int actual;
    int actual_len_read = parse_immediate_value(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_num_eq(expect, actual);
}

static void test_parse_immediate_value_negative() {
    char* input = "  #-0xC8 ";
    int expect = -0xC8;
    int expect_len_read = 8;

    int actual;
    int actual_len_read = parse_immediate_value(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_num_eq(expect, actual);
}

static void test_parse_immediate_value_large() {
    char* input = "  #0x8000 ";
    int expect = 0x902;
    int expect_len_read = 9;

    int actual;
    int actual_len_read = parse_immediate_value(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_num_eq(expect, actual);
}

static void test_parse_immediate_value_large_negative() {
    char* input = "  #0xFF000000 ";
    int expect = 0x4FF;
    int expect_len_read = 13;

    int actual;
    int actual_len_read = parse_immediate_value(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_num_eq(expect, actual);
}

static void test_parse_immediate_value_fail() {
    char* input = "  #68 ";
    int expect_len_read = PARSE_FAILURE;

    int actual;
    int actual_len_read = parse_immediate_value(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_int() {
    char* input = "  0x1234F678 ";
    int expect = 0x1234F678;
    int expect_len_read = 12;

    int actual;
    int actual_len_read = parse_int(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_num_eq(expect, actual);
}

static void test_parse_int_fail() {
    char* input = "  0x1234F67 ";
    int expect_len_read = PARSE_FAILURE;

    int actual;
    int actual_len_read = parse_int(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_str_test() {
    char* input = "  \"Test\" ";
    char* expect = "Test";
    int expect_len_read = 8;

    char* actual;
    int actual_len_read = parse_str(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, actual);
}

static void test_parse_str_helloworld() {
    char* input = "  \"Hello World\n\" ";
    char* expect = "Hello World\n";
    int expect_len_read = 16;

    char* actual;
    int actual_len_read = parse_str(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, actual);
}

static void test_parse_str_escape_one_dq() {
    char* input = "  \"escape1 \\\" end\" ";
    char* expect = "escape1 \" end";
    int expect_len_read = 18;

    char* actual;
    int actual_len_read = parse_str(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, actual);
}

static void test_parse_str_escape_one_bs() {
    char* input = "  \"escape2 \\\\ end\" ";
    char* expect = "escape2 \\ end";
    int expect_len_read = 18;

    char* actual;
    int actual_len_read = parse_str(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, actual);
}

static void test_parse_str_escape_one_bs_end() {
    char* input = "  \"escape3 \\\\\" ";
    char* expect = "escape3 \\";
    int expect_len_read = 14;

    char** actual;
    int actual_len_read = parse_str(input, actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, *actual);
}

static void test_parse_str_escape_one_bs_dq() {
    char* input = "  \"escape4 \\\\\\\" end\" ";
    char* expect = "escape4 \\\" end";
    int expect_len_read = 20;

    char* actual;
    int actual_len_read = parse_str(input, &actual);

    assert_two_num_eq(expect_len_read, actual_len_read);
    assert_two_str_eq(expect, actual);
}

static void unittests() {
    test_parse_one_movr1r2_mov();
    test_parse_one_movr1r2_mov_with_sp();
    test_parse_one_intermediate();
    test_parse_one_failure();
    test_parse_one_only_sp();
    test_parse_one_raw();
    test_parse_one_movlabel();
    test_parse_register_r1();
    test_parse_register_fail();
    test_parse_register_fail_other_ch();
    test_skip_comma();
    test_skip_comma_fail();
    test_skip_bracket_open();
    test_skip_bracket_close();
    test_parse_immediate_value();
    test_parse_immediate_value_negative();
    test_parse_immediate_value_large();
    test_parse_immediate_value_large_negative();
    test_parse_immediate_value_fail();
    test_parse_int();
    test_parse_int_fail();
    test_parse_str_test();
    test_parse_str_helloworld();
    test_parse_str_escape_one_dq();
    test_parse_str_escape_one_bs();
    test_parse_str_escape_one_bs_end();
    test_parse_str_escape_one_bs_dq();

    printf("All unittests successfully passed.\n");
}


#if 0
int main(int argc, char* argv[]) {
    unittests();
    return 0;
}
#endif