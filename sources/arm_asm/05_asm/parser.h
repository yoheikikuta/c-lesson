#pragma once

#define PARSE_FAILURE -1
#define PARSE_STR_SIZE 1024

enum ParseState {
   S_INITIAL,
   S_READING,
   S_ESCAPING,
   S_FINAL
};

struct Substring {
   char* str;
   int len;
};

/*
" mov" -> return 4, out_subs->str = "mov", out_subs->len = 3
" r12" -> return 4, out_register = 12
" ," -> return 2
" [" -> return 2
" ]" -> return 2
*/
int parse_one(char* str, struct Substring* out_subs);
int parse_register(char* str, int* out_register);
int skip_comma(char* str);
int skip_sbracket_open(char* str);
int skip_sbracket_close(char* str);