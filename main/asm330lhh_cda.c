#include <math.h>
#include <string.h>
#include "asm330lhh_cda.h"

int sum_abs_diff(int16_t *new_val) {
    int16_t prev_index = index_ringbuff;
    index_ringbuff = (index_ringbuff + 1)%length;
    int16_t next_index = (index_ringbuff + 1)%length;

    int16_t before_write = *(buff_x+index_ringbuff);
    *(buff_x+index_ringbuff) = *new_val;
    prev_val[0] += abs(*new_val - *(buff_x+prev_index)) - abs(before_write - *(buff_x+next_index));

    before_write = *(buff_y+index_ringbuff);
    *(buff_y+index_ringbuff) = *(new_val+1);
    prev_val[1] += abs(*(new_val+1) - *(buff_y+prev_index)) - abs(before_write - *(buff_y+next_index));
    
    before_write = *(buff_z+index_ringbuff);
    *(buff_z+index_ringbuff) = *(new_val+2);
    prev_val[2] += abs(*(new_val+2) - *(buff_z+prev_index)) - abs(before_write - *(buff_z+next_index));

    if((prev_val[0]>threshold?1:0)+(prev_val[1]>threshold?1:0)+(prev_val[2]>threshold?1:0) >= 2) {
        return 1;
    }

    return 0;
}