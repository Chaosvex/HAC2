#include "codefinder.h"
#include "misc.h"

PIMAGE_SECTION_HEADER GetSection(char *exe, char *name)
{
	PIMAGE_DOS_HEADER dosHeader;

	dosHeader = (PIMAGE_DOS_HEADER)exe;

	if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	IMAGE_NT_HEADERS * NtHeader = (IMAGE_NT_HEADERS*)((DWORD)exe+dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)( (DWORD)NtHeader + sizeof(IMAGE_NT_HEADERS));

	for(WORD i = 0; i < NtHeader->FileHeader.NumberOfSections; i++)
	{
		if(_stricmp((char*)Section->Name, name) == 0)
			return Section;

		Section++;
	}

	return 0;
}

DWORD XXFindCode(char *Exe, char *code, char*mask, size_t size, bool fastFind) {
    PIMAGE_SECTION_HEADER CodeSection = GetSection(Exe, ".text");

	//Couldn't find the text section
    if(!CodeSection)
		return -1;

	if(fastFind) {
		return signatureFind((BYTE*)code, size, (BYTE*)Exe+CodeSection->VirtualAddress, CodeSection->SizeOfRawData);
	} else {
		for(UINT i = 0; i < CodeSection->SizeOfRawData-size; i++) {
			size_t j = 0;
			while(code[j] == *(Exe+CodeSection->VirtualAddress+i+j) || (mask[j] == 0)) {
				if(++j == size) {
					return CodeSection->VirtualAddress+i+(DWORD)Exe;
				}
			}
		}
	}

	return -1;
}

bool ishexdigit(char c) {
	if(c >= '0' && c <= '9')
		return true;
	if(c >= 'A' && c <= 'F')
		return true;
	if(c >= 'a' && c <= 'f')
		return true;

	return false;
}

long GetByte(string& s) {
	if(s == "??")
		return -2;

	if(ishexdigit(s[0]) && ishexdigit(s[1])) {
		long ret;
		sscanf_s(s.c_str(), "%02X", &ret);
		return ret;
	}

	return -1;
}

/*
 * Strips spaces from the string
 */
void NoSpaces(string &s) {
	size_t name_pos = 0;
	while((name_pos = s.find(" ", name_pos)) != string::npos)
		s.replace(name_pos, 1, "");
}

DWORD FindCode(char *Exe, string& code) {
	bool fastFind = true;
	NoSpaces(code); //strip spaces from string

	/*A byte is two chars, so if the string contains an odd number of chars
	  then we know it must be invalid*/ 
	if(!code.length() || (code.length() % 2) != 0)
		return -1;

	size_t length = code.length() / 2; //num of bytes represented by string
	char *code_data = new char[length];
	char *mask_data = new char[length];

	/*Sets all bytes to 01. If a wildcard (??) should be used in the match
	  then the byte is set to 00 later on*/
	memset(mask_data, 1, length);

	string::iterator i = code.begin();
	
	//Loop over every second character in the string (each byte)
	for(size_t pos = 0; i < code.end(); i += 2, pos++) {
		long c = GetByte(string(i, i+2));
		if(c == -1) {
			delete[] code_data;
			delete[] mask_data;
			return -1; //non-hex data found (<00 || >FF)
		}

		if(c == -2) {//wildcard (??)
			mask_data[pos] = 0;
			fastFind = false;
		} else {
			code_data[pos] = (char)c;
		}
	}

	PIMAGE_DOS_HEADER dosHeader;

	dosHeader = (PIMAGE_DOS_HEADER)Exe;

	//Returns if not a valid PE image
	if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		delete[] code_data;
		delete[] mask_data;
		return NULL;
	}

	//Actual signature scan
    DWORD ret = XXFindCode(Exe, code_data, mask_data, length, fastFind);

	delete[] code_data;
    delete[] mask_data;

	//Signature not found
	if(ret == -1)
		return NULL;

	//Found, return base + offset
	return ret;
}