#include "FontHandler.h"
#include "encfont.h"
#include "console.h"
#include <cstdint>
#include <Windows.h>
#include <wincrypt.h>
#include "DebugHelper.h"
namespace FontHandler {

HANDLE hFont = NULL;
BYTE pkey[] = {0x08, 0x02, 0x00, 0x00, 0x01, 0x68, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x50, 0x46, 0xBC, 0x68, 0x08};
std::uint8_t efr[] = {0xB2, 0xFA, 0x25, 0x21, 0x79, 0x57, 0x6D, 0x29, 0x4A, 0x44, 0x89, 0x2F, 0xC0, 0xD0, 0x2B, 0xD9, 0xD8, 0x78, 0x62};
std::uint8_t efk[] = {0xE6, 0xAC, 0x6B, 0x4E, 0x0B, 0x33, 0x28, 0x6F, 0x67, 0x16, 0xEC, 0x48, 0xB5, 0xBC, 0x4A, 0xAB, 0x9B, 0x17, 0x0C};
char* buffer;

DWORD decrypt(BYTE** data, DWORD& size) {
	HCRYPTPROV hProv;
	HCRYPTKEY key;
	DWORD error = 0;

	if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0)) {
		if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
			return GetLastError();
		}
	}

	if(!CryptImportKey(hProv, pkey, sizeof(pkey), 0, 0, &key)) {
		return GetLastError();
	}

	*data = new BYTE[sizeof(font)];
	memcpy(*data, font, sizeof(font));
	size = sizeof(font);

	if(!CryptDecrypt(key, 0, TRUE, 0, *data, &size)) {
		error = GetLastError();
	}

	CryptDestroyKey(key);
	CryptReleaseContext(hProv, 0);
		
	if(error) {
		delete[] *data;
		return error;
	}

	return 0;
}

DWORD loadFont(char** name) {
	BYTE* data;
	DWORD size;
	DWORD error = decrypt(&data, size);
	
	if(error != 0) {
		return error;
	}

	buffer = static_cast<char*>(calloc(1, 30));

	for(int i = 0; i < sizeof(efk); i++) {
		buffer[i] = efr[i] ^ efk[i];
	}

	*name = buffer;

	DWORD loaded;
	hFont = AddFontMemResourceEx(data, size, 0, &loaded);

	SecureZeroMemory(data, size);
	delete[] data;

	return !hFont? GetLastError() : 0;
}

void releaseFont() {
	SecureZeroMemory(buffer, 30);
	free(buffer);
}

};