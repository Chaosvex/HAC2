#include "AppSearch.h"
#include <Windows.h>

namespace AppSearch {

std::string RegistryQueryValue(HKEY hKey, const std::string& szName) {
    std::string value;
    DWORD type, size = 0;
 
	if (RegQueryValueEx(hKey, szName.c_str(), NULL, &type, NULL, &size) == ERROR_SUCCESS && size > 0) {
        value.resize(size);
		RegQueryValueEx(hKey, szName.c_str(), NULL, &type, (LPBYTE)&value[0], &size);
    }
 
    return value;
}
 
bool find(const std::string& program) {
    HKEY hKey;
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
							0, KEY_READ, &hKey);
    
    if(ret != ERROR_SUCCESS) {
        return false;
	}
 
    DWORD dwIndex = 0;
    DWORD cbName = 1024;
    TCHAR szSubKeyName[1024];
	bool found = false;

    while ((ret = RegEnumKeyEx(hKey, dwIndex, szSubKeyName, &cbName, NULL, NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS) {
        dwIndex++;
        cbName = 1024;
		
		if (ret != ERROR_SUCCESS) {
			continue;
		}

        HKEY hItem;
            
		if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hItem) != ERROR_SUCCESS) {
			continue;
		}
 
        std::string name = RegistryQueryValue(hItem, "DisplayName");
        
		RegCloseKey(hItem);
		
		if(name == program) {
			found = true;
			break;
		}
	}

	RegCloseKey(hKey);
	return found;
}

}