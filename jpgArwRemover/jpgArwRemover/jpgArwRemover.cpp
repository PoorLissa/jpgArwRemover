// jpgArwRemover.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>


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
				   _currentDir += static_cast<char>(path[i]);
				}
			}
			else
			{
			   _error = "Could not find current directory";
			}
		}

		void setCurrentDir(wstring str)
		{
			string path("");

			str = str.substr(6, str.length());

			for(unsigned int i = 0; i < str.length(); i++)
			{
				if( str[i] != '"' )
					path += static_cast<char>(str[i]);
			}

			if( path[path.length()] != '\\' )
				path += '\\';

			if( dirExists(path) )
			{
			   _currentDir = path;
			}
			else
			{
			   _error  = "Program started with argument '/path' [" + path + "] which is not valid.\n\nPress 'Enter' to Exit.";
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
		{					auto func = [](vector<string> &vec)
							{ 
								for(size_t i = 0; i < vec.size(); i++)
									cout << "    " << vec[i] << endl;
								
								cout << endl;
							};

			cout << "  Found *.ARW files:" << endl;
			func(vecArw);

			cout << "  Found *.JPG files:" << endl;
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

			sort(vecDif.begin(), vecDif.end());

			return;
		}

		void relocate()
		{
			if( vecDif.size() )
			{
				string dir = _currentDir + "\\_diff";

				cout << "  Moving to " << dir << "\\ ..." << endl;

				CreateDirectoryA(dir.c_str(), NULL);

				if( dirExists(dir) )
				{
					int cnt = 0;

					for(size_t i = 0; i < vecDif.size(); i++)
					{
						wstring old_file = getWStr(_arwDir + "\\" + vecDif[i]);
						wstring new_file = getWStr(    dir + "\\" + vecDif[i]);

						cout << "\t" << vecDif[i] << endl;

						#ifndef _DEBUG

							if( fileExists(new_file) )
							{
								cout << "\t\tFile exists already. Trying to find a new name for it..." << endl;

								wstring testFile;

								int pos = new_file.find_last_of('.');
								int num = 1;

								wstring fName = new_file.substr(0, pos) + L"_extra_file_";
								wstring eName = new_file.substr(pos, new_file.length());

								do  {

									testFile = fName + to_wstring(num) + eName;
									num++;

								}
								while( fileExists(testFile) );

								new_file = testFile;

								wcout << "\t\tNew name found: " << new_file << endl;
							}

							if( MoveFile(old_file.c_str(), new_file.c_str()) )
								cnt++;

						#else

							cnt++;

						#endif
					}

					cout << endl;

					#ifndef _DEBUG
						cout << "  Moved " << cnt << " files." << endl;
					#else
						cout << "  Found " << cnt << " files. Files were NOT moved (Program is running in DEBUG mode)." << endl;
					#endif
				}
			}
			else
			{
				cout << "  No files to move." << endl;
			}

			cout << "\n  Press Enter to exit: ";

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

		string getStr(wstring ws)
		{
			string S;
			
			for(size_t i = 0; i < ws.length(); i++)
				S += static_cast<char>(ws[i]);

			return S;
		}

		bool dirExists(string s)
		{
			wstring wS = getWStr(s);

			DWORD dwAttrib = GetFileAttributes(wS.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		};

		bool fileExists(wstring s)
		{
			DWORD dwAttrib = GetFileAttributes(s.c_str());

			return (dwAttrib != INVALID_FILE_ATTRIBUTES);
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
							name.push_back( static_cast<char>(ffd.cFileName[i]) );
							i++;
						}

						vec.push_back(name);
						res++;
					}
				}
				while( FindNextFile(hFind, &ffd) );

				FindClose(hFind);
			}

			sort(vec.begin(), vec.end());

			return res;
		}

		bool doCompare(string arw, string jpg)
		{											auto removeExt = [](string &s) 
													{ 
														int pos = s.find_last_of('.');
														s = s.substr(0, pos);
													};
			bool res = true;

			removeExt(arw);
			removeExt(jpg);

			res = (arw == jpg.substr(0, arw.length()));

			return res;
		}

	private:

		vector<string> vecArw;
		vector<string> vecJpg;
		vector<string> vecDif;

		string _currentDir, _jpgDir, _arwDir, _error;
};

// ------------------------------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
	myApp app;


	// set path from the argument, if present
    for(int i = 0; i < argc; i++)  
	{
		wstring param(argv[i]);

		if( param.substr(0, 6) == L"/path=" )
		{
			app.setCurrentDir(param);
		}
	}


	if( app.getError().empty() )
	{
		cout << "  Current Directory: '" << app.getExeDir() << "'\n" << endl;

		if( app.getBothDirs() )
		{
			cout << "  Found all necessary sub-directories: 'jpg' and 'arw'.\n" << endl;

			app.getFiles();

			app.showFiles();

			app.findDiff();

			app.relocate();
		}
		else
		{
			cout << "  Error: Could not find necessary sub-directories: 'jpg' and 'arw'." << endl;
		}
	}
	else
	{
		cout << "Error: " << app.getError() << endl;
	}

	getchar();

	return 0;
}
