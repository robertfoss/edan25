#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <spu_intrinsics.h>
#include <stdio.h>
#include <spu_mfcio.h>
#include <libsync.h>
#include <stdbool.h>
#include <string.h>
#include <libmisc.h>

#include "driver.h"
#include "vertex.h"


#define D(x) 


param_t         param A16;
int             spu_num = 0;
int             tag[5] = {0 };
unsigned int    bitset_size;
unsigned int    bitset_subsets;


void bitset_megaop(unsigned int *in, unsigned int *out, unsigned int *use, unsigned int *def){
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        in[i]  = out[i];
		in[i]  = in[i] & (~(in[i] & def[i]));
		in[i] |= use[i];
    }
}


bool bitset_get_bit(unsigned int* arr, unsigned int bit){
    unsigned int bit_offset = (bit / (sizeof(unsigned int) * 8));
    unsigned int bit_local_index = (unsigned int) (bit % (sizeof(unsigned int) * 8));
    return (arr[bit_offset]) & (1 << bit_local_index);
}

void work()
{
printf("SPU[%u] work()\n", spu_num);
	unsigned int inbox, offset;
    unsigned int *in = malloc_align(bitset_size, ALIGN_EXP);
    unsigned int *out = malloc_align(bitset_size, ALIGN_EXP);
    unsigned int *use = malloc_align(bitset_size, ALIGN_EXP);
    unsigned int *def = malloc_align(bitset_size, ALIGN_EXP);
    if(in == NULL || out == NULL || use == NULL || def == NULL) {
	    printf("malloc_align() failed\n");
	    exit(1);
    }

	while(1) {
		inbox = spu_read_in_mbox();

        if(inbox == UINT_MAX)
        {
            printf("SPU[%u] received exit signal.. exiting.\n", spu_num);
            return;
        }
		
		offset = bitset_subsets*inbox;

		mfc_get(in,  (unsigned int) (param.bs_in_addr  + offset), bitset_size, tag[0], 0, 0);
		mfc_get(out, (unsigned int) (param.bs_out_addr + offset), bitset_size, tag[1], 0, 0);
		mfc_get(use, (unsigned int) (param.bs_use_addr + offset), bitset_size, tag[2], 0, 0);
		mfc_get(def, (unsigned int) (param.bs_def_addr + offset), bitset_size, tag[3], 0, 0);
		mfc_write_tag_mask(1<<tag[0] | 1<<tag[1] | 1<<tag[2] | 1<<tag[3]);
		mfc_read_tag_status_all();

D(printf("SPU[%d] index: %u  bitset_subsets: %u  offset: %u\n", spu_num, inbox, bitset_subsets, offset);
printf("SPU[%d]\t&use: %p\n\t&def: %p\n\t&out: %p\n\t&in:  %p\n", spu_num, (void*)param.bs_use_addr, (void*)param.bs_def_addr, (void*)param.bs_out_addr, (void*)param.bs_in_addr);
void *tmp_ptr = (void*) (param.bs_use_addr  + offset);
printf("SPU[%d] read\t\t&%p = use(%p)={", spu_num, (void*)use, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(use, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_def_addr  + offset);
printf("SPU[%d] read\t\t&%p = def(%p)={", spu_num, (void*)def, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(def, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_out_addr  + offset);
printf("SPU[%d] read\t\t&%p = out(%p)={", spu_num, (void*)out, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(out, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_in_addr  + offset);
printf("SPU[%d] read\t\t&%p = in (%p)={", spu_num, (void*)in, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(in, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n"));
		bitset_megaop(in, out, use, def);		

D(printf("SPU[%d] calculated\tin={", spu_num);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(in, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");)

		mfc_put(in, (unsigned int)  (param.bs_in_addr  +  offset), bitset_size, tag[0], 0, 0);
		mfc_write_tag_mask(1<<tag[0]);
		mfc_read_tag_status_all();

		spu_write_out_intr_mbox(inbox);
	}
}


int main(unsigned long long id, unsigned long long parm) 
{

    id = id;
    mfc_get((void*) &param, (unsigned int) parm, sizeof(param_t), 1, 0, 0);
    mfc_write_tag_mask(1<<1);
    mfc_read_tag_status_all();


    int j;
    int tag_num = param.proc * 5;
    for(j = 0; j < param.proc*5+5; ++j){
    	tag[j] = tag_num++;
    }

	spu_num = param.proc;
    bitset_size = param.bitset_size;
    bitset_subsets = param.bitset_subsets;

	printf("SPU[%d] initiated, with tags{%d, %d, %d, %d, %d} bitset_size==%u  bitset_subsets==%u  " \
           "&in==%p  &out==%p  &use==%p  &def==%p\n",
           spu_num, tag[0], tag[1], tag[2], tag[3], tag[4], bitset_size, bitset_subsets, (void*)param.bs_in_addr, (void*)param.bs_out_addr, (void*)param.bs_use_addr, (void*)param.bs_def_addr);
	
	work();
	
    return 0;
}

