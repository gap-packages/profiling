/* See md5.c for explanation and copyright information.  */

#ifndef MD5_H
#define MD5_H

#include <stdint.h>

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, const uint8_t *buf, unsigned len);
void MD5Final(uint8_t digest[16], struct MD5Context *context);
void MD5Transform(uint32_t buf[4], const uint8_t in[64]);

#endif /* !MD5_H */
