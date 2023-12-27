#include <stdio.h>
#include <string.h>

#include "x86emu.h"
#include "services.h"
#include "i386.h"
#include "type.h"

static emulator_env x86_program_env;

void flush_log(x86emu_t* emu, char *buf, unsigned size)
{
	if(!buf || !size) return;

	printf("%.*s\n", size, buf);
}

int intr_handler(x86emu_t *emu, u8 num, unsigned type)
{
	size_t i;
	message mess;

	printf("Interrupt issued ! INT %d, type: ", num);
	switch(type & 0xFF) {
	case INTR_TYPE_FAULT:
		printf("FAULT\n");
		x86emu_stop(emu);
		return 1;
	case INTR_TYPE_SOFT:
		printf("SOFT\n");
	}

	printf("dest-src: %8x, &msg: %8x, direction: %d\n",
		emu->x86.R_EAX, emu->x86.R_EBX + emu->x86.R_DS_BASE, emu->x86.R_ECX);
	
	for(i = 0; i < sizeof(message); i ++)
		((uint8_t*)&mess)[i] =
			(uint8_t) x86emu_read_byte(emu, emu->x86.R_DS_BASE + emu->x86.R_EBX + i);

	if(interpret_message(&x86_program_env, emu->x86.R_EAX, &mess))
		emu->x86.R_EAX = -1;
	else {
		emu->x86.R_EAX = 0;
		
		if(x86_program_env.stop)
			x86emu_stop(emu);
	}
}

#define PAGE_SIZE	4096

void clear_memory(x86emu_t *emu, unsigned *addr, size_t length, int page_align, int page_flags)
{
	size_t i;

	x86emu_set_perm(emu, *addr, *addr+length, page_flags);
	for(i = 0; i < length; i ++)
		x86emu_write_byte(emu, (*addr)++, 0);
	
	if(page_align) {
		for(; (*addr) & (PAGE_SIZE - 1); (*addr) ++) {
			x86emu_write_byte(emu, *addr, 0);
			x86emu_set_perm(emu, *addr, *addr+1, page_flags);
		}
	}
}

void put_content_in_memory(x86emu_t *emu,
	unsigned *addr, size_t length,
	FILE *fp, int page_align, int page_flags)
{
	size_t i;
	int c;

	c = 0;
	for(i = 0; i < length && c != EOF; i ++) {
		c = fgetc(fp);
		x86emu_set_perm(emu, *addr, *addr+1, page_flags);
		x86emu_write_byte_noperm(emu, *addr, c);
		(*addr) ++;
	}
	*addr += length;

	clear_memory(emu, addr, 0, page_align, page_flags);
}

void push_environnement(x86emu_t *emu,
	unsigned *addr, emulator_env program_env,
	int page_align, int page_flags)
{
	int i, j;
	size_t start_off;
	int args_length[program_env.argc];
	int env_length[program_env.envc];
	
	for(i = 0; i < program_env.argc; i ++)
		args_length[i] = 1+strlen(program_env.argv[i]);

	for(i = 0; i < program_env.envc; i ++)
		env_length[i] = 1+strlen(program_env.envp[i]);

	x86emu_set_perm(emu, *addr, *addr + 4, page_flags);
	x86emu_write_dword(emu, *addr, program_env.argc);
	*addr+=4;

	start_off = 0;
	for(i = 0; i < program_env.argc; i ++) {
		x86emu_set_perm(emu, *addr + 4*i, *addr + 4*(i+1), page_flags);
		x86emu_write_dword(emu, *addr + 4*i,
			*addr + 4*(program_env.argc + program_env.envc + 1) + start_off);
		start_off += args_length[i];
	}

	for(i = 0; i < program_env.envc; i ++) {
		x86emu_set_perm(emu,
			*addr + 4*(i + program_env.argc),
			*addr + 4*(i + program_env.argc + 1),
			page_flags);

		x86emu_write_dword(emu,
			*addr + 4*(i + program_env.argc),
			*addr + 4*(program_env.argc + program_env.envc + 1) + start_off);
		start_off += env_length[i];
	}
	x86emu_set_perm(emu,
		*addr + 4*(program_env.envc + program_env.argc),
		*addr + 4*(program_env.envc + program_env.argc + 1),
		page_flags);
	x86emu_write_dword(emu, *addr + 4*(program_env.envc + program_env.argc), 0);
	*addr += 4*(program_env.envc + program_env.argc + 1);

	for(i = 0; i < program_env.argc; i ++) {
		for(j = 0; j < args_length[i]; j ++) {
			x86emu_set_perm(emu, *addr, *addr + 1, page_flags);
			x86emu_write_byte_noperm(emu, *addr, program_env.argv[i][j]);
			(*addr) ++;
		}
	}

	for(i = 0; i < program_env.envc; i ++) {
		for(j = 0; j < env_length[i]; j ++) {
			x86emu_set_perm(emu, *addr, *addr + 1, page_flags);
			x86emu_write_byte_noperm(emu, *addr, program_env.envp[i][j]);
			(*addr) ++;
		}
	}

	clear_memory(emu, addr, 0, page_align, page_flags);
}

int run_x86_emulator(emulator_env program_env, FILE *fp, struct exec header)
{
	size_t stack_size;
	unsigned int text_addr, data_addr, stack_addr, addr;
	x86emu_t *emu;

	x86_program_env = program_env;

	fseek(fp, header.a_hdrlen, SEEK_SET);

	emu = x86emu_new(0, 0);
	x86emu_reset(emu);
	x86emu_set_intr_handler(emu, intr_handler);
	
	/* log buf size of 1000000 is purely arbitrary */
	x86emu_set_log(emu, 1000000, flush_log);
	x86emu_clear_log(emu, 0);

	text_addr = 0;
	data_addr = 0x80000000;
	
	addr = 0;
	if(header.a_flags & A_SEP) {
		addr = text_addr;
		put_content_in_memory(emu, &addr, header.a_text, fp, !(header.a_flags & A_PAL), X86EMU_PERM_RX | X86EMU_PERM_VALID);
		
		addr = data_addr;
		put_content_in_memory(emu, &addr, header.a_data, fp, !(header.a_flags & A_PAL), X86EMU_PERM_RW | X86EMU_PERM_VALID);
	} else {
		data_addr = text_addr;
		addr = text_addr;
		put_content_in_memory(emu, &addr, header.a_data, fp, !(header.a_flags & A_PAL), X86EMU_PERM_RWX | X86EMU_PERM_VALID);
	}
	clear_memory(emu, &addr, header.a_bss, 1, X86EMU_PERM_RW | X86EMU_PERM_VALID);

	// The stack
	stack_addr = 0xE0000000;
	stack_size = header.a_total - (header.a_text + header.a_data + header.a_bss);
	addr = stack_addr;
	clear_memory(emu, &addr, stack_size, 0, X86EMU_PERM_RW | X86EMU_PERM_VALID);

	push_environnement(emu, &addr, program_env, 1, X86EMU_PERM_RW | X86EMU_PERM_VALID);
	
	emu->x86.R_EBP =
	emu->x86.R_ESP = stack_size + stack_addr - data_addr;
	emu->x86.R_EIP = header.a_entry;

	// Set the CPU to 32 bits
	emu->x86.R_CR0 |= 1;
	/* set default data/address size to 32 bit */
	emu->x86.R_CS_ACC = 0b110010011011;
	emu->x86.R_DS_ACC =
	emu->x86.R_ES_ACC =
	emu->x86.R_FS_ACC =
	emu->x86.R_GS_ACC =
	emu->x86.R_SS_ACC = 0b110010010011;

	/* set base */
	emu->x86.R_CS_BASE = text_addr;
    emu->x86.R_DS_BASE =
    emu->x86.R_ES_BASE =
    emu->x86.R_FS_BASE =
    emu->x86.R_GS_BASE =
	emu->x86.R_SS_BASE = data_addr;

    /* maximize descriptor limits */
    emu->x86.R_CS_LIMIT =
    emu->x86.R_DS_LIMIT =
    emu->x86.R_ES_LIMIT =
    emu->x86.R_FS_LIMIT =
    emu->x86.R_GS_LIMIT =
    emu->x86.R_SS_LIMIT = ~0;

	fclose(fp);

	printf("Text length: %8x; Data length: %8x; BSS length: %8x; Stack length: %8lx",
		header.a_text, header.a_data, header.a_bss, stack_size);
	emu->log.trace = X86EMU_TRACE_CODE | X86EMU_TRACE_REGS | X86EMU_TRACE_DATA;
	x86emu_run(emu, X86EMU_RUN_LOOP | X86EMU_RUN_NO_CODE | X86EMU_RUN_NO_EXEC);
	x86emu_dump(emu, X86EMU_DUMP_DEFAULT);
	x86emu_clear_log(emu, 1);

	printf("Done !\n");
	x86emu_done(emu);

	return x86_program_env.exit_status;
}