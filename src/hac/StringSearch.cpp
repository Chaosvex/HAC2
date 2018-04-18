#include "StringSearch.h"

DWORD signatureFind(BYTE* signature, size_t sigLength, BYTE* memory, size_t memLength) {
	size_t bad_char_skip[256];
	size_t last = sigLength - 1;
	for(size_t i = 0; i <= 255; i++) {
		bad_char_skip[i] = sigLength;
	}

	for(size_t i = 0; i < last; i++) {
		bad_char_skip[signature[i]] = last - i;
	}

	while(memLength >= sigLength) {
		for(int i = last; signature[i] == memory[i]; i--) {
			if(i == 0) {
				return (DWORD)memory;
			}
		}

		memLength -= bad_char_skip[memory[last]];
		memory += bad_char_skip[memory[last]];
	}

	return -1;
}