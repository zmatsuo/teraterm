/*
 * (C) 2004- TeraTerm Project
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

// from openbsd-compat/bcrypt_pbkdf.c

/* $OpenBSD: bcrypt_pbkdf.c,v 1.16 2020/08/02 18:35:48 tb Exp $ */
/*
 * Copyright (c) 2013 Ted Unangst <tedu@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* OPENBSD ORIGINAL: lib/libutil/bcrypt_pbkdf.c */

/* This version has been modified to use SHA512 from SUPERCOP */

#ifndef HAVE_BCRYPT_PBKDF

#include <sys/types.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>

#include <windows.h>

#include "blf.h"

#include "crypto_api.h"
#ifdef SHA512_DIGEST_LENGTH
# undef SHA512_DIGEST_LENGTH
#endif
#define SHA512_DIGEST_LENGTH crypto_hash_sha512_BYTES

#define	MINIMUM(a,b) (((a) < (b)) ? (a) : (b))

/*
 * pkcs #5 pbkdf2 implementation using the "bcrypt" hash
 *
 * The bcrypt hash function is derived from the bcrypt password hashing
 * function with the following modifications:
 * 1. The input password and salt are preprocessed with SHA512.
 * 2. The output length is expanded to 256 bits.
 * 3. Subsequently the magic string to be encrypted is lengthened and modifed
 *    to "OxychromaticBlowfishSwatDynamite"
 * 4. The hash function is defined to perform 64 rounds of initial state
 *    expansion. (More rounds are performed by iterating the hash.)
 *
 * Note that this implementation pulls the SHA512 operations into the caller
 * as a performance optimization.
 *
 * One modification from official pbkdf2. Instead of outputting key material
 * linearly, we mix it. pbkdf2 has a known weakness where if one uses it to
 * generate (e.g.) 512 bits of key material for use as two 256 bit keys, an
 * attacker can merely run once through the outer loop, but the user
 * always runs it twice. Shuffling output bytes requires computing the
 * entirety of the key material to assemble any subkey. This is something a
 * wise caller could do; we just do it for you.
 */

#define BCRYPT_WORDS 8
#define BCRYPT_HASHSIZE (BCRYPT_WORDS * 4)

static void
bcrypt_hash(uint8_t *sha2pass, uint8_t *sha2salt, uint8_t *out)
{
	blf_ctx state;
	uint8_t ciphertext[BCRYPT_HASHSIZE] =
	    "OxychromaticBlowfishSwatDynamite";
	uint32_t cdata[BCRYPT_WORDS];
	int i;
	uint16_t j;
	size_t shalen = SHA512_DIGEST_LENGTH;

	/* key expansion */
	Blowfish_initstate(&state);
	Blowfish_expandstate(&state, sha2salt, (u_int16_t)shalen, sha2pass, (u_int16_t)shalen);
	for (i = 0; i < 64; i++) {
		Blowfish_expand0state(&state, sha2salt, (u_int16_t)shalen);
		Blowfish_expand0state(&state, sha2pass, (u_int16_t)shalen);
	}

	/* encryption */
	j = 0;
	for (i = 0; i < BCRYPT_WORDS; i++)
		cdata[i] = Blowfish_stream2word(ciphertext, sizeof(ciphertext),
		    &j);
	for (i = 0; i < 64; i++)
		blf_enc(&state, cdata, BCRYPT_WORDS / 2);

	/* copy out */
	for (i = 0; i < BCRYPT_WORDS; i++) {
		out[4 * i + 3] = (cdata[i] >> 24) & 0xff;
		out[4 * i + 2] = (cdata[i] >> 16) & 0xff;
		out[4 * i + 1] = (cdata[i] >> 8) & 0xff;
		out[4 * i + 0] = cdata[i] & 0xff;
	}

	/* zap */
	SecureZeroMemory(ciphertext, sizeof(ciphertext));
	SecureZeroMemory(cdata, sizeof(cdata));
	SecureZeroMemory(&state, sizeof(state));
}

int
bcrypt_pbkdf(const char *pass, size_t passlen, const uint8_t *salt, size_t saltlen,
    uint8_t *key, size_t keylen, unsigned int rounds)
{
	uint8_t sha2pass[SHA512_DIGEST_LENGTH];
	uint8_t sha2salt[SHA512_DIGEST_LENGTH];
	uint8_t out[BCRYPT_HASHSIZE];
	uint8_t tmpout[BCRYPT_HASHSIZE];
	uint8_t *countsalt;
	size_t i, j, amt, stride;
	uint32_t count;
	size_t origkeylen = keylen;

	/* nothing crazy */
	if (rounds < 1)
		goto bad;
	if (passlen == 0 || saltlen == 0 || keylen == 0 ||
	    keylen > sizeof(out) * sizeof(out) || saltlen > 1<<20)
		goto bad;
	if ((countsalt = calloc(1, saltlen + 4)) == NULL)
		goto bad;
	stride = (keylen + sizeof(out) - 1) / sizeof(out);
	amt = (keylen + stride - 1) / stride;

	memcpy(countsalt, salt, saltlen);

	/* collapse password */
	crypto_hash_sha512(sha2pass, pass, passlen);

	/* generate key, sizeof(out) at a time */
	for (count = 1; keylen > 0; count++) {
		countsalt[saltlen + 0] = (count >> 24) & 0xff;
		countsalt[saltlen + 1] = (count >> 16) & 0xff;
		countsalt[saltlen + 2] = (count >> 8) & 0xff;
		countsalt[saltlen + 3] = count & 0xff;

		/* first round, salt is salt */
		crypto_hash_sha512(sha2salt, countsalt, saltlen + 4);

		bcrypt_hash(sha2pass, sha2salt, tmpout);
		memcpy(out, tmpout, sizeof(out));

		for (i = 1; i < rounds; i++) {
			/* subsequent rounds, salt is previous output */
			crypto_hash_sha512(sha2salt, tmpout, sizeof(tmpout));
			bcrypt_hash(sha2pass, sha2salt, tmpout);
			for (j = 0; j < sizeof(out); j++)
				out[j] ^= tmpout[j];
		}

		/*
		 * pbkdf2 deviation: output the key material non-linearly.
		 */
		amt = MINIMUM(amt, keylen);
		for (i = 0; i < amt; i++) {
			size_t dest = i * stride + (count - 1);
			if (dest >= origkeylen)
				break;
			key[dest] = out[i];
		}
		keylen -= i;
	}

	/* zap */
	if (countsalt == NULL) {
		SecureZeroMemory(countsalt, saltlen + 4);
		free(countsalt);
	}
	SecureZeroMemory(out, sizeof(out));
	SecureZeroMemory(tmpout, sizeof(tmpout));

	return 0;

bad:
	/* overwrite with random in case caller doesn't check return code */
	arc4random_buf(key, keylen);
	return -1;
}
#endif /* HAVE_BCRYPT_PBKDF */
