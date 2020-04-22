// Main.cpp

#include "VFS.h"
#include <conio.h>

#define ARROW(x, y)\
			if (x == y)\
				(std::cout << "-> ");
#define FILE_EMULATOR "C:/Users/Intel/Desktop/VFS.VFS"

int main()
{
	VFS fileSystem(FILE_EMULATOR);	// ����������� �������� �������
	vector<string> directories;	// ��� ���������� � ������� ����������
	vector<string> files;		// ��� ����� � ������� ����������
	int countOfElements = 0;	// ���������� ��������� � ����������
	int position = 0;			// ������� �������

	while (true)
	{
		system("cls");
		cout << "1) Create directory\n2) Create file\n3) Rename\n4) Delete\n5) Exit\n\n";
		cout << fileSystem.getOccMem() << " bytes is used from " << fileSystem.getAllMem() << " bytes\n";
		cout << "Current path: " << fileSystem.getCurPath() << endl << endl;

		countOfElements = directories.size() + files.size();	// ������������ ���������� ��������� ��� ���������

		if (fileSystem.getCurPath() != "/home")	// ���� �� �� � �����, �� ������������ ../
		{
			ARROW(0, position);
			cout << "../" << endl;
			countOfElements++;
		}
		for (int i = 0; i < directories.size(); i++)	// ��������� ���� ���������� � ������� ����������
		{
			ARROW(i + 1, position);
			cout << directories[i] << '/' << endl;
		}
		for (int i = 0; i < files.size(); i++)	// ��������� ���� ������ � ������� ����������
		{
			ARROW(i + 1, position);
			cout << files[i] << '/' << endl;
		}

		switch (_getch())
		{
		case 'w':	// �������� �� ����
			if (position == 0)
				position = countOfElements - 1;
			else
				position--;
			break;
		case 's':	// �������� �� ����
			if (position == countOfElements - 1)
				position = 0;
			else
				position++;
			break;
		case '1':
		{
			system("cls");
			cout << "New directory name: ";
			string dirName;
			while (dirName.empty() || dirName.size() > MAX_NAME_SIZE)
			{
				getline(cin, dirName);
				if (dirName.size() > MAX_NAME_SIZE)
				{
					cout << "Name is too long!" << endl;
					system("pause");
					cout << "New directory name: ";
				}
			}
			fileSystem.createDir(dirName);
			break;
		}
		case '2':
		{
			system("cls");
			cout << "New file name: ";
			string fileName;
			while (fileName.empty() || fileName.size() > MAX_NAME_SIZE)
			{
				getline(cin, fileName);
				if (fileName.size() > MAX_NAME_SIZE)
				{
					cout << "Name is too long!" << endl;
					system("pause");
					cout << "New file name: ";
				}
			}
			fileSystem.createDir(fileName);
			break;
		}
		case '3':
		{
			system("cls");
			if (countOfElements == 0)
			{
				cout << "You didn't choose anything!" << endl;
				system("pause");
				break;
			}
			cout << "New name: ";
			string newName;
			while (newName.empty() || newName.size() > MAX_NAME_SIZE)
			{
				getline(cin, newName);
				if (newName.size() > MAX_NAME_SIZE)
				{
					cout << "Name is too long!" << endl;
					system("pause");
					cout << "New name: ";
				}
			}
			//if ((position <= directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 <= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// ���������, ��� �������: ���������� ��� ����
			//{
			//	fileSystem.renameDir(directories[position])
			//}
			break;
		}
		case '4':
		{
			if (countOfElements == 0)
			{
				cout << "You didn't choose anything!" << endl;
				system("pause");
				break;
			}
			break;
		}
		case '5':
			return 0;
		}
	}
	return 0;
}