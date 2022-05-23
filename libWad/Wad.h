#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

class Wad {
public:
	struct Node {
		Node(string name, int offset, int length, Node* parent);
		Node* findChild(string name);
		Node* findParent();
		vector<Node*> children;
		Node* parent;
		string name;
		int offset;
		int length;
		char* contents;
	};
	Wad();
	Node* searchDirectory(string fileName, Node* file);
	Node* getRoot();
	void Load(string magic, const string& path);
	void Print(Node* currentFile);
	void Clear(Node* currentFile);
	void addFile(Node* directory, string name, int offset, int length);
	void removeDelimiters(const string& path, vector<string>& rawPath);
	static Wad* loadWad(const string& path);
	void loadNodes(Node* current);
	bool isMapMarker(string& file);
	bool isDirectoryStart(string& file);
	bool isDirectoryEnd(string& file);
	string getDirectoryName(string& file);
	string getMagic();
	bool isContent(const string& path);
	bool isDirectory(const string& path);
	int getSize(const string& path);
	int getContents(const string& path, char* buffer, int length, int offset = 0);
	int getDirectory(const string& path, vector<string>* directory);
private:
	string wadFile;
	string magic;
	Node* root;
};
