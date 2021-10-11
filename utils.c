#include <efi.h>

int memcmp(const void *aptr, const void *bptr, UINT64 n) {
	const UINT8 *a = aptr, *b = bptr;

	for (UINT64 i = 0; i < n; i++) {
		if (a[i] < b[i]) {
			return -1;
		} else if (a[i] > b[i]) {
			return 1;
		}
	}
	
	return 0;
}
