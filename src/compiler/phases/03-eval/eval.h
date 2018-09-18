#pragma once

#include "../../ctx.h"

void do_eval(ctx_t *ctx);

value_t eval_list_block(ctx_t *ctx, const node_t *it);

value_t eval_node(ctx_t *ctx, const node_t *it);
