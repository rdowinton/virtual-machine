#ifndef _BIN_C
#define _BIN_C
#endif
#include "bin.h"

SECTION **sections;
unsigned int num_sections;

char *mapped_bin;
BIN_HDR *bin_hdr;
SECTION_HDR *section_hdrs;
SECTION_HDR *data_shdr;
SECTION_HDR *code_shdr;
char *data_section;
char *code_section;
unsigned int code_offset;

SECTION *get_section(char *name)
{
	int i;

	for(i = 0; i < num_sections; i++) {
		if(strcmp(sections[i]->name, name) == 0) {
			return sections[i];
		}
	}

	return NULL;
}

void load_bin(char *fn)
{
	struct stat s;

	if(stat(fn, &s) != 0) {
		fprintf(stderr, "stat() failed for file '%s': %s\n", fn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int fd = open(fn, O_RDONLY);

	if(!fd) {
		fprintf(stderr, "open() failed for file '%s': %s\n", fn, strerror(errno));
	}

	mapped_bin = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	if(mapped_bin == MAP_FAILED) {
		close(fd);
		fprintf(stderr, "mmap() failed for file '%s': %s\n", fn, strerror(errno));
		exit(EXIT_FAILURE);
	}

	bin_hdr = (BIN_HDR*)mapped_bin;

	if(bin_hdr->magic != BIN_MAGIC) {
		close(fd);
		fprintf(stderr, "Not a valid bin\n");
		exit(EXIT_FAILURE);
	}

	if(bin_hdr->stack_size == 0) {
		close(fd);
		fprintf(stderr, "Stack size is 0\n");
		exit(EXIT_FAILURE);
	}

	print_dbg("Stack size: %d\n", bin_hdr->stack_size);
	print_dbg("Num sections: %d\n", bin_hdr->num_sections);
	print_dbg("Data section offset: %d\n", bin_hdr->data_section);
	print_dbg("Code section offset: %d\n", bin_hdr->code_section);
	print_dbg("Entry point: %d\n", bin_hdr->entry_point);

	section_hdrs = (SECTION_HDR*)((intptr_t)mapped_bin + (intptr_t)bin_hdr->sections);

	print_dbg("sizeof(SECTION_HDR) = %d\n", sizeof(SECTION_HDR));
	int i;
	SECTION_HDR *shdr = section_hdrs;

	int d_off = -1, c_off = 1;	
	
	for(i = 0; i < bin_hdr->num_sections; i++) {
		//shdr = (SECTION_HDR*)(section_hdrs + (i * sizeof(SECTION_HDR)));
		print_dbg("Section: %s - size: %d offset: %d\n", shdr[i].name, shdr[i].size, shdr[i].offset);
		
		if(strcmp(shdr[i].name, "data") == 0) {
			d_off = shdr[i].offset;
			data_shdr = &shdr[i];
		} else if(strcmp(shdr[i].name, "code") == 0) {
			c_off = shdr[i].offset;
			code_shdr = &shdr[i];
		}
		//printf("Section: %s - size: %d offset: %d\n", shdr->name, shdr->size, shdr->offset);
	}
	
	char *p = (char*)((intptr_t)shdr + 32);//(num_sections * sizeof(SECTION_HDR)));

	data_section = (char*)(p + d_off);
	bin_hdr->entry_point -= c_off;
	code_offset = c_off;
	print_dbg("Adjusted entry point: %d\n", bin_hdr->entry_point);
	code_section = (p + c_off);//(c_off * sizeof(int64_t)));
	//int64_t *p = (int64_t*)(section_hdrs + sizeof(SECTION_HDR));

	//SECTION_HDR *p2 = (SECTION_HDR*)p;

	/*
	sections = malloc(bin_hdr->num_sections * sizeof(int));
	
	int i;
	int offset = 0;
	SECTION_HDR *shdr;

	for(i = 0; i < bin_hdr->num_sections; i++) {
		shdr = (SECTION_HDR*)(section_hdrs + offset);
		sections[i] = malloc(sizeof(SECTION));
		sections[i]->name = malloc(strlen(shdr->name) + 1);
		memset(sections[i]->name, 0, strlen(shdr->name) + 1);
		strcpy(sections[i]->name, shdr->name);
		offset += sizeof(SECTION_HDR);
		sections[i]->data = (int64_t*)(section_hdrs + offset);
		offset += (shdr->size * sizeof(int64_t));
		printf("Section %s\n", sections[i]->name);
	}
	*/
	
	/*
	data_shdr = (SECTION_HDR*)(section_hdrs + bin_hdr->data_section); 
	code_shdr = (SECTION_HDR*)(section_hdrs + bin_hdr->code_section);
	print_dbg("Data size: %d\n", data_shdr->size);
	print_dbg("Code size: %d\n", code_shdr->size);
	data_section = (char*)(section_hdrs + data_shdr->offset);
	code_section = (int64_t*)(section_hdrs + code_shdr->offset);
	*/
}

