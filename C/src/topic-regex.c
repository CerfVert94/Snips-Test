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
#include"topic-regex.h"
#endif
//#define DEBUG_MODE
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
	token_t head = NULL,  new_node;
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
				new_node->token[token_len] = '\0';
				if (!new_node->token) {
					fprintf(stderr, "Malloc failed\n");
					exit(EXIT_FAILURE);
				}
				// Initialize : copy the tokenized pattern into a token node / identify its type 
				strncpy(new_node->token, &patt[token_start], token_len);
				new_node->type = identify_grammar_type(new_node->token);
				new_node->len = token_len;
				new_node->next = NULL;
				free(patt);
				return new_node;
				
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
bool match(const char* str, const char *pattern) {
	
	int patt_idx = 0,  idx = 0, match_len = 0, len = strlen(str), patt_len = strlen(pattern);
	char character_rule;
	bool mode_plus, mode_asterisk;
#ifdef DEBUG_MODE
	bool next = true;
#endif 
	token_t tok = tokenize(pattern);
	while(idx < len) {
		if (idx >= len)
			return false;
#ifdef DEBUG_MODE
		if(next) {
			next = false;
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

				if (patt_idx >= patt_len)	
					return true;
				// If plus grammar rule was skipped without matching character, then return false
				if(mode_plus && match_len == 0)
					return false;		
	
				mode_asterisk = true;
				mode_plus = false;
#ifdef DEBUG_MODE
				fprintf(stderr,"Asterisk\n");
#endif
				break;
			case TYPE_GRAMMAR_PLUS :

				//fprintf(stderr,"Plus\n");
				// Diffentiate an escape sequence by sign.
				if (tok->token[0] == '\\') 
					character_rule = -tok->token[1];
				else 
					character_rule = tok->token[0];
				if (patt_idx >= patt_len - 1)	
					return true;
				// If plus grammar rule was skipped without matching character, then return false
				if(mode_plus && match_len == 0)
					return false;			
				mode_asterisk = false;
				mode_plus = true;
#ifdef DEBUG_MODE
				fprintf(stderr,"Plus\n");
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
#ifdef DEBUG_MODE
							fprintf(stderr, "%c Plus match(%d/%d)\n", str[idx],idx + tok->len, len -1);
#endif
							match_len++;
							idx++;
							// Keep checking for the current token :
							continue;
						}
						return false;
					}
					else {
#ifdef DEBUG_MODE
	
						fprintf(stderr, "%c Plus / Exact match(%d/%d)\n", str[idx],idx + tok->len, len -1);
#endif
						// If plus grammar rule was skipped without matching character, then return false
						if(mode_plus && match_len == 0)
							return false;
						idx += tok->len;
					}

				}
				else if(strncmp(&str[idx], tok->token, tok->len)) {
					return false;
				}
#ifdef DEBUG_MODE
				
				fprintf(stderr, "%s : ",tok->token);
				fprintf(stderr, "Exact\n");	
#endif
				mode_plus = mode_asterisk = false;
				idx += tok->len;
				if(idx >= len)
					return true;
				match_len = 0;

				break;
			default :
#ifdef DEBUG_MODE
				exit_failure("Uknown pattern");
#endif
				break;			
		}
		patt_idx += tok->len+2;
		if (patt_idx < patt_len)		
			tok = tokenize(&pattern[patt_idx]);
			
#ifdef DEBUG_MODE
		next = true;
		fprintf(stderr,"TOK : %s\n",tok->token);
#endif
	}
#ifdef DEBUG_MODE
	fprintf(stderr, "\n");
#endif
	return true;

}

#ifdef MAIN
int main(int argc, char* argv[])
{
	if(match("hermes/hotword/default/detected", "/hermes\\/hotword/.+/detected")) {
		fprintf(stderr, "Match\n");
	}
	else {
		fprintf(stderr, "Mismatch\n");
	}
	if(match("hermes/intent/test", "/hermes\\/intent/.+")) {
		fprintf(stderr, "Match\n");
	}
	else {
		fprintf(stderr, "Mismatch\n");
	}
	return 0;
}
#endif


