#ifndef LIBSTRPREPARE_H
#define LIBSTRPREPARE_H

#include <wchar.h>

char *xasprintf(const char *fmt, ...);
int get_char_width(wchar_t c);
int convert_to_wchar(const char *src, wchar_t **wsrc_out, size_t *wlen_out);
int compute_wchar_width(const wchar_t *w, size_t len);
void add_ellipsis(wchar_t *dest, size_t *dest_idx, size_t dest_size, const wchar_t *ellipsis, size_t ell_len);
int calculate_visual_width(const char *src);
int prepare_display_wstring(
const char *src, int max_visual_width, wchar_t *dest, size_t dest_size, int add_suffix, const wchar_t *ellipsis, int visual_offset, int use_middle_ellipsis);

#endif
