#ifndef _BIN_H
#define _BIN_H

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>

enum data_type {
	DT_STRING = 1,
	DT_INT32 = 2
};

/*
 * idx is used to identify the variable in instructions
 * addr is relative to data section
 * strings are stored the same as any other data type, size includes \0
 * however, upon assembly strings are replaced with their address relative to data section
 */
typedef struct {
	unsigned int idx;
	enum data_type data_type;
	unsigned int offset;
	unsigned int size;
} DATA_ENTRY;

#ifdef _BIN_C
#include "vm.h"
#endif

#define BIN_MAGIC	0xAEAECFCF

#define SECTION_NAME_SIZE 8

/*
 * offset is relative to bin_hdr->sections
 */
typedef struct {
	char name[SECTION_NAME_SIZE];
	unsigned int offset;
	unsigned int size;
} SECTION_HDR;

#define SUBROUTINE_NAME_SIZE 32

typedef struct {
	char name[SUBROUTINE_NAME_SIZE];
	unsigned int offset;
	unsigned int size;
} SUBROUTINE_HDR;

/*
 * data_section and code_section each point to a SECTION_HDR relative to sections
 * e.g. data_section address = (sections + data_section)
 *
 * entry_point relative to code section
 *
 * sections is relative to end of VM_BIN
 */
typedef struct {
	unsigned int magic;
	unsigned int version;
	unsigned int stack_size;
	unsigned int entry_point;
	unsigned int num_sections;
	unsigned int data_section;
	unsigned int code_section;
	SECTION_HDR *sections;
} BIN_HDR;

typedef struct {
	char *name;
	int64_t *data;
} SECTION;

extern SECTION **sections;
extern unsigned int num_sections;

extern char *mapped_bin;
extern BIN_HDR *bin_hdr;
extern SECTION_HDR *section_hdrs;
extern SECTION_HDR *data_shdr;
extern SECTION_HDR *code_shdr;
extern char *data_section;
extern char *code_section;
extern unsigned int code_offset;

#endif

