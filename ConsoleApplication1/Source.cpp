#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <windows.h>
#include <fstream>
#include <io.h>
#include <TCHAR.H>

using namespace std;
DWORD FileSize = 0, FileIter=0, FileIterWr=0;
int num = 0;
int ibs = 512;
int obs = 2048;
int _SKIP = 0;
int _SEEK = 0;
int _COUNT = 0;
bool _ansi = false;
int cbs = 512;
bool _notrunc = false;
bool _sync = false; int _syncCount = 0; int _writeTempCount = 0;
bool _noerror = false;
bool _lcase = false;
bool _ucase = false;
bool _block = false;
bool _unblock = false;
bool _swabb = false;
const int N = 256;
char *FName = "help";
DWORD dwCreation = CREATE_NEW;
DWORD dwCreationAccess = GENERIC_WRITE;


LPWSTR CharToLPWSTR(LPCSTR char_string)
{
	LPWSTR res;
	DWORD res_len = MultiByteToWideChar(1251, 0, char_string, -1, NULL, 0);
	res = (LPWSTR)GlobalAlloc(GPTR, (res_len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(1251, 0, char_string, -1, res, res_len);
	return res;
}
void ReadHelp()
{
	char S[N] = { "" }; //В S будут считываться строки
	ifstream in1(FName); //Открыли файл для чтения
	if (!in1.is_open()) {// если файл не открыт
		cout << "File will not open!\n"; // сообщить об этом
		return;
	}
	while (!in1.eof()) //Будем читать информацию пока не дойдем до конца файла
	{
		in1.getline(S, N); //Построчное считывание информации в S

		cout << S << endl; //Вывод очередной строки на экран
	}
	in1.close();  //Закрыли открытый файл
	system("pause");
}
void Helper()
{
	cout << "Example: mv [options...] source_file file_destanation\n" <<
		"mv[options...] source_file... catalog" << endl;
	system("Pause");
}
void Help()
{
	ReadHelp();
}
bool FileExists(LPCTSTR fname)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(fname, &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		// Если этого не сделать то произойдет утечка ресурсов
		::FindClose(hFind);
		return true;
	}
	return false;
}
char up(char a) {

	if (iswlower(a))
		a = towupper(a);
	else return a;
	return a;
}
char low(char a) {

	if (iswupper(a))
		a = towlower(a);
	else return a;
	return a;
}
wstring AnsiToWide(const string& in_sAnsi)
{
	wstring wsWide;
	wsWide.resize(in_sAnsi.length(), 0);
	MultiByteToWideChar(
		CP_ACP,
		0,
		&in_sAnsi[0],
		(int)in_sAnsi.length(),
		&wsWide[0],
		(int)wsWide.length());

	return wsWide;
}

void FormingFinalMas(char **mass1, char **mass2) {
	int tm = 0, z = 0;
	for (int i = 0; i <= FileIter; i++) {
		for (int j = 0; j < ibs; j++) {
			if (tm < obs) {
				if (_lcase)
					mass1[z][tm] = up(mass2[i][j]);
				else if (_ucase)
					mass1[z][tm] = low(mass2[i][j]);
				else mass1[z][tm] = mass2[i][j];
				tm++;
			}
			else {
				if (z == FileIterWr) break;
				z++; tm = 0;
				if (_lcase)
					mass1[z][tm] = up(mass2[i][j]);
				else if (_ucase)
					mass1[z][tm] = low(mass2[i][j]);
				else mass1[z][tm] = mass2[i][j];
				tm ++;
			}
			if (j == ibs - 1 && _block)
				if (mass1[z][tm] == '\n')
					mass1[z][tm] = ' ';
			if (j == ibs - 1 && _unblock)
				if (mass1[z][tm] == ' ')
					mass1[z][tm] = '\n';
			j == ibs - 1 ? mass1[z][tm] = 0 : false;
		}
		if (i == FileIter) mass1[z][tm]= 0;
	}
	
}
void FormingFinalSwabMas(char **masWr, char **mass) {
	int tm = 0, z = 0;
	for (int i = 0; i <= FileIter; i++) {
		for (int j = 0; j < ibs; j++) {
			if (tm <= obs) {
				if (j == ibs - 1) {
					if (_lcase)
						masWr[z][tm] = up(mass[i][j]);
					else if (_ucase)
						masWr[z][tm] = low(mass[i][j]);
					else masWr[z][tm] = mass[i][j];
				}
				else {
					if (_lcase)
						masWr[z][tm] = up(mass[i][j + 1]);
					else if (_ucase)
						masWr[z][tm] = low(mass[i][j + 1]);
					else masWr[z][tm] = mass[i][j + 1];
				}
				tm++;
				if (_lcase)
					masWr[z][tm] = up(mass[i][j]);
				else if (_ucase)
					masWr[z][tm] = low(mass[i][j]);
				else masWr[z][tm] = mass[i][j];
				tm++; j++;
			}
			else {
				z++; tm = 0;
				if (_lcase)
					masWr[z][tm] = up(mass[i][j]);
				else if (_ucase)
					masWr[z][tm] = low(mass[i][j]);
				else masWr[z][tm] = mass[i][j];
				tm++;
			}
			if (j == ibs - 1 && _block)
				if (masWr[z][tm] == '\n')
					masWr[z][tm] = ' ';
			if (j == ibs - 1 && _unblock)
				if (masWr[z][tm] == ' ')
					masWr[z][tm] = '\n';
		}
	}
}
void Writer(char **mass, LPCTSTR fileDest, DWORD Filesize, DWORD FileIter) {
	DWORD  dwTemp, nOBS = obs; int temp; bool tmMnObs = false;
	// подготавливаем поля структуры асинхронной операции
	//OVERLAPPED gOverlapped;
	//gOverlapped.Offset = _SEEK;
	//gOverlapped.OffsetHigh = 0;
	//gOverlapped.hEvent = 0;
	if (FileExists(fileDest))
		DeleteFile(fileDest);
	//HANDLE hFile = CreateFile(fileDest, dwCreationAccess, 0, NULL,
	//	dwCreation, FILE_FLAG_OVERLAPPED, NULL);
	HANDLE hFile = CreateFile(fileDest, dwCreationAccess, 0, NULL,
		dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		return;
	}
	(Filesize / obs) > 0 ? FileIterWr = (Filesize / obs)+1: FileIterWr = 1;
	_SEEK > FileIterWr ? FileIterWr = 1 : FileIterWr -= _SEEK;
	char** masWr = new char*[FileIterWr+1];
	for (int i = 0; i <= FileIterWr; i++) masWr[i] = new char[obs];

	!_swabb? FormingFinalMas(masWr, mass): FormingFinalSwabMas(masWr, mass);
	DWORD nnOBS;
	nnOBS = 0;
	for (DWORD i = 0; i < obs; i++) {
		if (FileIterWr > 0)
		if (masWr[FileIterWr-1][i] == 0 || masWr[FileIterWr-1][i] == '\0') {
			nnOBS = i;
			break;
		}
		else if (masWr[FileIterWr][i] == 0 || masWr[FileIterWr][i] == '\0') {
			nnOBS = i;
			break;
		}
	}

	for (int i = _SEEK; i < FileIterWr; i++) {
		if ((i == FileIterWr-1)|| (i == FileIterWr)) {
			if (_ansi) {
				wstring wstr = AnsiToWide(masWr[i]);
				WriteFile(hFile, (LPCVOID)&wstr[0], nnOBS, &dwTemp, NULL);
			}
			if (nnOBS == 0) break;
			WriteFile(hFile, masWr[i], nnOBS, &dwTemp, NULL);
		}
		else {
			//WriteFile(hFile, masWr[i], nOBS, &dwTemp, &gOverlapped);
			//gOverlapped.Offset += nOBS;
			//gOverlapped.OffsetHigh = 0;
			WriteFile(hFile, masWr[i], nOBS, &dwTemp, NULL);
			int count = 0;
		}
	}
	CloseHandle(hFile);
}
void Forming(HANDLE hFile) {
	bool countFalse = false;
	FileSize = GetFileSize(hFile, NULL);
	if ((FileSize / ibs) > 0)
		FileIter = (FileSize / ibs);
	else FileIter = 1;
	(_SKIP > FileIter)? FileIter = 1 : FileIter -= _SKIP;
	(_COUNT > FileIter) ? FileIter = 1 : countFalse = true;
	if (countFalse) _COUNT == 0 ? FileIter : FileIter = _COUNT;
	_syncCount = ibs - (FileSize % ibs);
}

void ReadFile(LPCTSTR file, LPCTSTR fileDest) {
	OVERLAPPED gOverlapped;
	DWORD  dwTemp, nIBS = ibs, oldFileIter;
	// подготавливаем поля структуры асинхронной операции
	gOverlapped.Offset = _SKIP*ibs;
	gOverlapped.OffsetHigh = 0;
	gOverlapped.hEvent = 0;
	int i, str; int temp;

	HANDLE fin = CreateFile(file, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//
	//Forming
	Forming(fin);
	//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//
	char** mass = new char*[FileIter];
	for (int i = 0; i <= FileIter; i++) mass[i] = new char[ibs];
	//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//
	//SYNC
	if (_sync)
		for (int i = _syncCount; i < ibs; i++) mass[FileIter][i] = 0;
	else if (_notrunc) for (int i = _syncCount; i < ibs; i++) mass[FileIter][i] = '\0';
	else for (int i = _syncCount; i < ibs; i++) mass[FileIter][i] = '\0';
	//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//-//
	for (int i = 0; i <= FileIter; i++) {
			try {
				ReadFile(fin, mass[i], ibs, &dwTemp, &gOverlapped);
			}
			catch (exception e) {
				if (_noerror)
					i++;
			}
			gOverlapped.Offset += ibs;
			gOverlapped.OffsetHigh = 0;
		}

	FileIter==0? Writer(mass, fileDest, ibs, FileIter): Writer(mass, fileDest, FileIter*ibs, FileIter);

	CloseHandle(fin);
}

void StartValue(char *value, int nextValue) {
	if (strcmp(value, "-ibs") == 0)
		ibs = nextValue;
	if (strcmp(value, "-obs") == 0)
		obs = nextValue;
	if (strcmp(value, "-bs") == 0) { ibs = nextValue; obs = nextValue; }
	if (strcmp(value, "-cbs") == 0) { cbs = nextValue; }
	if (strcmp(value, "-seek") == 0) _SEEK = nextValue;
	if (strcmp(value, "-skip") == 0) _SKIP = nextValue;
	if (strcmp(value, "-count") == 0) _COUNT = nextValue;
}
void Conv(int zap, char **conv) {
	for (int i = 0; i <= zap; i++) {
		if (strcmp(conv[i], "lcase") == 0)
			_lcase = true;
		if (strcmp(conv[i], "ucase") == 0)
			_ucase = true;
		if (strcmp(conv[i], "swab") == 0)
			_swabb = true;
		if (strcmp(conv[i], "ascii") == 0)
			_ansi = true;
		if (strcmp(conv[i], "block") == 0)
			_block = true;
		if (strcmp(conv[i], "unblock") == 0)
			_unblock = true;
		if (strcmp(conv[i], "noerror") == 0)
			_noerror = true;
		if (strcmp(conv[i], "notrunc") == 0)
			_notrunc = true;
		if (strcmp(conv[i], "sync") == 0)
			_sync = true;

	}
}
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Russian");

	if (argc > 1) {

		if (strcmp(argv[1], "--help") == 0) {
			Help();
		}
		for (int i = 1; i < argc - 2; i++) {
			int valueI = 0, valueZ = 0, nextValueI = 0, nextValueZ = 0; int zap = 0; int colZap[10]; int m = 0;
			char *value, *nextValue; bool go = false; bool goNext = false;
			const char *oldname = argv[i];
			for (int j = 0; j < strlen(oldname); j++) {
				if (argv[i][j] == '=') break;
				else valueI++;
			}
			for (int j = 0; j < strlen(oldname); j++) {
				if (argv[i][j] == '=') goNext = true;
				if (goNext) nextValueI++;
			}
			goNext = false;
			value = new char[valueI];
			nextValue = new char[nextValueI];
			for (int j = 0; j < strlen(oldname); j++) {
				if (argv[i][j] == '=') break;
				else { value[valueZ] = argv[i][j]; valueZ++; }
			}
			for (int j = valueZ; j < strlen(oldname); j++) {
				if (argv[i][j] == '=')
					go = true;
				else if (go) { nextValue[nextValueZ] = argv[i][j]; nextValueZ++; }
			}
			go = false;
			value[valueZ] = '\0'; nextValue[nextValueZ] = '\0';
			if (strcmp(value, "-conv") == 0) {
				for (int j = 0; j < 10; j++) colZap[j] = 0;
				for (int j = 0; j < nextValueZ; j++) {
					if (nextValue[j] == ',') zap++;
					colZap[zap]++;
				}
				char ** conv = new char*[zap + 1];
				for (int j = 0; j <= zap; j++) conv[j] = new char[colZap[j]];
				zap = 0;
				for (int j = 0; j < nextValueZ; j++) {
					if (nextValue[j] == ',') {
						conv[zap][m] = '\0'; zap++; m = 0; continue;
					}
					conv[zap][m] = nextValue[j];
					m++;
				}
				conv[zap][m] = '\0';
				Conv(zap, conv);
			}
			else StartValue(value, atoi(nextValue));
			value = NULL; nextValue = NULL;
		}
	}
	ReadFile(CharToLPWSTR(argv[argc - 2]), CharToLPWSTR(argv[argc - 1]));
	
	if (argc == 1) {
		Helper();
		//choose('-v', "E:\\help.txt", "E:\\help1.txt"); //debug
	}
	return 0;

}