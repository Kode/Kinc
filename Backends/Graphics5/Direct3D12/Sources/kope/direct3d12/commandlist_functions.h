#ifndef KOPE_D3D12_COMMANDLIST_FUNCTIONS_HEADER
#define KOPE_D3D12_COMMANDLIST_FUNCTIONS_HEADER

#include <kope/graphics5/commandlist.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters);

#ifdef __cplusplus
}
#endif

#endif
