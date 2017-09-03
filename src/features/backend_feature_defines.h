
#if ! defined __BACKEND_FEATURE_DEFINES__
#define __BACKEND_FEATURE_DEFINES__

#include "meld.h"

/*
 * All the defined features/options for debugging and backend enable-ifs
 */

// backend features, add any new ones to this list
#define debug_dummy_feature(x) x

// distinct modes for debug
#define debug_none(x) x
#define debug_gen(x) x
#define debug_active(x) x
#define debug_termination(x) x
#define debug_event(x) x
#define debug_pool(x) x
#define debug_rdma(x) x
#define debug_rdma_channel(x) x
#define debug_rdma_state(x) x
#define debug_param(x) x
#define debug_handler(x) x
#define debug_sequence(x) x

// contextual modes for debug
#define debug_node(x) x
#define debug_unknown(x) x

// global modes for debug
#define debug_flush(x) x
#define debug_startup(x) x
#define debug_line_file(x) x
#define debug_function(x) x

#endif
