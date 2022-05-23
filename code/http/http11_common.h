#ifndef _http11_common_h
#define _http11_common_h

#include <sys/types.h>

// 请求行、响应行相关回调
typedef void (*element_cb)(void *data, const char *at, size_t length);
// 报头域Content-Length，Content-Language，Content-Encoding相关回调
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif