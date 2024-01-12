#include <stdio.h>
#include <string.h>

#include "x86emu.h"
#include "services.h"
#include "i386.h"
#include "type.h"
#include "macros.h"

void flush_log(x86emu_t* emu, char *buf, unsigned size)
{
	if(!buf || !size) return;

	DEBUG_LOG("%.*s\n", size, buf);
}

int intr_handler(x86emu_t *emu, u8 num, unsigned type)
{
	size_t i;
	uint32_t message_ptr;
	message mess;
	int direction;

	Emulator_Env *env;

	env = (Emulator_Env*) emu->private;

	if((type & 0xFF) == INTR_TYPE_FAULT) {
		DEBUG_LOG("Interrupt %d: FAULT\n", num);
		x86emu_stop(emu);
		return 1;
	}

	direction = emu->x86.R_ECX;
	message_ptr = emu->x86.R_DS_BASE + emu->x86.R_EBX;
	for(i = 0; i < sizeof(message); i ++) {
		((uint8_t*)&mess)[i] =
			(uint8_t) x86emu_read_byte_noperm(
				emu,
				message_ptr + i
			);
		
/*
		if(i % 8 == 0)
			DEBUG_LOG("%08lx: ", message_ptr + i);
		DEBUG_LOG("%02x ", ((uint8_t*)&mess)[i]);
		if(i % 8 == 7 || i == sizeof(message) - 1)
			DEBUG_LOG("\n");
*/
	}
//	DEBUG_LOG("Pointer: %8x\n", message_ptr);

	emu->x86.R_EAX = interpret_message(env, emu->x86.R_EAX, &mess, direction);
	if(emu->x86.R_EAX)
		env->response.m_type = -1;
	else {
		env->response.m_type = 0;
		if(env->stop)
			x86emu_stop(emu);
	}

	if(direction == RECEIVE || direction == BOTH) {
		for(i = 0; i < sizeof(message); i ++) {
			x86emu_write_byte_noperm(emu,
				message_ptr + i,
				((uint8_t*)&(env->response))[i] & 0xFF
			);
			//DEBUG_LOG("w[%8lx]: %2x\n", message_ptr+i, ((uint8_t*)&mess)[i]);
		}
	}

	return 1;
}

int get_permission_and_value_to_modify(
	Emulator_Env *env, u32 addr, uint8_t **value_to_modify,
	unsigned int type, int *inverted
)
{
	size_t stack_size;
	size_t heap_size;

	int permissions;

	stack_size = array_size(&env->stack) - 1;
	heap_size = array_size(&env->heap);
	
	permissions = 0;
	if(value_to_modify)
		*value_to_modify = NULL;
	
	if(inverted) *inverted = 0;

	if(addr >= env->text_start && addr < env->text_start + env->hdr.a_text) {
		if(value_to_modify)
			*value_to_modify =
				env->text + (addr - env->text_start);
		permissions |= X86EMU_PERM_X;
	}

	if(addr >= env->data_start && addr < env->data_start + env->hdr.a_data) {
		if(value_to_modify)
			*value_to_modify =
				env->data + (addr - env->data_start);
		permissions |= X86EMU_PERM_R | X86EMU_PERM_W;
	}

	if(addr >= env->bss_start && addr < env->bss_start + env->hdr.a_bss) {
		if(value_to_modify)
			*value_to_modify =
				env->bss + (addr - env->bss_start);
		permissions |= X86EMU_PERM_R | X86EMU_PERM_W;
	}

	// If we're "pushing" a value
	if(addr >= (uint32_t)(~0 - stack_size - 4)
		&& addr < (uint32_t)(~0 - stack_size)
		&& (type & X86EMU_PERM_W)
	) {
		array_push(&env->stack, NULL);
		array_push(&env->stack, NULL);
		array_push(&env->stack, NULL);
		array_push(&env->stack, NULL);
		stack_size += 4;
	}

	if(addr > (uint32_t)(~0 - stack_size))  {
		if(value_to_modify)
			*value_to_modify =
				env->stack.array + (~0 - addr);
		permissions |= X86EMU_PERM_R | X86EMU_PERM_W;

		if(inverted) *inverted = 1;
	}

	if(addr >= env->heap_start && addr < env->heap_start + heap_size)  {
		if(value_to_modify)
			*value_to_modify =
				env->heap.array + (addr - env->heap_start);
		permissions |= X86EMU_PERM_R | X86EMU_PERM_W;
	}

	return permissions;
}

int read_byte(Emulator_Env* env, uint32_t addr)
{
	uint8_t *val;
	
	addr += env->data_start;
	get_permission_and_value_to_modify(env, addr, &val, X86EMU_PERM_R, NULL);

	return val ? *val : 0;
}

void write_byte(Emulator_Env* env, uint32_t addr, int c)
{
	uint8_t *val;
	
	addr += env->data_start;
	get_permission_and_value_to_modify(env, addr, &val, X86EMU_PERM_R, NULL);

	if(val)
		*val = c;
}

void write_dword(Emulator_Env* env, uint32_t addr, uint32_t c)
{
	int inverted;
	uint8_t *val;
	
	addr += env->data_start;
	if(
		get_permission_and_value_to_modify(env, addr+3, &val, X86EMU_PERM_R, NULL) !=
		get_permission_and_value_to_modify(env, addr, &val, X86EMU_PERM_R, &inverted)
	)
		return;

	if(!val)
		return;
	
	val[0] = c & 0xFF;
	if(inverted) {
		val[-1] = (c >>  8) & 0xFF;
		val[-2] = (c >> 16) & 0xFF;
		val[-3] = (c >> 24) & 0xFF;
	} else {
		val[1] = (c >>  8) & 0xFF;
		val[2] = (c >> 16) & 0xFF;
		val[3] = (c >> 24) & 0xFF;
	}
}

unsigned int memio_handler(x86emu_t *emu, u32 addr, u32 *val, unsigned type)
{
	uint8_t *value_to_modify;
	Emulator_Env *env;
	unsigned int bits;
	int other_perm;
	int permissions;
	int inverted;

	bits = type & 0xff;
	type &= ~0xff;

	env = (Emulator_Env*) emu->private;

	if(type == X86EMU_MEMIO_I || type == X86EMU_MEMIO_O) {
		*val = -1;
		return 1;
	}

	permissions = get_permission_and_value_to_modify(
		env, addr, &value_to_modify, type, &inverted);
	
	// Check if we have a valid value
	if(!value_to_modify) {
		*val = -1;
		return 1;
	}
	
	switch(bits) {
	case X86EMU_MEMIO_8_NOPERM:
		goto skip_permissions_check;
	case X86EMU_MEMIO_8:
		other_perm = permissions;
		break;
	case X86EMU_MEMIO_16:
		other_perm = get_permission_and_value_to_modify(
			env, addr+1, NULL, type, NULL);
		break;
	case X86EMU_MEMIO_32:
		other_perm = get_permission_and_value_to_modify(
			env, addr+3, NULL, type, NULL);
		break;
	}

	// If we're between two differents
	// segments
	if(other_perm != permissions) {
		*val = -1;
		return 1;
	}

	// Check permissions
	if(
		!((permissions & X86EMU_PERM_X) && (type == X86EMU_MEMIO_X)) &&
		!((permissions & X86EMU_PERM_R) && (type == X86EMU_MEMIO_R)) &&
		!((permissions & X86EMU_PERM_W) && (type == X86EMU_MEMIO_W))
	) {
		*val = -1;
		return 1;
	}

skip_permissions_check:
	switch(type) {
	case X86EMU_MEMIO_X:
	case X86EMU_MEMIO_R:
		switch(bits) {
		case X86EMU_MEMIO_8:
		case X86EMU_MEMIO_8_NOPERM:
			*val = value_to_modify[0];
			break;
		case X86EMU_MEMIO_16:
			*val  = value_to_modify[0];
			if(inverted)
				*val |= value_to_modify[-1] << 8;
			else
				*val |= value_to_modify[1] << 8;
			break;
		case X86EMU_MEMIO_32:
			*val  = value_to_modify[0];
			if(inverted) {
				*val |= value_to_modify[-1] << 8;
				*val |= value_to_modify[-2] << 16;
				*val |= value_to_modify[-3] << 24;
			} else {
				*val |= value_to_modify[1] << 8;
				*val |= value_to_modify[2] << 16;
				*val |= value_to_modify[3] << 24;
			}
			break;
		}
		break;
	case X86EMU_MEMIO_W:
		switch(bits) {
		case X86EMU_MEMIO_8:
		case X86EMU_MEMIO_8_NOPERM:
			value_to_modify[0] = (*val) & 0xFF;
			break;
		case X86EMU_MEMIO_16:
			value_to_modify[0] = (*val) & 0xFF;

			if(inverted)
				value_to_modify[-1] = ((*val) >> 8) & 0xFF;
			else
				value_to_modify[1] = ((*val) >> 8) & 0xFF;
			break;
		case X86EMU_MEMIO_32:
			value_to_modify[0] = (*val) & 0xFF;
			if(inverted) {
				value_to_modify[-1] = ((*val) >>  8) & 0xFF;
				value_to_modify[-2] = ((*val) >> 16) & 0xFF;
				value_to_modify[-3] = ((*val) >> 24) & 0xFF;
			} else {
				value_to_modify[1] = ((*val) >>  8) & 0xFF;
				value_to_modify[2] = ((*val) >> 16) & 0xFF;
				value_to_modify[3] = ((*val) >> 24) & 0xFF;
			}
			break;
		}
		break;
	}
	
	return 0;
}

void x86_init_stack(Emulator_Env *env, int argc, char* argv[])
{
	int i, j;
	cpu_ptr address;
	char c;
	size_t stack_size;
	size_t arg_size;
	size_t args_start[argc];

	for(i = argc-1; i >= 0; i --) {
		arg_size = strlen(argv[i]);
		for(j = arg_size; j >= 0; j --)
			array_push(&env->stack, &argv[i][j]);
		args_start[i] = array_size(&env->stack) - 1;
	}

	// Push envp (TODO)
	for(j = 0; j < 4; j ++) {
		c = (0 >> (8*(3-j))) & 0xFF;
		array_push(&env->stack, &c);
	}

	array_push(&env->stack, NULL);
	array_push(&env->stack, NULL);
	array_push(&env->stack, NULL);
	array_push(&env->stack, NULL);
	for(i = argc-1; i >= 0; i --) {
		address = ~0 - args_start[i] - env->data_start;
		for(j = 0; j < 4; j ++) {
			c = (address >> (8*(3-j))) & 0xFF;
			array_push(&env->stack, &c);
		}
	}

	// Push argc
	for(j = 0; j < 4; j ++) {
		c = (argc >> (8*(3-j))) & 0xFF;
		array_push(&env->stack, &c);
	}

	env->stack_ptr_start = array_size(&env->stack) - 1;
	stack_size = env->hdr.a_total - (
		env->hdr.a_bss +
		env->hdr.a_data +
		env->hdr.a_text
	//TODO: Change 8192
	) + 8192;
	for(i = 0; i < (int) stack_size; i ++)
		array_push(&env->stack, NULL);
}

void x86_emulator_init(Emulator_Env *env, int argc, char* argv[])
{
	env->read_byte = read_byte;
	env->write_byte = write_byte;
	env->write_dword = write_dword;

	env->text_start = 0x0;
	if(TEXT_DATA_SEPERARED(env))
		env->data_start = 0x80000000;
	else
		env->data_start = env->text_start;
	env->bss_start = env->data_start + env->hdr.a_data;
	env->heap_start = env->bss_start + env->hdr.a_bss;

	x86_init_stack(env, argc, argv);
}

int run_x86_emulator(Emulator_Env *env)
{
	x86emu_t *emu;

	emu = x86emu_new(0, 0);
	x86emu_reset(emu);
	x86emu_set_intr_handler(emu, intr_handler);
	x86emu_set_memio_handler(emu, memio_handler);
	emu->private = env;
	/* log buf size of 1000000 is purely arbitrary */
	x86emu_set_log(emu, 1000000, flush_log);
	x86emu_clear_log(emu, 0);

	emu->x86.R_EBP =
	emu->x86.R_ESP = ~0
		- env->stack_ptr_start
		- env->data_start;
	emu->x86.R_EIP = env->hdr.a_entry;

	// Set the CPU to 32 bits
	emu->x86.R_CR0 |= 1;
	/* set default data/address size to 32 bit */
	emu->x86.R_CS =
	emu->x86.R_DS =
	emu->x86.R_ES =
	emu->x86.R_FS =
	emu->x86.R_GS =
	emu->x86.R_SS = 0;

	emu->x86.R_CS_ACC = 0b110011111011;
	emu->x86.R_DS_ACC =
	emu->x86.R_ES_ACC =
	emu->x86.R_FS_ACC =
	emu->x86.R_GS_ACC =
	emu->x86.R_SS_ACC = 0b110011110011;

	/* set base */
	emu->x86.R_CS_BASE = env->text_start;
    emu->x86.R_DS_BASE =
    emu->x86.R_ES_BASE =
    emu->x86.R_FS_BASE =
    emu->x86.R_GS_BASE =
	emu->x86.R_SS_BASE = env->data_start;

    /* maximize descriptor limits */
    emu->x86.R_CS_LIMIT =
    emu->x86.R_DS_LIMIT =
    emu->x86.R_ES_LIMIT =
    emu->x86.R_FS_LIMIT =
    emu->x86.R_GS_LIMIT =
    emu->x86.R_SS_LIMIT = ~0;

	DEBUG_LOG("Text length: %8x; Data length: %8x; BSS length: %8x; Stack length: %8lx\n",
		env->hdr.a_text, env->hdr.a_data, env->hdr.a_bss, array_size(&env->stack));

	emu->log.trace = 0;//X86EMU_TRACE_CODE | X86EMU_TRACE_REGS | X86EMU_TRACE_DATA;
	x86emu_run(emu, X86EMU_RUN_LOOP | X86EMU_RUN_NO_CODE | X86EMU_RUN_NO_EXEC);
	x86emu_dump(emu, X86EMU_DUMP_DEFAULT);
	x86emu_clear_log(emu, 1);

	DEBUG_LOG("Done !\n");
	x86emu_done(emu);

	return env->exit_status;
}
