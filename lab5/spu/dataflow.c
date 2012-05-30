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


#ifdef DEBUG1
#define TRACE1 if(1)
#else
#define TRACE1 if(0)
#endif

#ifdef DEBUG2
#define TRACE2 if(1)
#else
#define TRACE2 if(0)
#endif

//#define DEBUG_WORK
#ifdef DEBUG_WORK
#define TRACE_WORK if(1)
#else
#define TRACE_WORK if(0)
#endif


param_t		    param A16;
int             spu_num=0;
int 		    tag[5] = {0 };
int             nvertex=0;
int             nsym=0;
unsigned int    bitset_size=0;
unsigned int    mem = 0;
unsigned int    free_mem = 0;

vertex_t fetch_vertex(unsigned int host_vertex_ptr, unsigned int index);
//void* aligned_malloc(size_t bytes, size_t alignment);

inline
unsigned int pad_length(unsigned int input){
    if(input<=1)
        return 1;
    if(input<=2)
        return 2;
    if(input<=4)
        return 4;
    if(input<=8)
        return 8;
    if(input<=16)
        return 16;

    unsigned int mod = input % 16;
    if(mod == 0){
        return input;
    } else {
        return input + 16 - mod;
    }
}

unsigned int* bitset_copy(unsigned int* bs){
    mem += bitset_size;
    unsigned int* new_bs = malloc(bitset_size);//aligned_malloc(pad_length(bitset_size), (size_t) ALIGN_CONSTANT);
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        new_bs[i] = bs[i];
    }
    return new_bs;
}

bool bitset_equals(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        if(bs1[i] != bs2[i]){
            return false;
        }
    }
    return true;
}

void bitset_or(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        bs1[i] |= bs2[i];
    }
}

void bitset_and_not(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < bitset_size / sizeof(unsigned int); ++i){
        unsigned int tmp = bs1[i] & bs2[i];
        tmp = ~tmp;
        bs1[i] = tmp & bs1[i];
    }
}

unsigned int spu_work_ctr[2];// DELETEME:
unsigned int spu_work_next[100];// DELETEME:
unsigned int spu_work_returned[100];// DELETEME:

void print_spu_work(){
    printf("SPU: spu_%d_work_next{",spu_num);
    for(unsigned int j = 0; j < spu_work_ctr[0]; ++j){
        printf("%d ",  spu_work_next[j]);
    }
    printf("}\nSPU: spu_%d_work_return{",spu_num);
    for(unsigned int j = 0; j < spu_work_ctr[1]; ++j){
        printf("%d ",  spu_work_next[j]);
    }
    printf("}\n");
}


void free_vertex(vertex_t* v){
    free_align(v->in);
    free_align(v->out);
    free_align(v->use);
    free_align(v->def);
    free(v->pred_list);
    free(v->succ_list);
    free_mem += 4*pad_length(bitset_size) + sizeof(unsigned int)*v->pred_count + sizeof(unsigned int)*v->succ_count;
}

void test_abi(){
    mail_t mail;
    mail_t mail_back;

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d vertex_done: %u\n", spu_num, mail.str.vertex_done);
    mail_back.uint = 0;
    mail_back.str.vertex_done = 1;
    printf("SPU-%d  sending vertex_done (uint: %u)\n", spu_num, mail_back.uint);
    spu_write_out_intr_mbox(mail_back.uint);

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d requeue_vertex: %u\n", spu_num, mail.str.requeue_vertex);
    mail_back.uint = 0;
    mail_back.str.requeue_vertex = 1;
    spu_write_out_intr_mbox(mail_back.uint);

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d get_next_vertex: %u\n", spu_num, mail.str.get_next_vertex);
    mail_back.uint = 0;
    mail_back.str.get_next_vertex = 1;
    spu_write_out_intr_mbox(mail_back.uint);

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d complete_execution: %u\n", spu_num, mail.str.complete_execution);
    mail_back.uint = 0;
    mail_back.str.complete_execution = 1;
    spu_write_out_intr_mbox(mail_back.uint);

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d start_execution: %u\n", spu_num, mail.str.start_execution);
    mail_back.uint = 0;
    mail_back.str.complete_execution = 1;
    spu_write_out_intr_mbox(mail_back.uint);

    mail.uint = spu_read_in_mbox();
    printf("SPU-%d vertex_number: %u\n", spu_num, mail.str.vertex_number);
    mail_back.uint = 0;
    mail_back.str.vertex_number = 134217727;
    spu_write_out_intr_mbox(mail_back.uint);
}



inline
void requeue_vertex(unsigned int index){
    mail_t mail; // Warning! Is a union used to convert between unsigned ints and a bitfield. mail.uint / mail.str.*
    mail.uint = 0;
    mail.str.requeue_vertex = 1;
    mail.str.vertex_number = index;
    spu_write_out_intr_mbox(mail.uint);  
}

int counter = 0;
void computeIn_dummy(unsigned int vertices_addr, unsigned index){
    counter = (counter + 1) % 6;
    if(spu_num == 40){
        printf("SPU dummy: fetching_vertex, counter = %d\tindex = %d\n", counter, index);
    }
    vertex_t v = fetch_vertex(vertices_addr, index);
    if(spu_num == 40){
        printf("SPU dummy: counter = %d\tv_index = %d\n", counter, v.index);
    }
    if(counter == 0){
        requeue_vertex(index);
    }
    free_vertex(&v);
}


void computeIn(unsigned int vertices_addr, unsigned index){
    
    unsigned int* old;
    vertex_t u = fetch_vertex(vertices_addr, index);
    vertex_t v;

    //printf("computeIn: vertices_addr=%u\tindex=%u\n", vertices_addr, index);
    for(unsigned int i = 0; i < u.succ_count; ++i){
        //printf("computeInt wants to fetch %u.succ_list[%u] = %u\n", index, i, u.succ_list[i]);
        v = fetch_vertex(vertices_addr, u.succ_list[i]);
        bitset_or(u.out, v.in);
        free_vertex(&v);
    }

    mfc_write_tag_mask(1<<tag[1]);
    mfc_put(u.out,  (unsigned int) u.out_backup, bitset_size, tag[1], 0, 0);



    old = bitset_copy(u.in);



	memset(u.in, 0, bitset_size);
	bitset_or(u.in, u.out);
   /* printf("computeIn BEFORE and_not: u.def[%d] (%p) = { ", index, (void*)u.def);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((u.def[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");
    printf("computeIn BEFORE and_not: u.in[%d] (%p) = { ", index, (void*)u.in);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((u.in[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");*/

	bitset_and_not(u.in, u.def);

    /*printf("computeIn AFTER and_not: u.def[%d] (%p) = { ", index, (void*)u.def);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((u.def[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");
    printf("computeIn AFTER and_not: u.in[%d] (%p) = { ", index, (void*)u.in);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((u.in[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");*/



	bitset_or(u.in, u.use);



    if(!bitset_equals(u.in, old)){
        mfc_put( u.in,  (unsigned int) u.in_backup, bitset_size, tag[1], 0, 0);

        for(unsigned int i = 0; i < u.pred_count; ++i){
            //printf("computeInt wants to fetch %u.pred_list[%u] = %u\n", index, i, u.succ_list[i]);
            v = fetch_vertex(vertices_addr, u.pred_list[i]);
            if(!v.listed){
                requeue_vertex(v.index);
            }
            free_vertex(&v);
        }
    }

    /*printf("computeIn writeback: u.in[%d] (%p) = { ", index, (void*)u.in);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((u.in[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");
*/
    free_mem += pad_length(bitset_size);
    free(old);
    mfc_read_tag_status_any();
}

void print_mail_t(mail_t mail){
    //printf("\tmail_t, uint: %u\n", mail.uint);
    printf("\tvertex_done: %u\n", mail.str.vertex_done);
    printf("\trequeue_vertex: %u\n", mail.str.requeue_vertex);
    printf("\tget_next_vertex: %u\n", mail.str.get_next_vertex);
    printf("\tcomplete_execution: %u\n", mail.str.complete_execution);
    printf("\tstart_execution: %u\n", mail.str.start_execution);
    printf("\tvertex_number: %u\n", mail.str.vertex_number);
}


void work(unsigned int host_vertex_ptr2){
    // WARNING! ASSUMPTIONS AHEAD! .start_execution is assumed to be delivered with the first vertex.
    mail_t mail; // Warning! Is a union used to convert between unsigned ints and a bitfield. mail.uint / mail.str.*
    mail.uint = spu_read_in_mbox();


    static volatile unsigned int host_vertex_ptr; // Prevent gcc from optimizing..
    host_vertex_ptr = host_vertex_ptr2;

    unsigned int vertex_nbr; 
    while(!mail.str.start_execution){
        mail.uint = spu_read_in_mbox();
        TRACE_WORK printf("SPU-%d waiting for start_execution: \n", spu_num);
    }

    //printf("SPU recieved (before loop):\n");
    //print_mail_t(mail);

    TRACE_WORK printf("start exec SPU\n");
    while(! mail.str.complete_execution){

        //printf("SPU-%d mem = %u, free_mem = %u, diff = %d\n", spu_num, mem, free_mem, mem - free_mem);

        vertex_nbr = mail.str.vertex_number;
        spu_work_next[spu_work_ctr[0]] = vertex_nbr;
        spu_work_ctr[0] +=1;
        //printf("work: host_vertex_ptr = %d\n", host_vertex_ptr);
        computeIn(host_vertex_ptr, vertex_nbr);
        //printf("computein done\n");
        mail.uint = 0;
        mail.str.vertex_done = 1;
        mail.str.get_next_vertex = 1;
        mail.str.vertex_number = vertex_nbr;


        TRACE_WORK printf("SPU-%d writing: \n", spu_num);

        spu_work_returned[spu_work_ctr[1]] = vertex_nbr;
        spu_work_ctr[1] +=1;
        spu_write_out_intr_mbox(mail.uint);

        //print_spu_work();
        TRACE_WORK printf("SPU-%d read in while: \n", spu_num);
        mail.uint = spu_read_in_mbox();
        //printf("SPU recieved (in loop):\n");
        //print_mail_t(mail);
    }
    TRACE_WORK printf("SPU-%d complete_execution recieved: \n", spu_num);

    mail.uint = 0;
    mail.str.complete_execution = 1;
    TRACE_WORK printf("SPU-%d last write with complete_execution: \n", spu_num);
    spu_write_out_intr_mbox(mail.uint);
    TRACE_WORK printf("SPU-%d done!\n", spu_num);
}

void* safe_dma(unsigned int host_addr, unsigned int fetch_size, unsigned int tag){
    unsigned int* tmp;

    if(fetch_size <= 16){
        tmp = malloc_align(fetch_size, ALIGN_EXP);//aligned_malloc(fetch_size, 16);
        mfc_get(tmp, (unsigned int) host_addr, fetch_size, tag, 0, 0);
    } else {
        tmp = malloc_align(fetch_size, ALIGN_EXP);//aligned_malloc(fetch_size, 16);
        unsigned int j = 0;
        unsigned int fetch_subsize = 0;
        while(j < fetch_size){
            fetch_subsize = ((fetch_size - j) < 16) ? (fetch_size - j) : 16;
            //printf("SPU-%d safe_dma\tfetch_size=%d   j=%d   fetch_subsize=%d\n", spu_num, fetch_size, j, fetch_subsize);
            mfc_get(tmp+j, (unsigned int) (host_addr+j), fetch_subsize, tag, 0, 0);
            j += 16;
        }
    }
    return tmp;
}

vertex_t fetch_vertex(unsigned int host_vertex_ptr, unsigned int index){
    vertex_t v A16;
    //printf("SPU fetch_vertex[%u]: host_vertex_ptr = %u\n", index, host_vertex_ptr);
    host_vertex_ptr += index * sizeof(vertex_t);

    mfc_get((void *)&v, host_vertex_ptr, sizeof(vertex_t), tag[0], 0, 0);

    // Block until first dma is done
    mfc_write_tag_mask(1<<tag[0]);
    mfc_read_tag_status_any();
    

    mem += 4 * bitset_size;

//safe_dma(unsigned int host_addr, unsigned int fetch_size, unsigned int tag)
/*
    v.in = safe_dma((unsigned int) v.in, bitset_size, tag[0]);
    v.out = safe_dma((unsigned int) v.out, bitset_size, tag[1]);
    v.use = safe_dma((unsigned int) v.use, bitset_size, tag[2]);
    v.def = safe_dma((unsigned int) v.def, bitset_size, tag[3]);
*/
    unsigned int* tmp;

    tmp = malloc_align(bitset_size, ALIGN_EXP);//aligned_malloc(bitset_size, 16);
    mfc_get(tmp, (unsigned int) v.in, bitset_size, tag[0], 0, 0);
    v.in = tmp;


    tmp = malloc_align(bitset_size, ALIGN_EXP);
    mfc_get(tmp, (unsigned int) v.out, bitset_size, tag[0], 0, 0);
    v.out = tmp;

    tmp = malloc_align(bitset_size, ALIGN_EXP);
    mfc_get(tmp, (unsigned int) v.use, bitset_size, tag[0], 0, 0);
    v.use = tmp;

    tmp = malloc_align(bitset_size, ALIGN_EXP);
    mfc_get(tmp, (unsigned int) v.def, bitset_size, tag[0], 0, 0);
    v.def = tmp;

/* CANDIDATE FOR REMOVAL
    mfc_write_tag_mask(1<<tag[0]);
    mfc_read_tag_status_any();
    int free_tag = mfc_read_tag_status_immediate();
    if(free_tag & (1 << tag[0])){
        free_tag = tag[0];
    } else if (free_tag & (1 << tag[1])){
        free_tag = tag[1];
    } else if (free_tag & (1 << tag[2])){
        free_tag = tag[2];
    } else if (free_tag & (1 << tag[3])){
        free_tag = tag[3];
    }
*/
    unsigned int *sa = 0, *pa = 0;
    if(v.succ_count != 0){
        mem += pad_length(v.succ_count) * sizeof(unsigned int);
        tmp = malloc(pad_length(v.succ_count)*sizeof(unsigned int));
        mfc_get(tmp, (unsigned int) v.succ_list, pad_length(v.succ_count)*sizeof(unsigned int), tag[0], 0, 0);
        sa = v.succ_list;
        v.succ_list = tmp;
    }

    if(v.pred_count != 0){
        mem += pad_length(v.pred_count) * sizeof(unsigned int);
        tmp = malloc(pad_length(v.pred_count) * sizeof(unsigned int));
        mfc_get(tmp, (unsigned int) v.pred_list, pad_length(v.pred_count)*sizeof(unsigned int), tag[0], 0, 0);
        pa = v.pred_list;
        v.pred_list = tmp;
    }
    mfc_write_tag_mask(1<<tag[0]);
    mfc_read_tag_status_all();


    /*printf("\n*** SPU-%d vertices[%d] ***\nlisted: %d\npred: %p (%p)\nsucc: %p (%p)\nin: %p\nout: %p\nuse: %p\ndef: %p\nsucc_count: %u\npred_count: %u\n", spu_num, v.index, v.listed, (void*)v.pred_list, (void*)pa, (void*)v.succ_list, (void*)sa, (void*)v.in, (void*)v.out, (void*)v.use, (void*)v.def, v.succ_count, v.pred_count);
    printf("succ_list: { ");
    for(unsigned int i = 0; i < v.succ_count; ++i){
        printf("%u ", v.succ_list[i]);
    }
    printf("}\npred_list: { ");
    for(unsigned int i = 0; i < v.pred_count; ++i){
        printf("%u ", v.pred_list[i]);
    }
    printf("}\nSPU-%d v[%d].use: { ", spu_num, v.index);
    for(unsigned int i = 0; i < (bitset_size/sizeof(unsigned int)); ++i){
        printf("%u ", v.use[i]);
    }
    printf("}\nSPU-%d v[%d].use: { ", spu_num, v.index);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v.use[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
    printf("}\nSPU-%d v[%d].def: { ", spu_num, v.index);
    for(unsigned int i = 0; i < (bitset_size/sizeof(unsigned int)); ++i){
        printf("%u ", v.def[i]);
    }
    printf("}\nSPU-%d v[%d].def: { ", spu_num, v.index);
	for (unsigned int i = 0; i < bitset_size; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v.def[bit_offset] & (1 << bit_local_index))){
			printf("%d ", i);
		}
	}
	printf("}\n");
    printf("}\nSPU-%d bitset_size=%u", spu_num, bitset_size);
    printf("\n----\n");*/
    return v;
}

int main(unsigned long long id, unsigned long long parm)
{
    printf("SPU, id: %llu\n", id);
    id = id;
	spu_writech(MFC_WrTagMask, -1); 
	spu_mfcdma32((void *)&param, (unsigned int)parm, sizeof(param_t), 0, MFC_GET_CMD); //TODO: Maybe reserve tag number.                
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
    mfc_write_tag_mask(1);
    mfc_read_tag_status_any();

    int j;
    int tag_num = param.proc * 5;
    for(j = 0; j < param.proc*5+5; ++j){
    	tag[j] = tag_num++;
    }

	spu_num = param.proc;
    //printf("spu_num: %d\tparam.proc: %d\n", spu_num, param.proc);
	nvertex = param.nvertex;
    nsym =  param.nsym;
    bitset_size = param.bitset_size;

	printf("SPU-%d initiated, with tags{%d, %d, %d, %d, %d}\n", spu_num, tag[0], tag[1], tag[2], tag[3], tag[4]);

    /*if(spu_num == 4){
        printf("Malloc in SPU 4:\n");
        malloc(180);
    }*/

    work((unsigned int)param.vertices);
	
    return 0;
}



void print_vertex(vertex_t* v){
	int i;

	printf("use[%d] = { ", v->index);
	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->use[bit_offset] & (1 << bit_local_index))){//bitset_get_bit(v->use, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("def[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->def[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->def, i)){
			printf("%d ", i);
		}
	}
	printf("}\n\n");
	printf("in[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->in[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->in, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("out[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->out[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->out, i)){
			printf("%d ", i);
		}
	}
	printf("}\n\n");
}



