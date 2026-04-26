#define _GNU_SOURCE
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

#include "libstrprepare.h"

char *xasprintf(const char *fmt, ...)
{
    if (!fmt)
        return NULL;
    va_list ap, ap_copy;
    va_start(ap, fmt);
    va_copy(ap_copy, ap);
    char *res = NULL;
    int len = vasprintf(&res, fmt, ap_copy);
    va_end(ap_copy);
    va_end(ap);
    if (len < 0) {
        free(res);
        res = NULL;
    }
    return res;
}

int get_char_width(wchar_t c) {
    int w = wcwidth(c);
    return (w >= 0) ? w : 1;
}

void add_ellipsis(wchar_t *dest, size_t *dest_idx, size_t dest_size, const wchar_t *ellipsis, size_t ell_len) {
    for (size_t j = 0; j < ell_len && *dest_idx < dest_size - 1; j++) {
        dest[(*dest_idx)++] = ellipsis[j];
    }
}

int compute_wchar_width(const wchar_t *w, size_t len) {
    int width = 0;
    for (size_t i = 0; i < len; i++) {
        int char_width = get_char_width(w[i]);
        if (char_width >= 0) width += char_width;
    }
    return width;
}

int calculate_visual_width(const char *src) {
    if (!src) return 0;
    wchar_t *wsrc = NULL;
    size_t wlen = 0;
    if (convert_to_wchar(src, &wsrc, &wlen) != 0) return 0;
    int total_width = compute_wchar_width(wsrc, wlen);
    free(wsrc);
    return total_width;
}

int convert_to_wchar(const char *src, wchar_t **wsrc_out, size_t *wlen_out) {
    if (!src || !wsrc_out || !wlen_out) {
        if (wsrc_out) *wsrc_out = NULL;
        if (wlen_out) *wlen_out = 0;
        return -1;
    }
    size_t src_len = strlen(src);
    wchar_t *wsrc = calloc(src_len + 1, sizeof(wchar_t));
    if (!wsrc) {
        return -1;
    }
    mbstate_t state = {0};
    const char *ptr = src;
    size_t wlen = mbsrtowcs(wsrc, &ptr, src_len + 1, &state);
    if (wlen == (size_t)-1) {
        free(wsrc);
        wsrc = calloc(src_len + 1, sizeof(wchar_t));
        if (!wsrc) return -1;
        for (size_t i = 0; i < src_len; i++) {
            wsrc[i] = (wchar_t)src[i];
        }
        wsrc[src_len] = L'\0';
        wlen = src_len;
    }
    *wsrc_out = wsrc;
    *wlen_out = wlen;
    return 0;
}

int prepare_display_wstring(
const char *src, int max_visual_width, wchar_t *dest, size_t dest_size, int add_suffix, const wchar_t *ellipsis, int visual_offset, int use_middle_ellipsis) {
    if (!src || !dest || dest_size == 0) return -1;
    dest[0] = L'\0';
    if (strlen(src) == 0) return 0;
    if (!ellipsis) ellipsis = L"..";
    wchar_t *wsrc = NULL;
    size_t wlen = 0;
    if (convert_to_wchar(src, &wsrc, &wlen) != 0) return -1;
    int total_width = calculate_visual_width(src);
    if (total_width <= max_visual_width) {
        wcsncpy(dest, wsrc, dest_size - 1);
        dest[dest_size - 1] = L'\0';
        free(wsrc);
        return 0;
    }
    size_t ell_len = wcslen(ellipsis);
    int ell_width = compute_wchar_width(ellipsis, ell_len);
    int remaining_width = max_visual_width - ell_width;
    if (remaining_width < 2) {
        wcsncpy(dest, ellipsis, dest_size - 1);
        dest[dest_size - 1] = L'\0';
        free(wsrc);
        return 0;
    }
    size_t dest_idx = 0;
    int current_width = 0;
    if (use_middle_ellipsis == 2) {
    if (visual_offset < 0) visual_offset = 0;
    if (visual_offset > total_width) visual_offset = total_width;
    int effective_total = total_width - visual_offset;
    if (effective_total < 0) effective_total = 0;
    int need_leading_ellipsis = (effective_total > max_visual_width);
    int text_remaining_width = need_leading_ellipsis ? (max_visual_width - ell_width) : effective_total;
    if (text_remaining_width < 0) text_remaining_width = 0;
    int start_visual_position = effective_total - text_remaining_width;
    if (start_visual_position < 0) start_visual_position = 0;
    int current_offset = 0;
    size_t start_i = 0;
    while (start_i < wlen) {
        int char_width = get_char_width(wsrc[start_i]);
        if (current_offset + char_width > start_visual_position) break;
        current_offset += char_width;
        start_i++;
    }

    dest_idx = 0;
    current_width = 0;
if (need_leading_ellipsis) {
    if (dest_idx + ell_len < dest_size) {
        add_ellipsis(dest, &dest_idx, dest_size, ellipsis, ell_len);
    }
}
    size_t i = start_i;
      while (i < wlen && dest_idx < dest_size - 1) {
        int char_width = get_char_width(wsrc[i]);
        if (current_width + char_width > text_remaining_width) break;
        dest[dest_idx++] = wsrc[i++];
        current_width += char_width;
    }
} else if (use_middle_ellipsis) {
    int front_width = remaining_width / 2;
    int back_width = remaining_width - front_width;
    size_t i = 0;
    while (i < wlen && dest_idx < dest_size - 1 && current_width < front_width) {
        int char_width = get_char_width(wsrc[i]);
        if (current_width + char_width > front_width) break;
        dest[dest_idx++] = wsrc[i++];
        current_width += char_width;
    }
    add_ellipsis(dest, &dest_idx, dest_size, ellipsis, ell_len);
    current_width += ell_width;
    size_t back_start = wlen;
    int back_current = 0;
    while (back_start > i && back_current < back_width) {
        back_start--;
        int char_width = get_char_width(wsrc[back_start]);
        if (back_current + char_width > back_width) {
            back_start++;
            break;
        }
        back_current += char_width;
    }
    for (size_t k = back_start; k < wlen && dest_idx < dest_size - 1; k++) {
        dest[dest_idx++] = wsrc[k];
    }
} else {
    if (visual_offset < 0) visual_offset = 0;
    if (visual_offset > total_width) visual_offset = total_width;
    int current_offset = 0;
    size_t start_i = 0;
    while (start_i < wlen) {
        int char_width = get_char_width(wsrc[start_i]);
        if (current_offset + char_width > visual_offset) break;
        current_offset += char_width;
        start_i++;
    }
    int need_end_ellipsis = (start_i < wlen && total_width - current_offset > max_visual_width);
    if (need_end_ellipsis) {
        remaining_width = max_visual_width - ell_width;
        if (remaining_width < 0) remaining_width = 0;
    } else {
        remaining_width = max_visual_width;
    }
    size_t i = start_i;
    while (i < wlen && dest_idx < dest_size - 1) {
        int char_width = get_char_width(wsrc[i]);
        if (current_width + char_width > remaining_width) break;
        dest[dest_idx++] = wsrc[i++];
        current_width += char_width;
    }
    if (need_end_ellipsis) {
        add_ellipsis(dest, &dest_idx, dest_size, ellipsis, ell_len);
    }
}
    dest[dest_idx] = L'\0';
    if (add_suffix) {
        wchar_t suffix = L'/';
        int suffix_width = get_char_width(suffix);
        if (current_width + suffix_width <= max_visual_width && dest_idx < dest_size - 1) {
            dest[dest_idx++] = suffix;
            dest[dest_idx] = L'\0';
        }
    }
    free(wsrc);
    return 0;
}
