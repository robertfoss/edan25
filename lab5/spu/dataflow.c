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



//int             param.proc = 0;
//int             tag[5] = {0 };
//unsigned int    bitset_size;
//unsigned int    bitset_subsets;


void bitset_megaop(param_t param, unsigned int *in, unsigned int *out, unsigned int *use, unsigned int *def){
    for(unsigned int i = 0; i < param.bitset_subsets; ++i) {
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

void work(param_t param)
{
printf("SPU[%u] work()\n", param.proc);
	unsigned int inbox, offset;
    unsigned int *in = malloc_align(param.bitset_size, ALIGN_EXP);
    unsigned int *out = malloc_align(param.bitset_size, ALIGN_EXP);
    unsigned int *use = malloc_align(param.bitset_size, ALIGN_EXP);
    unsigned int *def = malloc_align(param.bitset_size, ALIGN_EXP);
    if(in == NULL || out == NULL || use == NULL || def == NULL) {
	    printf("malloc_align() failed\n");
	    exit(1);
    }
    unsigned tag_1, tag_2, tag_3, tag_4;
    unsigned int tag_id;   
    /* Reserve a tag for application usage */ 
    if ((tag_1 = mfc_tag_reserve()) == MFC_TAG_INVALID) 
    {
        printf("ERROR: unable to reserve a tag_1\n"); 
    }
    if ((tag_2 = mfc_tag_reserve()) == MFC_TAG_INVALID) 
    {
        printf("ERROR: unable to reserve a tag_2\n"); 
    }
    if ((tag_3 = mfc_tag_reserve()) == MFC_TAG_INVALID) 
    {
        printf("ERROR: unable to reserve a tag_3\n"); 
    }
    if ((tag_4 = mfc_tag_reserve()) == MFC_TAG_INVALID) 
    {
        printf("ERROR: unable to reserve a tag_4\n");
    } 

	while(1) {
		inbox = spu_read_in_mbox();

        if(inbox == UINT_MAX)
        {
            printf("SPU[%u] received exit signal.. exiting.\n", param.proc);
            return;
        }
		
		offset = param.bitset_subsets*inbox;

		mfc_get(in,  (unsigned int) (param.bs_in_addr  + offset), param.bitset_size, tag_1, 0, 0);
		mfc_get(out, (unsigned int) (param.bs_out_addr + offset), param.bitset_size, tag_2, 0, 0);
		mfc_get(use, (unsigned int) (param.bs_use_addr + offset), param.bitset_size, tag_3, 0, 0);
		mfc_get(def, (unsigned int) (param.bs_def_addr + offset), param.bitset_size, tag_4, 0, 0);
		mfc_write_tag_mask(1 << tag_1 | 1 << tag_2 | 1 << tag_3 | 1 << tag_4);
		mfc_read_tag_status_all();

D(printf("SPU[%d] index: %u  bitset_subsets: %u  offset: %u\n", param.proc, inbox, param.bitset_subsets, offset);
printf("SPU[%d]\t&use: %p\n\t&def: %p\n\t&out: %p\n\t&in:  %p\n", param.proc, (void*)param.bs_use_addr, (void*)param.bs_def_addr, (void*)param.bs_out_addr, (void*)param.bs_in_addr);
void *tmp_ptr = (void*) (param.bs_use_addr  + offset);
printf("SPU[%d] read\t\t&%p = use(%p)={", param.proc, (void*)use, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(use, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_def_addr  + offset);
printf("SPU[%d] read\t\t&%p = def(%p)={", param.proc, (void*)def, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(def, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_out_addr  + offset);
printf("SPU[%d] read\t\t&%p = out(%p)={", param.proc, (void*)out, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(out, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
tmp_ptr = (void*) (param.bs_in_addr  + offset);
printf("SPU[%d] read\t\t&%p = in (%p)={", param.proc, (void*)in, tmp_ptr);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(in, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n"));
		bitset_megaop(param, in, out, use, def);		

D(printf("SPU[%d] calculated\tin={", param.proc);
	for (int i = 0; i < 100; ++i){
	if ( bitset_get_bit(in, i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");)

		mfc_put(in, (unsigned int)  (param.bs_in_addr  +  offset), param.bitset_size, tag_1, 0, 0);
		mfc_write_tag_mask(1 << tag_1);
		mfc_read_tag_status_all();

		spu_write_out_intr_mbox(inbox);
	}
}


int main(unsigned long long id, unsigned long long parm) 
{
    param_t         param A16;
    id = id;

    unsigned int tag_id;   
    /* Reserve a tag for application usage */ 
    if ((tag_id = mfc_tag_reserve()) == MFC_TAG_INVALID) 
    {
        printf("ERROR: unable to reserve a tag\n"); 
        return 1; 
    }   

    mfc_get((void*) &param, (unsigned int) parm, sizeof(param_t), tag_id, 0, 0);
    mfc_write_tag_mask(1 << tag_id);
    mfc_read_tag_status_all();
    mfc_tag_release(tag_id);

//    int j;
//    int tag_num = param.proc * 5;
//    for(j = 0; j < param.proc*5+5; ++j){
//    	tag[j] = tag_num++;
//    }

//	param.proc = param.proc;
//    bitset_size = param.bitset_size;
//    bitset_subsets = param.bitset_subsets;

/*	printf("SPU[%d] initiated, with tags{%d, %d, %d, %d, %d} &param==%p  bitset_size==%u  bitset_subsets==%u  " \
           "&in==%p  &out==%p  &use==%p  &def==%p\n",
           param.proc, tag[0], tag[1], tag[2], tag[3], tag[4], (void*)&param, bitset_size, bitset_subsets,
           (void*)param.bs_in_addr, (void*)param.bs_out_addr, (void*)param.bs_use_addr, (void*)param.bs_def_addr);
*/	
	printf("SPU[%d] initiated, &param==%p  bitset_size==%u  bitset_subsets==%u  " \
           "&in==%p  &out==%p  &use==%p  &def==%p\n",
           param.proc, (void*)&param, param.bitset_size, param.bitset_subsets,
           (void*)param.bs_in_addr, (void*)param.bs_out_addr, (void*)param.bs_use_addr, (void*)param.bs_def_addr);
	
	work(param);
	
    return 0;
}

