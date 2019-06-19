/**
Author : Roqyun KO
Library : Limited regex parser
Description : 
This is a parser that is capable of only simple regular expressions.
The regular expression format is that of node js.
This library is created soley for the snips-action-example.

1. A pattern is prefixed by a slash '/'.
2. + and * grammar work only with one character.
3. ignore case by default.
4. A few escape sequence are recognized
	a. \\
	b. \+
	c. \*
	d. \/
	e. \.
5. No global (space isn't accepted.)

Example :
Original pattern : ABC/DEF/.+/\/\+/ 
Adapted pattern for the program : ABC\/DEF\/.+\/\\\\\/\\+\/
**/

#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#ifndef _MQTT_H_
#include<mosquitto.h>
#endif

struct _token_s {
	char *token;
	short type;
	int len;
	struct _token_s *next;
} token_s;
typedef struct _token_s *token_t;

short identify_grammar_type(const char* pattern);
void exit_failure(const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(EXIT_FAILURE);
}
// Function to tokenize the pattern
token_t tokenize(const char* cst_patt)
{
	token_t head = NULL, temp = NULL, new_node;
	int idx = 0, len = strlen(cst_patt), token_start, token_len;
	char *patt;
	bool detected_esc_seq = false;
	
	patt = (char*)malloc(sizeof(char) * len + 1);

	if (!patt) {
		fprintf(stderr, "Malloc failed\n");
		exit(EXIT_FAILURE);
	}
	// Clone the pattern passed in argument (constant string can't be modified.)
	strncpy(patt, cst_patt, len);
		
		
	token_start = 0;
	token_len = 0;

	while (idx <= len) {
//		printf("%d. %c\n", idx, patt[idx]);
		// EOF or '/' signals a new pattern   
		if ((idx == len) || ((patt[idx] == '/') && !detected_esc_seq)) {
			// If the length of token is greater than 0, then a token exists => store it.
			if(token_len > 0) {
				// Allocate a memory space for a token node.
				new_node = (token_t)malloc(sizeof(struct _token_s) + 1);
				if (!new_node) {
					fprintf(stderr, "Malloc failed\n");
					exit(EXIT_FAILURE);
				}
				new_node->token = (char*)malloc(sizeof(char) * token_len + 1);
				if (!new_node->token) {
					fprintf(stderr, "Malloc failed\n");
					exit(EXIT_FAILURE);
				}
				// Initialize : copy the tokenized pattern into a token node / identify its type 
				strncpy(new_node->token, &patt[token_start], token_len);
				new_node->type = identify_grammar_type(new_node->token);
				new_node->len = token_len;
				new_node->next = NULL;

				// Define head, then add the rest at the tail of the singly linked list ADT.
				if (!head) {
					head = new_node;  
				}
				else {
					temp = head;
					while (temp->next) {
						temp = temp->next;
					}	
					temp->next = new_node;
				}
				
			}
			// The index of the next character is the start of a new pattern.
			token_start = ++idx;
			// Initialize the length of the next token.
			token_len = 0;	
			continue;	
		}
		// backslash '\' let parser ignore the grammar rule.
		else if(patt[idx] == '\\' && !detected_esc_seq) {
			detected_esc_seq = true;
			//Delete backslash	
			if(idx + 1 < len) {
				strncpy(&patt[idx], &patt[idx + 1], (len - idx) - 1);
				idx--;
				len--;
			}
		}
		// Otherwise, it's a tokenized pattern, so increment the length of it.
		else {
			detected_esc_seq = false;
			token_len++;
		}
		idx++;	
	}
	free(patt);
	return head;

}
#define TYPE_GRAMMAR_PLUS 0
#define TYPE_GRAMMAR_ASTERISK 1
#define TYPE_GRAMMAR_EXACT 2
short identify_grammar_type(const char* pattern) {
	int idx = 0, len = strlen(pattern); 
	bool detected_esc_seq = false;
	while (idx < len) {
		if (pattern[idx] == '\\' && !detected_esc_seq) {
			detected_esc_seq = true;
			idx++;
			continue;
		}
		else if (pattern[idx] == '+' && !detected_esc_seq && len <= 3) {
			return TYPE_GRAMMAR_PLUS;			
		}
		else if (pattern[idx] == '*' && !detected_esc_seq && len <= 3) {
			return TYPE_GRAMMAR_ASTERISK;			
		}

		detected_esc_seq = false;	
		idx++;
	}
	return TYPE_GRAMMAR_EXACT;
}


// Function to check regex (Only enough to check mqtt topic)
bool match(const char* str, const char *pattern)
{
	int idx = 0, match_len = 0, len = strlen(str);
	char character_rule;
	bool mode_plus, mode_asterisk, next = true;
	token_t tok = tokenize(pattern), temp;
	while(tok) {
		if (idx >= len)
			return false;
#ifdef DEBUG_MODE
		if(next) {
			next = false;
			printf("%s : ",tok->token);
		}
#endif
		switch(tok->type) {

			case TYPE_GRAMMAR_ASTERISK :
				// Diffentiate an escape sequence by sign.
				if (tok->token[0] == '\\') 
					character_rule = -tok->token[1];
				else 
					character_rule = tok->token[0];
				if(mode_plus && match_len == 0)
					return false;
				// If plus grammar rule was skipped without matching character, then return false
				if(mode_plus && match_len == 0)
					return false;			
				mode_asterisk = true;
				mode_plus = false;
#ifdef DEBUG_MODE
				printf("Asterisk\n");
#endif
				break;
			case TYPE_GRAMMAR_PLUS :
				// Diffentiate an escape sequence by sign.
				if (tok->token[0] == '\\') 
					character_rule = -tok->token[1];
				else 
					character_rule = tok->token[0];
				// If plus grammar rule was skipped without matching character, then return false
				if(mode_plus && match_len == 0)
					return false;			
				mode_asterisk = false;
				mode_plus = true;
#ifdef DEBUG_MODE
				printf("Plus\n");
#endif
				break;

			case TYPE_GRAMMAR_EXACT :
				if (len - idx < tok->len)		
					return false;
				if (mode_plus || mode_asterisk) {
					if (idx + tok->len > len) 
						return false;
					// Check for an exact match first :
					if (strncmp(&str[idx], tok->token, tok->len)) {
					// Next, check for the character preceded by + or *
						if(str[idx] == -character_rule || character_rule == '.') {
							match_len++;
							idx++;
							// Keep checking for the current token :
							continue;
						}
						return false;
					}
					else {
						// If plus grammar rule was skipped without matching character, then return false
						if(mode_plus && match_len == 0)
							return false;
						idx += tok->len;
					}

				}
				else if(strncmp(&str[idx], tok->token, tok->len)) {
					return false;
				}
				mode_plus = mode_asterisk = false;
				idx += tok->len;
				match_len = 0;
#ifdef DEBUG_MODE
				printf("Exact\n");	
#endif
				break;
			default :
#ifdef DEBUG_MODE
				exit_failure("Uknown pattern");
#endif
				break;			
		}
		next = true;
		temp = tok;
		tok = tok->next;
		free(temp);
	}
	return true;

}

#ifdef DEBUG_MODE
int main(int argc, char* argv[])
{
	if(argc <= 2) {
		fprintf(stderr, "Too few arguments ./%s [regex] [pattern]", argv[0]);
	}
	if(match(argv[1], argv[2])) {
		printf("Match\n");
	}
	else {
		printf("Mismatch\n");
	}
	return 0;
}
#endif
/*
bool match(const char* str, const char *pattern)
{
int match_len = 0;
	int str_len = strlen(str), patt_len = strlen(pattern);
	int str_idx = 0, patt_idx = 0 ;
	bool detected_esc_seq = false, all_character = false, plus = false, asterisk = false;
	char last_character = -1;
	printf("%d %s\n",patt_len, pattern);
	while (patt_idx < patt_len && str_idx < str_len) {
		detected_esc_seq = false;
	
		// Skip '/' character(s) in pattern.
		while (patt_idx < patt_len && pattern[patt_idx] == '/' && !detected_esc_seq) 
			patt_idx++;

		if (pattern[patt_idx] == '\\') {
			// Set escape sequence flag and then read next character.
			detected_esc_seq = true;
			patt_idx++;
		//	continue;
		}
		// If no esc_seq flag is set, then following characters represent grammar rules.
		if(!detected_esc_seq) {
			if (pattern[patt_idx] == '.') {
				last_character = '.';
				all_character = true;
				plus = false;
				asterisk = false;
				printf("%c %c(%d)\n", pattern[patt_idx], pattern[patt_idx + 1], pattern[patt_idx + 1] == '*');	
				if (((patt_idx + 1) < patt_len) && (pattern[patt_idx + 1] == '+')) {
					plus = true;
				}
				else if (((patt_idx + 1) < patt_len) && (pattern[patt_idx + 1] == '*')) { 
					asterisk = true;
				}
			}
			// Patterns with grammar rules such as + and * allow a string whose length is indefinite
			if (plus || asterisk) {
				patt_idx++;
				// Check for invalid grammar 
				if(last_character == -1) 
					return false;
				// Initialize the length of a matching pattern.
				match_len = 0;
				// Iterate until the pattern mismatches.
				while (str_idx < str_len && str[str_idx] != ' ') {
					// '.' grammar rule accept all characters.
					if(!all_character && last_character != str[str_idx]) 
						break;
					match_len++;		
					str_idx++;	
				}
				// Grammar rule requires at least matching 1 character.
				if(match_len <= 0 && plus)
					return false;
				// Initialize grammar rules.
				plus = asterisk = all_character = false;
			}
		}
		if (str_idx < str_len && str[str_idx] != pattern[patt_idx]) 
			return false;
		last_character = str[str_idx];
		str_idx++;
		patt_idx++;


	}	
	// If the index of either string or pattern isn't at the end, length check fails.
//	if (patt_idx < patt_len || str_idx < str_len)
//		return false;
	return true;
}*/


