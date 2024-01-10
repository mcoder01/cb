#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define SYMBOLS "0123456789ABCDEF"

typedef struct {
	int value;
	short int ascii_encoded;
} Base;

typedef struct opt {
	char* name;
	Base* base;
	char* help;
	struct opt* next;
} Option;

Option* opts = NULL;

struct {
	Base* in_base;
	Base* out_base;
	char* input;
	char* output;
	short int encode;
} data;

void show_error(char*);
void* check_allocation(void*);
void add_option(char*, int, char*, short int);
void show_help();
Base* select_base(char*);
void parse_args(int, char* argv[]);
void do_conversion();

int main(int argc, char* argv[]) {
	add_option("bin", 2, "Binary system. Power base of 2.", 0);
	add_option("oct", 8, "Octal system. Power base of 8.", 0);
	add_option("dec", 10, "Decimal system. Power base of 10.", 0);
	add_option("hex", 16, "Hexadecimal system. Power base of 16.", 0);
	add_option("ascii", 10, "ASCII encoding.", 1);
	parse_args(argc, argv);
	do_conversion();
	printf("%s\n", data.output);
}

void show_error(char* error) {
	fprintf(stderr, "%s\n", error);
	exit(1);
}

void* check_allocation(void* ptr) {
	if (!ptr) show_error("Memory error!");
	return ptr;
}

void add_option(char* name, int value, char* help, short int ascii_encoded) {
    Option* opt = (Option*) check_allocation(malloc(sizeof(Option)));
    opt->name = name;

	opt->base = (Base*) check_allocation(malloc(sizeof(Base)));
	opt->base->value = value;
	opt->base->ascii_encoded = ascii_encoded;

	opt->help = help;
    opt->next = opts;
    opts = opt;
}

void show_help() {
	fprintf(stderr, "Usage: ./cb <INPUT_BASE> <OUTPUT_BASE> [INPUT]\n\n");
	fprintf(stderr, "If INPUT is empty or equal to '-', reads from stdin.\n\n");

	Option* opt = opts;
	fprintf(stderr, "INPUT_BASE and OUTPUT_BASE must be integers from 1 to 16");
	fprintf(stderr, " or can be choosen from the following list:\n");
    while(opt) {
    	fprintf(stderr, " - %s\t%s\n", opt->name, opt->help);
        opt = opt->next;
    }

	fprintf(stderr, "\n");
	exit(1);
}

void parse_args(int argc, char* argv[]) {
	if (argc < 3 || argc > 4)
		show_help();

	if ((data.in_base = select_base(argv[1])) == 0
		|| (data.out_base = select_base(argv[2])) == 0)
		show_help();

	if (argc == 3 || argv[3][0] == '-') {
		data.input = (char*) check_allocation(malloc(BUF_SIZE));
		data.input[0] = '\0';
		char* buffer = (char*) check_allocation(malloc(BUF_SIZE));
		int totalSize = 1;
		while(fgets(buffer, BUF_SIZE, stdin)) {
			int len = strlen(buffer);
			totalSize += len;
			data.input = (char*) check_allocation(realloc(data.input, totalSize));
			strncat(data.input, buffer, len);
		}
	} else data.input = argv[3];
}

Base* select_base(char* arg) {
	int base = atoi(arg);
	if (base) {
		Base* out = (Base*) check_allocation(malloc(sizeof(Base)));
		out->value = base;
		out->ascii_encoded = 0;
		return out;
	}

	Option* opt = opts;
	while(opt) {
		if (strcmp(arg, opt->name) == 0)
			return opt->base;
		opt = opt->next;
	}

	return NULL;
}

int symbol_to_digit(char);
int to_decimal(char*);
char digit_to_symbol(int);
void to_output_base(int, char);

void do_conversion() {
	int len = strlen(data.input)+1;
	char* token = NULL;
	for (int i = 0; i < len; i++) {
		char c;
		if (data.in_base->ascii_encoded) {
			if (i < len-1) {
				token = (char*) check_allocation(malloc(2*sizeof(char)));
				token[0] = data.input[i];
				token[1] = '\0';
			}

			c = ' ';
		} else {
			c = data.input[i];
			data.input[i] = toupper(c);
		}
		
		if (token == NULL && c != ' ' && c != '\n' && c != '\0')
			token = &data.input[i];
		if (token != NULL && (c == ' ' || c == '\n' || c == '\0')) {
			if (!data.in_base->ascii_encoded)
				data.input[i] = '\0';
			int value = to_decimal(token);
			to_output_base(value, c);
			if (data.in_base->ascii_encoded)
				free(token);
			token = NULL;
		}
	}
}

int symbol_to_digit(char symbol) {
	if (data.in_base->ascii_encoded)
		return symbol;
	
	char* symbols = SYMBOLS;
	for (int i = 0; i < data.in_base->value; i++)
		if (symbols[i] == symbol)
			return i;

	show_error("Input is not valid!");
}

int to_decimal(char* input) {
	int result = 0;
	int len = strlen(input);
	for (int i = 0; i < len; i++) {
		int digit = symbol_to_digit(input[i]);
		result += digit*pow(data.in_base->value, len-i-1);
	}

	return result;
}

char digit_to_symbol(int digit) {
	char* symbols = SYMBOLS;
	if (digit < strlen(symbols))
		return symbols[digit];

	show_error("Unexpected error!");
}

void to_output_base(int value, char sep) {
	char* output;
	if (data.out_base->ascii_encoded) {
		output = (char*) check_allocation(malloc(sizeof(char)));
		output[0] = value;
	} else {
		int size = data.in_base->ascii_encoded ? 9 : log(value)/log(data.out_base->value)+2;
		output = (char*) check_allocation(malloc(size*sizeof(char)));
		output = &output[size-1];
		output[0] = '\0';
		size--;
		while (value > 0 || (value == 0 && data.in_base->ascii_encoded && size > 0)) {
			char symbol = digit_to_symbol(value%data.out_base->value);
			size--;
			output--;
			output[0] = symbol;
			value /= data.out_base->value;
		}
	}

	if (data.output == NULL)
		data.output = output;
	else {
		int size = strlen(data.output)+strlen(output)+2;
		data.output = (char*) check_allocation(realloc(data.output, size*sizeof(char)));
		if (sep != '\0' && !data.out_base->ascii_encoded)
			strcat(data.output, &sep);
		strcat(data.output, output);
	}
}
