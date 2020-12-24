//VFS.h

// Super Block	(��� ��������� ���������� ��� ������ �������� �������)
// 2^n inodes	(���������� ������������ ������ � super block'�)
// {
// n-size block [ 0 - cur | 1 - next | 2 - type | 3 - ptrBlock | 4 - 43 - name ]	(������ ����� ����� � super block'e)
// ...
// (n-size) block	(���������� ������ ������ �� ���������� ��������� ������������)
// }

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define QUANTITY_OF_INODES 128
///
#define DIRECTORY_TYPE 1
#define FILE_TYPE 2
// ����� ���� ��� ������, ����� � �.�.
///
#define SIZE_OF_BLOCK 128
#define MAX_NAME_SIZE 40

using namespace std;

typedef unsigned int uint;
typedef unsigned int fpointer;
typedef unsigned char Inode;

struct SuperBlock
{
	string sPath;
	uint fullMemory = 0;
	uint occupiedMemory = 0;
	uint freeINodes = 0;
	Inode inodes[QUANTITY_OF_INODES] = {};
};

class VFS
{
private:
	fstream file;
	SuperBlock superBlock;
	Inode curBlock;
	vector<string> path;
	vector<Inode> iPath;
	bool createFS();
	Inode findFreeINode();
	bool addBranch(string);
	bool delBranch();
public:
	VFS(string);
	~VFS();
	string openFile();
	bool saveFile(string);
	bool createDir(string);
	bool createFile(string);
	bool renameDir(string, string);
	bool renameFile(string, string);
	bool deleteDir(string);
	bool deleteFile(string);
	bool back();
	bool enterDir(string);
	bool enterFile(string);
	string getCurPath();
	uint getAllMem();
	uint getOccMem();
	vector<string> getDirectories();
	vector<string> getFiles();
};