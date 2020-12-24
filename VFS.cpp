// VFS.cpp

#include "VFS.h"

VFS::VFS(string fileEmulator)
{
	Inode nodes[QUANTITY_OF_INODES] = {};
	file.open(fileEmulator, ios_base::in | ios_base::out | ios_base::binary);
	if (!file.is_open())
	{
		cout << "Error open file!" << endl;
		return;
	}
	file.read((char*)nodes, QUANTITY_OF_INODES);
	if (file.eof())
	{
		VFS::createFS();
		superBlock.inodes[0] = 1;
		superBlock.freeINodes = QUANTITY_OF_INODES - 1;
		superBlock.occupiedMemory = SIZE_OF_BLOCK;
	}
	else
	{
		for (int i = 0; i < QUANTITY_OF_INODES; i++)
		{
			if (!nodes[i])	// Если индексный дескриптор свободен
				superBlock.freeINodes++;	// Увеличиваем количество свободных дескрипторов
			superBlock.inodes[i] = nodes[i];
		}
		superBlock.occupiedMemory = (QUANTITY_OF_INODES - superBlock.freeINodes) * SIZE_OF_BLOCK;
	}
	superBlock.fullMemory = QUANTITY_OF_INODES * SIZE_OF_BLOCK;
	curBlock = 0;
	superBlock.sPath = fileEmulator;
}

VFS::~VFS()
{
	file.close();
}

bool VFS::createFS()	// Создаём файловую систему
{
	Inode descs[QUANTITY_OF_INODES] = {};
	char emptySymb[QUANTITY_OF_INODES] = {};
	descs[0] = 1;
	file.clear();
	file.seekg(0, ios::beg);
	file.write((char*)descs, QUANTITY_OF_INODES);
	for (int i = 0; i < QUANTITY_OF_INODES; i++)
		file.write(emptySymb, SIZE_OF_BLOCK);
	return true;
}

Inode VFS::findFreeINode()
{
	Inode node = 0;
	if (superBlock.freeINodes != 0)
		for (int i = 1; i < QUANTITY_OF_INODES; i++)
			if (superBlock.inodes[i] == 0)
			{
				superBlock.inodes[i] = 1;
				superBlock.freeINodes--;
				superBlock.occupiedMemory += SIZE_OF_BLOCK;
				node = i;
				break;
			}
	return node;
}

string VFS::openFile()
{
	string text;
	char charText[SIZE_OF_BLOCK - 3] = {};
	Inode isNext = 0;
	Inode isFile = 0;
	Inode cur = curBlock + 1;
	do
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		file.read((char*)&isFile, 1);
		if (isFile == NULL)
			return text;
		file.read(charText, isFile);
		text += charText;
		for (int i = 0; i < SIZE_OF_BLOCK - 3; i++)
			charText[i] = 0;
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&isNext, 1);	// Есть ли у нас еще блоки
		cur = isNext;	// Присваиваем cur значение следующего блока
	} while (isNext != 0);
	return text;
}

bool VFS::saveFile(string text)
{
	char charText[SIZE_OF_BLOCK - 3] = {};
	Inode isNext = 0;
	Inode isFile = 0;
	Inode cur = curBlock + 1;

	file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
	char countByte = text.length();
	file.write(&countByte, 1);
	for (int i = 0; i < text.size() && i < SIZE_OF_BLOCK; i++)
		charText[i] = text[i];
	file.write(charText, SIZE_OF_BLOCK - 3);
	return true;
}

bool VFS::createDir(string dirName)
{
	Inode node;
	node = VFS::findFreeINode();	// Ищем свободный блок для директории
	if (node == 0)
	{
		cout << "There is not memory enough!" << endl;
		system("pause");
		return false;
	}
	char name[MAX_NAME_SIZE] = {};	// Помещаем сюда название директории
	for (int i = 0; i < dirName.size(); i++)
		name[i] = dirName[i];
	Inode next = curBlock + 1;
	while (true)	// Если в текущей директории уже есть несколько блоков, то движемся на последний
	{
		file.seekg(next * SIZE_OF_BLOCK, ios::beg);
		file.seekg(1, ios::cur);
		char isNext = 0;
		file.read(&isNext, 1);
		if (isNext == 0)
			break;
		else
			next = isNext;
	}
	int index;
	for (index = 0; index < 3; index++)	// Проверяем достаточно ли места в текущем блоке
	{
		char isType = 0;
		file.read(&isType, 1);
		if (isType == 0)
		{
			index++;
			file.seekg(-1, ios::cur);	// Нашли нужную позицию и становимся на байт с типом
			break;
		}
		if (index + 1 != 3)
			file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		else
			index = 3;
	}
	if (index == 4)	// Недостаточно места в текущем блоке, нужно выделить место под новый блок
	{
		Inode addNode;
		addNode = VFS::findFreeINode();
		if (addNode == 0)
		{
			cout << "There is not memory enough!" << endl;
			system("pause");
			superBlock.inodes[node] = 0;
			return false;
		}
		addNode++;
		file.seekg(-((MAX_NAME_SIZE + 2) * 2) - 2, ios::cur);	// Перемещаемся обратно в позицию, где указывается следующий блок, и заполняем её
		file.write((char*)&addNode, 1);
		next = addNode;
		file.seekg(next * SIZE_OF_BLOCK, ios::beg);	// Перемещаемся на новый блок
		file.seekg(2, ios::cur);
		index = 1;
	}
	char type = DIRECTORY_TYPE;
	file.write(&type, 1);
	file.write((char*)&node, 1);
	file.write(name, MAX_NAME_SIZE);
	file.seekg(0, ios::beg);	// Обновляем значения индексных дескрипторов в файле
	file.write((const char*)superBlock.inodes, QUANTITY_OF_INODES);
	return true;
}

bool VFS::createFile(string fileName)
{
	Inode node;
	node = VFS::findFreeINode();	// Ищем свободный блок для файла
	if (node == 0)
	{
		cout << "There is not memory enough!" << endl;
		system("pause");
		return false;
	}
	char name[MAX_NAME_SIZE] = {};	// Помещаем сюда название файла
	for (int i = 0; i < fileName.size(); i++)
		name[i] = fileName[i];
	Inode next = curBlock + 1;
	while (true)	// Если в текущей директории уже есть несколько блоков, то движемся на последний
	{
		file.seekg(next * SIZE_OF_BLOCK, ios::beg);
		file.seekg(1, ios::cur);
		char isNext = 0;
		file.read(&isNext, 1);
		if (isNext == 0)
			break;
		else
			next = isNext;
	}
	int index;
	for (index = 0; index < 3; index++)	// Проверяем достаточно ли места в текущем блоке
	{
		char isType = 0;
		file.read(&isType, 1);
		if (isType == 0)
		{
			index++;
			file.seekg(-1, ios::cur);	// Нашли нужную позицию и становимся на байт с типом
			break;
		}
		if (index + 1 != 3)
			file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		else
			index = 3;
	}
	if (index == 4)	// Недостаточно места в текущем блоке, нужно выделить место под новый блок
	{
		Inode addNode;
		addNode = VFS::findFreeINode();
		if (addNode == 0)
		{
			cout << "There is not memory enough!" << endl;
			system("pause");
			superBlock.inodes[node] = 0;
			return false;
		}
		addNode++;
		file.seekg(-((MAX_NAME_SIZE + 2) * 2) - 2, ios::cur);	// Перемещаемся обратно в позицию, где указывается следующий блок, и заполняем её
		file.write((char*)&addNode, 1);
		next = addNode;
		file.seekg(next * SIZE_OF_BLOCK, ios::beg);	// Перемещаемся на новый блок
		file.seekg(2, ios::cur);
		index = 1;
	}
	char type = FILE_TYPE;
	file.write(&type, 1);
	file.write((char*)&node, 1);
	file.write(name, MAX_NAME_SIZE);
	file.seekg(0, ios::beg);	// Обновляем значения индексных дескрипторов в файле
	file.write((const char*)superBlock.inodes, QUANTITY_OF_INODES);
	return true;
}

bool VFS::renameDir(string oldDirName, string newDirName)
{
	bool flag = true;
	Inode isDir = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isDir, 1);
			if (isDir == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isDir == DIRECTORY_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == oldDirName)	// Если это наша директория
				{
					file.seekg(-(MAX_NAME_SIZE), ios::cur);	// Попадаем в позицию с началом имени директории
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
					for (int i = 0; i < newDirName.size(); i++)	// Записываем новое название директории
						name[i] = newDirName[i];
					file.write(name, MAX_NAME_SIZE);	// Переименовываем директорию
					flag = false;
					break;
				}
				else    // Если это не наша директория
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

bool VFS::renameFile(string oldFileName, string newFileName)
{
	bool flag = true;
	Inode isFile = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isFile, 1);
			if (isFile == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isFile == FILE_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == oldFileName)	// Если это наш файл
				{
					file.seekg(-(MAX_NAME_SIZE), ios::cur);	// Попадаем в позицию с началом имени файла
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
					for (int i = 0; i < newFileName.size(); i++)	// Записываем новое название файла
						name[i] = newFileName[i];
					file.write(name, MAX_NAME_SIZE);	// Переименовываем файл
					flag = false;
					break;
				}
				else    // Если это не наш файл
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

bool VFS::deleteDir(string dirName)
{
	bool flag = true;
	Inode isDir = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isDir, 1);
			if (isDir == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isDir == DIRECTORY_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == dirName)	// Если это наша директория
				{
					file.seekg(-(MAX_NAME_SIZE + 2), ios::cur);	// Попадаем в позицию с информацией о текущем компоненте
					char emptySymb = NULL;
					for (int i = 0; i < 2 + MAX_NAME_SIZE; i++)
						file.write(&emptySymb, 1);
					flag = false;
					break;
				}
				else    // Если это не наша директория
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

bool VFS::deleteFile(string fileName)
{
	bool flag = true;
	Inode isFile = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isFile, 1);
			if (isFile == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isFile == FILE_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == fileName)	// Если это наш файл
				{
					file.seekg(-(MAX_NAME_SIZE + 2), ios::cur);	// Попадаем в позицию с информацией о текущем компоненте
					char emptySymb = NULL;
					for (int i = 0; i < 2 + MAX_NAME_SIZE; i++)
						file.write(&emptySymb, 1);
					flag = false;
					break;
				}
				else    // Если это не наш файл
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

bool VFS::addBranch(string name)
{
	path.push_back(name);
	return true;
}

bool VFS::delBranch()
{
	path.pop_back();
	return true;
}

bool VFS::back()
{
	VFS::delBranch();	// Убираем компонент из пути
	curBlock = iPath[iPath.size() - 1];
	iPath.pop_back();
	return true;
}

bool VFS::enterDir(string dirName)
{
	VFS::addBranch('/' + dirName);	// Добавляем компонент в путь
	bool flag = true;
	Inode isFDir = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isFDir, 1);
			if (isFDir == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isFDir == DIRECTORY_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == dirName)	// Если это наша директория
				{
					file.seekg(-(MAX_NAME_SIZE + 1), ios::cur);	// Попадаем в позицию с информацией о блоке текущего компонента
					Inode newBlock;
					file.read((char*)&newBlock, 1);
					iPath.push_back(curBlock);
					curBlock = newBlock;
					flag = false;
					break;
				}
				else    // Если это не наша директория
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

bool VFS::enterFile(string fileName)
{
	VFS::addBranch('/' + fileName);	// Добавляем компонент в путь
	bool flag = true;
	Inode isFile = 0;
	Inode cur = curBlock + 1;
	string tmpStr;
	char name[MAX_NAME_SIZE + 1] = {};
	while (flag)
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int i = 0; i < 3; i++)
		{
			file.read((char*)&isFile, 1);
			if (isFile == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isFile == FILE_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				tmpStr = name;
				if (tmpStr == fileName)	// Если это наш файл
				{
					file.seekg(-(MAX_NAME_SIZE + 1), ios::cur);	// Попадаем в позицию с информацией о блоке текущего компонента
					Inode newBlock;
					file.read((char*)&newBlock, 1);
					iPath.push_back(curBlock);
					curBlock = newBlock;
					flag = false;
					break;
				}
				else    // Если это не наш файл
					for (int i = 0; i < MAX_NAME_SIZE + 1; i++)	// Обнуляем массив с именем
						name[i] = 0;
			}
			else
				if (i != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&cur, 1);	// Есть ли у нас еще блоки
	}
	return true;
}

string VFS::getCurPath()
{
	string curPath = "/home";
	for (int i = 0; i < path.size(); i++)
		curPath += path[i];
	return curPath;
}

uint VFS::getAllMem()
{
	return superBlock.fullMemory;
}

uint VFS::getOccMem()
{
	return superBlock.occupiedMemory;
}

vector<string> VFS::getDirectories()
{
	Inode isNext = 0;
	Inode isDir = 0;
	Inode cur = curBlock + 1;
	char name[MAX_NAME_SIZE + 1] = {};
	int index = 0;
	vector<string> vecDir;
	do
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int index = 0; index < 3; index++)
		{
			file.read((char*)&isDir, 1);
			if (isDir == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isDir == DIRECTORY_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				vecDir.push_back(name);
			}
			else
				if (index != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&isNext, 1);	// Есть ли у нас еще блоки
		cur = isNext;	// Присваиваем cur значение следующего блока
	} while (isNext != 0);
	return vecDir;
}

vector<string> VFS::getFiles()
{
	Inode isNext = 0;
	Inode isFile = 0;
	Inode cur = curBlock + 1;
	char name[MAX_NAME_SIZE + 1] = {};
	int index = 0;
	vector<string> vecFile;
	do
	{
		file.seekg(cur * SIZE_OF_BLOCK + 2, ios::beg);
		for (int index = 0; index < 3; index++)
		{
			file.read((char*)&isFile, 1);
			if (isFile == NULL)
			{
				file.seekg(MAX_NAME_SIZE + 1, ios::cur);
				continue;
			}
			if (isFile == FILE_TYPE)
			{
				file.seekg(1, ios::cur);
				file.read(name, MAX_NAME_SIZE);
				vecFile.push_back(name);
			}
			else
				if (index != 2)
					file.seekg(MAX_NAME_SIZE + 1, ios::cur);
		}
		file.seekg(cur * SIZE_OF_BLOCK + 1, ios::beg);	// Встаём на ячейку со следующим блоком
		file.read((char*)&isNext, 1);	// Есть ли у нас еще блоки
		cur = isNext;	// Присваиваем cur значение следующего блока
	} while (isNext != 0);
	return vecFile;
}