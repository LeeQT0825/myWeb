#ifndef http11_parser_h
#define http11_parser_h

#include "http11_common.h"

// HTTP 请求解析结构体
typedef struct http_parser { 
  int cs;
  size_t body_start;
  int content_len;
  size_t nread;
  size_t mark;
  size_t field_start;
  size_t field_len;
  size_t query_start;
  int xml_sent;
  int json_sent;

  // 文本内容，所有对data的操作都是通过下面的回调完成
  void *data;

  int uri_relaxed;
  // 请求头域回调(自动添加请求头键值对)
  field_cb http_field;
  // 请求行回调
  element_cb request_method;
  element_cb request_uri;
  element_cb fragment;
  element_cb request_path;
  element_cb query_string;
  element_cb http_version;
  element_cb header_done;
  
} http_parser;

// 重置parser
int http_parser_init(http_parser *parser);
// 返回完成状态：出错：-1，完成：1，其他：0
int http_parser_finish(http_parser *parser);
// 执行分析，返回已分析的偏移量。off：起始位置
size_t http_parser_execute(http_parser *parser, const char *data, size_t len, size_t off);
// 
int http_parser_has_error(http_parser *parser);
// 
int http_parser_is_finished(http_parser *parser);

#define http_parser_nread(parser) (parser)->nread 

#endif