// Main.cpp

#include "VFS.h"
#include <conio.h>

#define ARROW(x, y)\
			if (x == y)\
				(std::cout << "->\t");
#define CURSOR(x, y)\
			if (x == y)\
				(std::cout << "|");
#define FILE_EMULATOR "C:/Users/Intel/Desktop/VFS.VFS"

void fileMode(VFS&);

int main()
{
	bool changes = true;		// Были ли какие-нибудь изменения в файловой системе
	VFS fileSystem(FILE_EMULATOR);	// Виртуальная файловая система
	vector<string> directories;	// Все директории в текущей директории
	vector<string> files;		// Все файлы в текущей директории
	int countOfElements = 0;	// Количество элементов в директории
	int position;				// Текущая позиция

	while (true)
	{
		system("cls");
		cout << "1) Create directory\n2) Create file\n3) Rename\n4) Delete\n5) Exit\n\n";
		cout << fileSystem.getOccMem() << " bytes is used from " << fileSystem.getAllMem() << " bytes\n";
		cout << "Current path: " << fileSystem.getCurPath() << endl << endl;

		if (changes)
		{
			directories = fileSystem.getDirectories();	// Получаем названия всех директорий
			files = fileSystem.getFiles();	// Получаем названия всех файлов
			countOfElements = directories.size() + files.size();	// Рассчитываем количество элементов для навигации
			position = 0;
		}

		if (fileSystem.getCurPath() != "/home")	// Если мы не в корне, то отрисовываем ../
		{
			ARROW(0, position);
			cout << "\t../" << endl;
			if (changes)
				countOfElements++;
		}
		for (int i = 0; i < directories.size(); i++)	// Отрисовка всех директорий в текущей директории
		{
			if (fileSystem.getCurPath() == "/home") { ARROW(i, position); }
			else { ARROW(i + 1, position); }
			cout << '\t' << directories[i] << '/' << endl;
		}
		for (int i = 0; i < files.size(); i++)	// Отрисовка всех файлов в текущей директории
		{
			if (fileSystem.getCurPath() == "/home") { ARROW(i + directories.size(), position); }
			else { ARROW(i + directories.size() + 1, position); }
			cout << '\t' << files[i] << endl;
		}

		changes = false;

		switch (_getch())
		{
		case 'w':	// Движение по меню
			if (position == 0)
				position = countOfElements - 1;
			else
				position--;
			break;
		case 's':	// Движение по меню
			if (position == countOfElements - 1)
				position = 0;
			else
				position++;
			break;
		case '\r':	// Если мы перешли в директорию или в файл
			if (countOfElements != NULL)	// Если есть элементы
			{
				if (position == 0 && fileSystem.getCurPath() != "/home")
				{
					fileSystem.back();
					changes = true;
				}
				else if ((position < directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 < directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
				{
					if (fileSystem.getCurPath() == "/home")
						fileSystem.enterDir(directories[position]);
					else
						fileSystem.enterDir(directories[position - 1]);
					changes = true;
				}
				else if ((position >= directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 >= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
				{
					if (fileSystem.getCurPath() == "/home")
						fileSystem.enterFile(files[position - directories.size()]);
					else
						fileSystem.enterFile(files[position - 1 - directories.size()]);
					fileMode(fileSystem);
					fileSystem.back();
				}
			}
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
			if (fileSystem.createDir(dirName))
				changes = true;
			break;
		}
		case '2':
		{
			system("cls");
			cout << "New file name: ";
			string fileName;
			while (fileName.empty() || fileName.size() > MAX_NAME_SIZE - 5)	// Не учитываем .file
			{
				getline(cin, fileName);
				if (fileName.size() > MAX_NAME_SIZE)
				{
					cout << "Name is too long!" << endl;
					system("pause");
					cout << "New file name: ";
				}
			}
			fileName += ".file";
			if (fileSystem.createFile(fileName))
				changes = true;
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
				if ((newName.size() > MAX_NAME_SIZE && (position < directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 <= directories.size() && position != 0 && fileSystem.getCurPath() != "/home")) ||
					(newName.size() > MAX_NAME_SIZE - 5 && (position >= directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 >= directories.size() && position != 0 && fileSystem.getCurPath() != "/home")))
				{
					cout << "Name is too long!" << endl;
					system("pause");
					cout << "New name: ";
				}
			}
			if ((position < directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 <= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
			{
				if (fileSystem.getCurPath() == "/home")
					fileSystem.renameDir(directories[position], newName);
				else
					fileSystem.renameDir(directories[position - 1], newName);
				changes = true;
			}
			if ((position >= directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 >= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
			{
				if (fileSystem.getCurPath() == "/home")
					fileSystem.renameFile(files[position - directories.size()], newName + ".file");
				else
					fileSystem.renameFile(files[position - 1 - directories.size()], newName + ".file");
				changes = true;
			}
			break;
		}
		case '4':
		{
			system("cls");
			if (countOfElements == 0)
			{
				cout << "You didn't choose anything!" << endl;
				system("pause");
				break;
			}
			char choice = 0;
			cout << "Are you sure you want to delete this ?\ny - yes / n - not\n";
			rewind(stdin);
			while (!(choice = _getch()) || (choice != 'y' && choice != 'n'))
			{
				rewind(stdin);
				cout << "Error! Try again\n";
			}
			if (choice == 'y')
			{
				if ((position < directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 <= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
				{
					if (fileSystem.getCurPath() == "/home")
						fileSystem.deleteDir(directories[position]);
					else
						fileSystem.deleteDir(directories[position - 1]);
					changes = true;
				}
				if ((position >= directories.size() && fileSystem.getCurPath() == "/home") || (position - 1 >= directories.size() && position != 0 && fileSystem.getCurPath() != "/home"))	// Проверяем, что выбрано: директория или файл
				{
					if (fileSystem.getCurPath() == "/home")
						fileSystem.deleteFile(files[position - directories.size()]);
					else
						fileSystem.deleteFile(files[position - 1 - directories.size()]);
					changes = true;
				}
				position = 0;
			}
			break;
		}
		case '5':
			system("cls");
			cout << "Are you sure you want to exit ?\ny - yes / n - not\n";
			char choice = 0;
			rewind(stdin);
			while (!(choice = _getch()) || (choice != 'y' && choice != 'n'))
			{
				rewind(stdin);
				cout << "Error! Try again\n";
			}
			if (choice == 'y')
				return 0;
		}
	}
	return 0;
}

void fileMode(VFS& fileSystem)
{
	int level = 1;
	string text = fileSystem.openFile();
	while (true)
	{
		system("cls");
		cout << "Current path: " << fileSystem.getCurPath() << endl << endl;

		ARROW(level, 0);
		cout << "Save" << endl;
		ARROW(level, 1);
		cout << "Back" << endl;

		cout << text;
		
		switch (char symb = (char)_getch())
		{
		case -32:
		case 0:
			switch (_getch())
			{
			case 72:
				if (level != 0)
					level--;
				break;
			//case 75:
			//	if (level == 2)
			//		if (position != 0)
			//			position--;
			//	break;
			//case 77:
			//	if (level == 2)
			//		if (position != text.size() - 1)
			//			position++;
			//	break;
			case 80:
				if (level != 2)
					level++;
				break;
			}
			break;
		case '\r':
			if (level == 0)
			{
				fileSystem.saveFile(text);
				return;
			}
			else if (level == 1)
				return;
			else
				text.push_back('\n');
			break;
		case '\b':
			if (text.size() != 0)
				text.pop_back();
			break;
		default:
			if (level == 2)
				text.push_back(symb);
		}
	}
}