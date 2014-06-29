#include "asm_data.h"

int lookup_addr(char *var_name)
{
	return 0xabcd;
}

int lookup_val(char *var_name)
{
	return 0xabcd;
}

unsigned int
parse_str(char *src, char *dest, int max)
{
	char *p = src;
	unsigned int idx = 0;

	while(*p) {
		if(*p != '\\') {
			dest[idx++] = *p++;
		} else {
			p++;

			switch(*p++) {
				case 't':
					dest[idx++] = '\t';
				break;
				case 'r':
					dest[idx++] = '\r';
				break;
				case 'n':
					dest[idx++] = '\n';
				break;
				case 'a':
					dest[idx++] = '\a';
				break;
				case '"':
					dest[idx++] = '"';
				break;
				case '\\':
					dest[idx++] = '\\';
				break;
				default:
					fprintf(stderr, "Unknown escape character \\%c", *p);
					exit(EXIT_FAILURE);
				break;
			}
		}
	}

	dest[idx++] = 0;

	return idx;
}

char *asm_data(char *line, DATA_ENTRY *entry, char **name_out)
{
	char *data = malloc(1024);

	print_dbg("Data line: %s\n", line);

	if(strncmp(line, "string", 6) == 0) {
		char *p = trim_ws(line + 6);
		char *tmp = strstr(p, " ");
	
		*tmp = 0;

		*name_out = malloc(strlen(p) + 1);
		memset(*name_out, 0, strlen(p) + 1);
		strcpy(*name_out, p);

		p = tmp + 1;
		tmp = strstr(p, "\"");
		p = tmp + 1;
		tmp = strstr(p, "\"");

		/* Account for escaped '"' in string */
		while(*(tmp - 1) == '\\') {
			tmp = strstr(tmp, "\"");
		}

		*tmp = 0;

		entry->data_type = DT_STRING;
		entry->size = parse_str(p, data, 1024);

		print_dbg("String '%s': [%s]\n", *name_out, data);
	} else if(strncmp(line, "int32", 5) == 0) {
		char *p = line + 5;

		while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

		char *tmp = strstr(p, " ");
		*tmp = 0;

		*name_out = malloc(strlen(p) + 1);
		memset(*name_out, 0, strlen(p) + 1);
		strcpy(*name_out, p);

		p = tmp + 1;

		while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

		int val;

		if(strncmp(p, "0x", 2) == 0) {
			val = strtol(p, NULL, 0);	
		} else {
			val = atoi(p);
		}

		entry->data_type = DT_INT32;
		entry->size = sizeof(int);
		memcpy(data, &val, sizeof(int));

		print_dbg("Int32 '%s': %d\n", *name_out, val);
	} else {
		free(data);
	}

	return data;
}


