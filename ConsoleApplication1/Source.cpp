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
int num = 0;
int ibs = 512;
int obs = 2048;
int _SKIP = 0;
int _SEEK = 0;
int _COUNT = 0;
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
void Writer(char **mass, LPCTSTR fileDest, DWORD FileSize, DWORD FileIter, int tempIBS) {
	DWORD  dwTemp, FileIterWr, nOBS = obs; int temp; bool tmMnObs = false;
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
	if ((FileSize / obs) > 0) {
		temp = 0;
		temp = (FileSize%obs);
		FileIterWr = (FileSize / obs) + 1;
		if (_SEEK > FileIterWr - 1) return;
	}
	else {
		temp = FileSize%obs;
		FileIterWr = 1;
		tmMnObs = true;
	}
	
	char** masWr = new char*[FileIterWr];
	for (int i = 0; i < FileIterWr; i++) {
		if (i == FileIterWr - 1&&temp>0)
			masWr[i] = new char[temp];
		else masWr[i] = new char[obs];
	}
	int tm = 0,z =0;
	if (tempIBS == 0&&FileIter>1)
		FileIter -= 1;
	if (!_swabb) {
		for (int i = 0; i < FileIter; i++) {
			for (int j = 0; j < ibs; j++) {
				if (tm < obs) {
					masWr[z][tm] = mass[i][j];
					tm++;
				}
				else {
					z++;
					masWr[z][tm] = mass[i][j];
					tm = 0;
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
	char temmm ;
	if (_swabb) {
		for (int i = 0; i < FileIter; i++) {
			for (int j = 0; j < ibs; j++) {
				if (tm < obs) {
					if (j == ibs - 1)
						masWr[z][tm] = mass[i][j];
					else masWr[z][tm] = mass[i][j+1];
					tm++;
					masWr[z][tm] = mass[i][j];
					tm++; j++;
				}
				else {
					z++;
					masWr[z][tm] = mass[i][j];
					tm = 0;
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
	if(!tmMnObs)
		for (int j = 0; j < tempIBS; j++) {
			if (tm < obs) {
				masWr[z][tm] = mass[FileIter - 1][j];
				tm++;
			}
			else {
				z++;
				tm = 0;
				masWr[z][tm] = mass[FileIter - 1][j];

			}
		}
	masWr[z][tm] = '\0';
	DWORD nnOBS = obs;
	if (!_notrunc)
		for (int i = 0; i < obs; i++)
			if (masWr[FileIterWr - 1][i] == -51) {
				nnOBS = i;
				break;
			}
			
	for (int i = _SEEK; i < FileIterWr; i++) {
		if (i == FileIterWr - 1) {
			//gOverlapped.Offset -= _SEEK;
			//WriteFile(hFile, masWr[i], nOBS, &dwTemp, &gOverlapped);
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

void ReadFile(LPCTSTR file, LPCTSTR fileDest) {
	OVERLAPPED gOverlapped;
	DWORD  dwTemp, FileSize, FileIter, nIBS = ibs, oldFileIter;
	// подготавливаем поля структуры асинхронной операции
	gOverlapped.Offset = _SKIP*ibs;
	gOverlapped.OffsetHigh = 0;
	gOverlapped.hEvent = 0;
	int i, str; int temp;
	
	HANDLE fin = CreateFile(file, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	//HANDLE fin = CreateFile(file, GENERIC_READ, 0, NULL,
	//	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	FileSize = GetFileSize(fin, NULL); temp = 0;
	if ((FileSize / ibs) > 0) {
		FileIter = (FileSize / ibs)+1;
		if (!_notrunc && !_sync) {
			temp = 0;
			FileIter = (FileSize / ibs);
		}
		else if(!_sync&&_notrunc)
			temp = ibs;
		else if ((_sync && !_notrunc)|| (_sync && _notrunc)) {
			_syncCount = (FileSize%ibs);
			temp = ibs;
		}
	}
	else FileIter = 1;
	if (_SKIP > FileIter - 1) return;
	else FileIter -= _SKIP;
	if (_COUNT > FileIter - 1) return;
	else if(_COUNT!=0)
	{ oldFileIter = FileIter; FileIter = _COUNT;  }
	else  oldFileIter = FileIter;
	char** mass = new char*[FileIter];
	for (int i = 0; i < FileIter; i++) {
		if (i == (FileIter - 1) && i == oldFileIter - 1&&i>0)
			if(temp>0)
				mass[i] = new char[temp];
			else continue;
		else mass[i] = new char[ibs];
	}
	if(_sync)
		for (int i = _syncCount; i <= ibs; i++) {
			mass[FileIter - 1][i] = '\0';
		}
	for (int i = 0; i < FileIter; i++) {
		
		if (i == (FileIter - 1)&&i== oldFileIter-1&&temp>0){
			
			//gOverlapped.Offset -= nIBS;
			nIBS = temp;
			//gOverlapped.Offset = nIBS-ibs;
			try {
				ReadFile(fin, mass[i], nIBS, &dwTemp, &gOverlapped);
			}
			catch (exception e) {
				if (_noerror)
					i++;
			}
			//ReadFile(fin, mass[i], nIBS, &dwTemp, NULL);
			mass[i][temp] = '\0';
		}
		else {
			try {
				ReadFile(fin, mass[i], nIBS, &dwTemp, &gOverlapped);
			}
			catch (exception e) { 
				if(_noerror)
					i++; }
			//ReadFile(fin, mass[i], nIBS, &dwTemp, NULL);
			int count = 0;
			gOverlapped.Offset += nIBS;
			gOverlapped.OffsetHigh = 0;
		}

	}

	Writer(mass, fileDest, FileIter*ibs, FileIter, temp);
	
	CloseHandle(fin);
}

void StartValue(char *value, int nextValue) {
	if(strcmp(value,"-ibs")==0)
		ibs = nextValue;
	if (strcmp(value, "-obs") == 0)
		obs = nextValue;
	if (strcmp(value, "-bs") == 0){ ibs = nextValue; obs = nextValue; }
	if (strcmp(value, "-cbs") == 0) { cbs = nextValue; }
	if (strcmp(value, "-seek") == 0) _SEEK = nextValue;
	if (strcmp(value, "-skip") == 0) _SKIP = nextValue;
	if (strcmp(value, "-count") == 0) _COUNT = nextValue;
}
void Conv(int zap, char **conv) {
	for (int i = 0; i <= zap; i++) {
		/*if (strcmp(conv[i], "lcase"))
			continue;
		if (strcmp(conv[i], "ucase"))
			continue;*/
		if (strcmp(conv[i], "swab"))
			_swabb=true;
		if (strcmp(conv[i], "ascii"))
			continue;
		if (strcmp(conv[i], "block"))
			_block=true;
		if (strcmp(conv[i], "unblock"))
			_unblock=true;
		if (strcmp(conv[i], "noerror"))
			_noerror=true;
		if (strcmp(conv[i], "notrunc")==0)
			_notrunc = true;
		if (strcmp(conv[i], "sync")==0)
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
			if (strcmp(value, "-conv")==0) {
				for (int j = 0; j < 10; j++) colZap[j] = 0;
				for (int j = 0; j < nextValueZ; j++) {
					if (nextValue[j] == ',') zap++;
					colZap[zap]++;
				}
				char ** conv = new char*[zap+1];
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
	
}