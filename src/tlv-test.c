#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "tlv.h"

static void dump(const unsigned char *ptr, size_t len)
{
	int i, j;

	for (i = 0; i < len; i += 16) {
		printf("\t%02x:", i);
		for (j = 0; j < 16; j++) {
			if (i + j < len)
				printf(" %02hhx", ptr[i + j]);
			else
				printf("   ");
		}
		printf(" |");
		for (j = 0; j < 16 && i + j < len; j++) {
			printf("%c", (ptr[i+j] >= 0x20 && ptr[i+j] < 0x7f) ? ptr[i+j] : '.' );
		}
		printf("\n");
	}
}

static bool print_cb(void *data, const struct tlv *tlv)
{
	if (!tlv) {
		printf("NULL\n");
		return false;
	}
	printf("Tag %4hx %02zx:\n", tlv->tag, tlv->len);

	dump(tlv->value, tlv->len);

	return true;
}

int main(void) {
	struct {
		size_t len;
		const unsigned char buf[256];
	} tests[] = {
		{ 0x1c, {0x6f, 0x1a, 0x84, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0xa5, 0x08, 0x88, 0x01, 0x02, 0x5f, 0x2d, 0x02, 0x65, 0x6e}},
		{ 0x1b, {0x6f, 0x19, 0x84, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0xa5, 0x07, 0x88, 0x01, 0x02, 0x5f, 0x2d, 0x02, 0x65}},
		{ 0x1d, {0x6f, 0x1a, 0x84, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0xa5, 0x08, 0x88, 0x01, 0x02, 0x5f, 0x2d, 0x02, 0x65, 0x6e, 0x00}},
		{ 0x26, {0x6f, 0x24, 0x84, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0xa5, 0x08, 0x88, 0x01, 0x01, 0x5f, 0x2d, 0x02, 0x65, 0x6e, 0xa5, 0x08, 0x88, 0x01, 0x02, 0x5f, 0x2d, 0x02, 0x65, 0x6e}},
	};
	struct tlvdb *t;
	struct tlv *tlv;
	int i;

	for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		printf("Test %d\n", i);
		t = tlvdb_parse(tests[i].buf, tests[i].len);
		if (t) {
			tlvdb_visit(t, print_cb, NULL);
			tlv = tlvdb_get(t, 0x88, NULL);
			print_cb(NULL, tlv);
			tlv = tlvdb_get(t, 0x88, tlv);
			print_cb(NULL, tlv);
			tlvdb_free(t);
		}
	}

	return 0;
}
