/*
 * Copyright (C) 2004- TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <zlib.h>

#if 1
typedef struct buffer {
	char *buf;	   /* バッファの先頭ポインタ。realloc()により変動する。*/
	size_t offset; /* 現在の読み出し位置 */
	size_t maxlen; /* バッファの最大サイズ */
	size_t len;	   /* バッファに含まれる有効なデータサイズ */
} buffer_t;
#else
typedef struct buffer buffer_t;
#endif

void buffer_clear(buffer_t *buf);
buffer_t *buffer_init(void);
void buffer_free(buffer_t *buf);
void *buffer_append_space(buffer_t * buf, size_t size);
int buffer_append(buffer_t *buf, const void *ptr, size_t size);
int buffer_append_length(buffer_t *msg, const void *ptr, size_t size);
void buffer_put_raw(buffer_t *msg, const void *ptr, size_t size);
char *buffer_get_string(char **data_ptr, int *buflen_ptr);
void buffer_put_string(buffer_t *msg, const char *ptr, size_t size);
void buffer_put_cstring(buffer_t *msg, const char *ptr);
void buffer_put_char(buffer_t *msg, int value);
void buffer_put_padding(buffer_t *msg, size_t size);
void buffer_put_int(buffer_t *msg, int value);
int buffer_len(buffer_t *msg);
char *buffer_ptr(buffer_t *msg);
void buffer_put_bignum(buffer_t *buffer, const BIGNUM *value);
void buffer_put_bignum2(buffer_t *msg, const BIGNUM *value);
void buffer_get_bignum2(char **data, BIGNUM *value);
void buffer_get_bignum2_msg(buffer_t *msg, BIGNUM *value);
void buffer_get_bignum_SECSH(buffer_t *buffer, BIGNUM *value);
int buffer_put_bignum2_bytes(buffer_t *buf, const void *v, size_t len);
void buffer_put_ecpoint(buffer_t *msg, const EC_GROUP *curve, const EC_POINT *point);
void buffer_get_ecpoint(char **data, const EC_GROUP *curve, EC_POINT *point);
void buffer_get_ecpoint_msg(buffer_t *msg, const EC_GROUP *curve, EC_POINT *point);
char *buffer_tail_ptr(buffer_t *msg);
int buffer_overflow_verify(buffer_t *msg, size_t len);
void buffer_consume(buffer_t *buf, size_t shift_byte);
void buffer_consume_end(buffer_t *buf, size_t shift_byte);
int buffer_compress(z_stream *zstream, char *payload, size_t len, buffer_t *compbuf);
int buffer_decompress(z_stream *zstream, char *payload, size_t len, buffer_t *compbuf);
int buffer_get_ret(buffer_t *msg, void *buf, size_t len);
int buffer_get_int_ret(unsigned int *ret, buffer_t *msg);
unsigned int buffer_get_int(buffer_t *msg);
int buffer_get_char_ret(char *ret, buffer_t *msg);
int buffer_get_char(buffer_t *msg);
void buffer_rewind(buffer_t *buf);
void *buffer_get_string_msg(buffer_t *msg, int *buflen_ptr);
int buffer_remain_len(buffer_t *msg);

#endif				/* BUFFER_H */
