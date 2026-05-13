#ifndef _API_TDNN_H
#define _API_TDNN_H

void tdnn_set_dimwnsion(uint32_t outdim, uint32_t indim);
void tdnn_start(void);
void tdnn_finish(void);
void tdnn_kick(int32_t *out, int8_t *in);
void tdnn_wait(void);

#endif // _API_TDNN_H
