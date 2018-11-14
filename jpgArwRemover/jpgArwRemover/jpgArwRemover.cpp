// jpgArwRemover.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>


using namespace std;


class myApp {

	public:

		myApp() 
		{
			TCHAR path[_MAX_PATH];

			int len = GetCurrentDirectory(_MAX_PATH, path);

			if( len > 0 )
			{
			   _currentDir.clear();
			   _error.clear();

				for(int i = 0; i < len; i++)
				{
				   _currentDir += (char)path[i];
				}
			}
			else
			{
			   _error = "Could not find current directory";
			}
		}

		bool getBothDirs()
		{
			bool res = false;

		   _jpgDir = _currentDir + "\\jpg\\";
		   _arwDir = _currentDir + "\\arw\\";

			if( dirExists( _jpgDir ) && dirExists( _arwDir ) )
				res = true;

			return res;
		}

		void getFiles()
		{
			int num;
			
			num = findFiles(_arwDir, vecArw, "*.arw");
			num = findFiles(_jpgDir, vecJpg, "*.jpg");

			return;
		}

		void showFiles()
		{
			auto func = [](vector<string> &vec) { for(size_t i = 0; i < vec.size(); i++)
												      cout << "    " << vec[i] << endl;
												};
			cout << "  *.ARW:" << endl;
			func(vecArw);

			cout << "  *.JPG:" << endl;
			func(vecJpg);

			return;
		}

		void findDiff()
		{
			vecDif.clear();

			for(size_t i = 0; i < vecArw.size(); i++)
			{
				bool found = false;

				string arw = vecArw[i];

				for(size_t j = 0; j < vecJpg.size(); j++)
				{
					string jpg = vecJpg[j];

					if( doCompare(arw, jpg) )
					{
						found = true;
						break;
					}
				}

				if( !found )
				{
					vecDif.push_back(arw);
				}
			}

			return;
		}

		void relocate()
		{
			if( vecDif.size() )
			{
				cout << "Moving..." << endl;

				string dir = _currentDir + "\\_diff";

				CreateDirectoryA(dir.c_str(), NULL);

				if( dirExists(dir) )
				{
					for(size_t i = 0; i < vecDif.size(); i++)
					{
						wstring old_file = getWStr(_arwDir + "\\" + vecDif[i]);
						wstring new_file = getWStr(    dir + "\\" + vecDif[i]);

						cout << "\t" << vecDif[i] << endl;

						MoveFile(old_file.c_str(), new_file.c_str());
					}
				}
			}

			return;
		}

		string getExeDir() { return _currentDir; }
		string getError()  { return _error;		 }

	private:

		wstring getWStr(string s)
		{
			wstring wS;
			
			for(size_t i = 0; i < s.length(); i++)
				wS += s[i];

			return wS;
		}

		bool dirExists(string s)
		{
			wstring wS = getWStr(s);

			DWORD dwAttrib = GetFileAttributes(wS.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		};

		int findFiles(string dir, vector<string> &vec, char *mask)
		{
			WIN32_FIND_DATA ffd;
			TCHAR szDir[MAX_PATH];
			HANDLE hFind = INVALID_HANDLE_VALUE;

			int res = 0;
			vec.clear();
			dir += mask;

			for(size_t i = 0; i < dir.length(); i++)
			{
				szDir[i+0] = dir[i];
				szDir[i+1] = '\0';
			}

			hFind = FindFirstFile(szDir, &ffd);

			if( hFind != INVALID_HANDLE_VALUE )
			{
				do {

					if( !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					{
						int i = 0;
						string name("");

						while( ffd.cFileName[i] != '\0' )
						{
							name.push_back(ffd.cFileName[i]);
							i++;
						}

						vec.push_back(name);
						res++;
					}
				}
				while( FindNextFile(hFind, &ffd) );

				FindClose(hFind);
			}

			return res;
		}

		bool doCompare(string arw, string jpg)
		{
			auto removeExt = [](string &s) { 
									int pos = s.find_last_of('.');
									s = s.substr(0, pos);
			};

			auto remove1st = [](string &s) { 
									int pos = s.find_last_of('.');
									s = s.substr(1, pos);
			};

			bool res = true;

			removeExt(arw);
			removeExt(jpg);

			remove1st(arw);
			remove1st(jpg);

			int len = arw.length();

			res = (arw == jpg.substr(0, len));

			return res;
		}

	private:

		vector<string> vecArw;
		vector<string> vecJpg;
		vector<string> vecDif;

		string _currentDir, _jpgDir, _arwDir, _error;
};



int _tmain(int argc, _TCHAR* argv[])
{
	myApp app;

	if( app.getError().empty() )
	{
		cout << "Current Directory: '" << app.getExeDir() << "'\n" << endl;

		if( app.getBothDirs() )
		{
			cout << "Found both child directories: 'jpg' and 'arw':" << endl;

			app.getFiles();

			app.showFiles();

			app.findDiff();

			app.relocate();
		}
		else
		{
			cout << "Error: Could not find child directories: 'jpg' and 'arw'" << endl;
		}
	}
	else
	{
		cout << "Error: " << app.getError() << endl;
	}

	getchar();

	return 0;
}
