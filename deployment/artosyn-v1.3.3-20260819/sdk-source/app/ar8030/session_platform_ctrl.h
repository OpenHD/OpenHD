#pragma once

#include "bb_api.h"

int platform_com_ctrl(bb_host_t* phost, uint32_t id, void* ipt, int ipt_len, void* opt, int opt_len);
