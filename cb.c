#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define SYMBOLS "0123456789ABCDEF"

typedef struct opt {
	char* name;
	int value;
	char* help;
	struct opt* next;
} Option;

Option* opts = NULL;

struct {
	unsigned int in_base;
	unsigned int out_base;
	char* input;
	char* output;
} data;

void add_option(char*, int, char*);
void parse_args(int, char* argv[]);
void show_help();
void do_conversion();

int main(int argc, char* argv[]) {
	add_option("bin", 2, "Binary system. Power base: 2");
	add_option("oct", 8, "Octal system. Power base: 8");
	add_option("dec", 10, "Decimal system. Power base: 10");
	add_option("hex", 16, "Hexadecimal system. Power base: 16");
	parse_args(argc, argv);
	do_conversion();
	printf("%s\n", data.output);
}

void add_option(char* name, int value, char* help) {
    Option* opt = (Option*) malloc(sizeof(Option));
    opt->name = name;
	opt->value = value;
	opt->help = help;
    opt->next = opts;
    opts = opt;
}

int select_base(char* arg) {
	Option* opt = opts;
	while(opt) {
		if (strcmp(arg, opt->name) == 0)
			return opt->value;
		opt = opt->next;
	}

	return 0;
}

void parse_args(int argc, char* argv[]) {
	if (argc < 5 || argc > 6) {
		show_help();
		exit(1);
	}

	if (strcmp(argv[1], "from") != 0 ||
		(data.in_base = select_base(argv[2])) == 0) {
		show_help();
		exit(1);
	}

	if (strcmp(argv[3], "to") != 0 ||
		(data.out_base = select_base(argv[4])) == 0) {
		show_help();
		exit(1);
	}

	if (argc == 5) {
		data.input = (char*) malloc(BUF_SIZE);
		data.input[0] = '\0';
		char* buffer = (char*) malloc(BUF_SIZE);
		int totalSize = 1;
		while(fgets(buffer, BUF_SIZE, stdin)) {
			totalSize += strlen(buffer);
			data.input = (char*) realloc(data.input, totalSize);
			strcat(data.input, buffer);
		}
	} else data.input = argv[5];
}

void show_help() {
	printf("Usage: ./cb from <INPUT_BASE> to <OUTPUT_BASE> [INPUT]\n\n");
	printf("If INPUT is empty or equal to '-', reads from stdin.\n\n");

	Option* opt = opts;
	printf("INPUT_BASE and OUTPUT_BASE can be choosen from the following list:\n");
    while(opt) {
    	printf(" - %s\t%s\n", opt->name, opt->help);
        opt = opt->next;
    }
	printf("\n");
}

int symbol_to_digit(char);
int to_decimal(char*);
char digit_to_symbol(int);
void to_output_base(int, char);

void do_conversion() {
	int len = strlen(data.input)+1;
	char* token = NULL;
	for (int i = 0; i < len; i++) {
		char c = data.input[i];
		data.input[i] = toupper(c);
		if (token == NULL && c != ' ' && c != '\n' && c != '\0')
			token = &data.input[i];
		if (token != NULL && (c == ' ' || c == '\n' || c == '\0')) {
			data.input[i] = '\0';
			int value = to_decimal(token);
			to_output_base(value, c);
			token = NULL;
		}
	}
}

int symbol_to_digit(char symbol) {
	char* symbols = SYMBOLS;
	for (int i = 0; symbols[i]; i++)
		if (symbols[i] == symbol)
			return i;

	printf("Error: input is not valid!\n");
	exit(1);
}

int to_decimal(char* input) {
	int result = 0;
	int len = strlen(input);
	for (int i = 0; i < len; i++) {
		int digit = symbol_to_digit(input[i]);
		result += digit*pow(data.in_base, len-i-1);
	}

	return result;
}

char digit_to_symbol(int digit) {
	char* symbols = SYMBOLS;
	if (digit < strlen(symbols))
		return symbols[digit];

	printf("Unexpected error!\n");
	exit(1);
}

void to_output_base(int value, char sep) {
	int size = log(value)/log(data.out_base)+2;
	char* output = (char*) malloc(size*sizeof(char));
	output = &output[size-1];
	output[0] = '\0';
	while (value > 0) {
		char symbol = digit_to_symbol(value%data.out_base);
		output--;
		output[0] = symbol;
		value /= data.out_base;
	}

	if (data.output == NULL)
		data.output = output;
	else {
		size = strlen(data.output)+strlen(output)+2;
		data.output = (char*) realloc(data.output, size*sizeof(char));
		if (sep != '\0')
			strcat(data.output, &sep);
		strcat(data.output, output);
	}
}
