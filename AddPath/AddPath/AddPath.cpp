#include <Windows.h>
#include <memory>
#include <iostream>
#include <string>
#include <tchar.h>
using namespace std;
#ifdef _UNICODE
#define tcout wcout
#define tstring wstring
#else
#define tcout cout
#define tstring string
#endif

#define HKLM HKEY_LOCAL_MACHINE
#define SUBKEY _T("System\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define VALUE _T("Path")

int main(int argc, TCHAR* argv[])
{
	tcout << _T("This program add a customed path into the environment variable \"Path\"") << endl;
	if (argc < 2)
	{
		tcout << _T("Usage: ") << argv[0] << _T(" added_path") << endl;
		return 0;
	}
	DWORD type = 0;
	DWORD len = 0;
	//get len
	auto ret = RegGetValue(HKLM, SUBKEY, VALUE, RRF_RT_REG_SZ|RRF_RT_REG_EXPAND_SZ|RRF_NOEXPAND, &type, nullptr, &len);
	if (ret != ERROR_SUCCESS)
	{
		cout << "RegGetValueW() Fail; GetLastError() = " << GetLastError() << endl;
		return 0;
	}
	tcout << _T("Original Path len = ") << len << endl;
	
	std::unique_ptr<BYTE[]> buf(new BYTE [len + 2]);
	buf[len] = 0;
	buf[len+1] = 0;
	// get "Path"
	ret = RegGetValue(HKLM, SUBKEY, VALUE, RRF_RT_REG_SZ|RRF_RT_REG_EXPAND_SZ|RRF_NOEXPAND, &type, buf.get(), &len);
	if (ret != ERROR_SUCCESS)
	{
		tcout << _T("RegGetValueW() Fail; GetLastError() = ") << GetLastError() << endl;
		return 0;
	}

	HKEY result;
	ret = RegCreateKeyEx(HKLM, SUBKEY, 0, nullptr, 0, KEY_WRITE, nullptr, &result, nullptr);
	if (ret != ERROR_SUCCESS)
	{
		tcout << _T("RegCreateKeyEx() Fail; GetLastError() = ") << GetLastError() << endl;
		return 0;
	}

	tstring new_path = tstring(argv[1]) + _T(";") + (TCHAR*)buf.get();
	ret = RegSetValueEx(result, VALUE, 0, type, (BYTE*)new_path.c_str(), (DWORD)(sizeof(TCHAR)*(new_path.length()+1)) );
	if (ret != ERROR_SUCCESS)
	{
		tcout << _T("RegSetValueEx() Fail; GetLastError() = ") << GetLastError() << endl;
		return 0;
	}

	ret = RegCloseKey(result);
	if (ret != ERROR_SUCCESS)
	{
		tcout << _T("RegCloseKey() Fail; GetLastError() = ") << GetLastError() << endl;
		return 0;
	}

	BOOL bRet = SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, LPARAM(_T("Environment")));
	if (!bRet)
	{
		tcout << _T("SendNotifyMessage() Fail; GetLastError() = ") << GetLastError() << endl;
		return 0;
	}

	tcout << _T("Original Path = ") << (TCHAR*)(buf.get()) << endl << endl;
	tcout << _T("New Path = ") << new_path << endl << endl;
	system("pause");
	return 0;
}