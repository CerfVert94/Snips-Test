
struct _token_s {
	char *token;
	short type;
	int len;
	struct _token_s *next;
} token_s;
typedef struct _token_s *token_t;

short identify_grammar_type(const char* pattern);
void exit_failure(const char *format, ...); 
token_t tokenize(const char* cst_patt);
bool match(const char* str, const char *pattern);


