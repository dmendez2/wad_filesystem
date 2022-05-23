#include "Wad.h"

Wad::Node::Node(string name, int offset, int length, Node* parent) {
	this->name = name;
	this->offset = offset;
	this->length = length;
	this->parent = parent;
	contents = new char[length];
}

Wad::Node* Wad::Node::findChild(string name) {
	for (int ii = 0; ii < children.size(); ii += 1) {
		if (children.at(ii)->name == name) {
			return children.at(ii);
		}
	}
	return nullptr;
}

void Wad::Load(string magic, const string& path) {
	this->magic = magic;
	this->wadFile = path;
}

Wad::Node* Wad::Node::findParent() {
	return parent;
}

Wad::Wad() {
	root = new Node("/", 0, 0, nullptr);
}

void Wad::Clear(Node* currentFile) {
	if (currentFile != nullptr) {
		vector<Node*> children = currentFile->children;
		for (int ii = 0; ii < children.size(); ii += 1) {
			Clear(children.at(ii));
		}
		delete[] currentFile->contents;
		delete currentFile;
	}
}

void Wad::addFile(Node* directory, string name, int offset, int length) {
	if (directory != nullptr) {
		Node* newFile = new Node(name, offset, length, directory);
		directory->children.push_back(newFile);
	}
}

Wad::Node* Wad::searchDirectory(string fileName, Node* file) {
	if (file->name == fileName) {
		return file;
	}
	else {
		vector<Node*> children = file->children;
		Node* temp;
		for (int ii = 0; ii < children.size(); ii += 1) {
			string currentFile = children.at(ii)->name;
			if (currentFile == fileName) {
				return children.at(ii);
			}
			else {
				temp = searchDirectory(fileName, children.at(ii));
				if (temp != nullptr) {
					return temp;
				}
			}
		}
	}
	return nullptr;
}

void Wad::Print(Node* currentFile) {
	if (currentFile != nullptr) {
		vector<Node*> children = currentFile->children;
		cout << currentFile->name << " ";
		cout << currentFile->offset << " ";
		cout << currentFile->length << endl;
		for (int ii = 0; ii < children.size(); ii += 1) {
			Print(children.at(ii));
		}
	}
}

Wad::Node* Wad::getRoot() {
	return root;
}

string Wad::getMagic() {
	return magic;
}

Wad* Wad::loadWad(const string& path) {

	Wad* fileSystem = new Wad(); 

	Node* root = fileSystem->getRoot();
	Wad::Node* workingDirectory = root;

	//Opening the file
	ifstream inFile(path, ios::binary);

	//Setting up our variables to store the header data
	string magic;
	int numDescriptors;
	int descriptorOffset;

	if (inFile.is_open()) {
		//Reading the Wad file header
		for (int ii = 0; ii < 4; ii += 1) {
			char temp;
			inFile.read((char*)&temp, sizeof(temp));
			magic.push_back(temp);
		}
		inFile.read((char*)&numDescriptors, sizeof(numDescriptors));
		inFile.read((char*)&descriptorOffset, sizeof(descriptorOffset));

		//Storing the magic of the Wad file and the Wad filename 
		fileSystem->Load(magic, path);

		//Going to the offset given by the Wad file header
		inFile.seekg(descriptorOffset);
		for (int ii = 0; ii < numDescriptors; ii += 1) {
			//Setting up our variables to store file data
			int elementOffset;
			int elementLength;
			string elementName;
			inFile.read((char*)&elementOffset, sizeof(elementOffset));
			inFile.read((char*)&elementLength, sizeof(elementLength));
			for (int jj = 0; jj < 8; jj += 1) {
				char temp;
				inFile.read((char*)&temp, sizeof(temp));
				if (isalpha(temp) || isdigit(temp) || ispunct(temp)) {
					elementName.push_back(temp);
				}
			}
			
			//Checks to see if file has suffix '_START' If it does, adds file to current directory. Then changes working directory to the just added file
			if (fileSystem->isDirectoryStart(elementName)) {
				string directoryName = fileSystem->getDirectoryName(elementName);
				fileSystem->addFile(workingDirectory, directoryName, elementOffset, elementLength);
				workingDirectory = workingDirectory->findChild(directoryName);
			}
			//If the file has suffix 'END', changes working directory to the parent directory
			else if (fileSystem->isDirectoryEnd(elementName)) {
				workingDirectory = workingDirectory->findParent();
			}
			//If the file has the format 'EXMY', where X and Y are numbers 0-9, then this is a map marker. Add map marker to current directory and then change working directory to the map marker
			//The next 10 files will be added in the map marker directory, then revert the working directory to the parent directory
			else if (fileSystem->isMapMarker(elementName)) {
				fileSystem->addFile(workingDirectory, elementName, elementOffset, elementLength);
				workingDirectory = workingDirectory->findChild(elementName);
				int limit = ii + 10;
				for (ii; ii < limit; ii += 1) {
					int markerOffset;
					int markerLength;
					string markerName;
					inFile.read((char*)&markerOffset, sizeof(markerOffset));
					inFile.read((char*)&markerLength, sizeof(markerLength));
					for (int jj = 0; jj < 8; jj += 1) {
						char temp2;
						inFile.read((char*)&temp2, sizeof(temp2));
						if (isalpha(temp2) || isdigit(temp2) || ispunct(temp2)) {
							markerName.push_back(temp2);
						}
					}
					fileSystem->addFile(workingDirectory, markerName, markerOffset, markerLength);
				}
				workingDirectory = workingDirectory->findParent();
			}
			//If the file is not of directory format, add a file to the current working directory
			else {
				fileSystem->addFile(workingDirectory, elementName, elementOffset, elementLength);
			}
			
		}
			//Closing the file
			inFile.close();
		}
		fileSystem->loadNodes(fileSystem->root);
		return fileSystem;
}

void Wad::loadNodes(Node* current) {
	if (current->length != 0) {
		ifstream inFile(wadFile, ios::binary);
		if (inFile.is_open()) {
			inFile.seekg(current->offset);
			inFile.read(current->contents, current->length);
		}
		inFile.close();
	}

	for (int ii = 0; ii < current->children.size(); ii += 1) {
		loadNodes(current->children[ii]);
	}
}

bool Wad::isMapMarker(string& file) {
	if (file.at(0) == 'E' && isdigit(file.at(1)) && file.at(2) == 'M' && isdigit(file.at(3)))
		return true;
	else
		return false;
}

bool Wad::isDirectoryStart(string& file) {
	string startString = "_START";
	size_t found = file.find(startString);

	if (found != string::npos && found == file.size()-6)
		return true;
	else
		return false;
}

bool Wad::isDirectoryEnd(string& file) {
	string endString = "_END";
	size_t found = file.find(endString);

	if (found != string::npos && found == file.size()-4)
		return true;
	else
		return false;
}

string Wad::getDirectoryName(string& file) {
	int ii = 0;
	string name;
	while (file.at(ii) != '_') {
		name.push_back(file.at(ii));
		ii += 1;
	}
	return name;
}

bool Wad::isContent(const string& path) {
	vector<string> rawPath;
	removeDelimiters(path, rawPath);
	string fileName = rawPath.at(rawPath.size() - 1);
	Node* file = searchDirectory(fileName, root); 
	if (file != nullptr) {
		if (file->length != 0)
			return true;
		else
			return false;
	}
	else {
		return false;
	}
}

bool Wad::isDirectory(const string& path) {
	vector<string> rawPath;
	removeDelimiters(path, rawPath);
	string fileName = rawPath.at(rawPath.size() - 1);
	Node* file = searchDirectory(fileName, root);
	if (file != nullptr) {
		if (file->length == 0)
			return true;
		else
			return false;
	}
	else {
		return false;
	}
}

int Wad::getSize(const string& path) {
	if (isContent(path)) {
		vector<string> rawPath;
		removeDelimiters(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		Node* file = searchDirectory(fileName, root);
		return file->length;
	}
	else {
		return -1;
	}
}

void Wad::removeDelimiters(const string& path, vector<string>& rawPath) {
	int ii = 0;
	rawPath.push_back("/");
	while (ii < path.size()) {
		string temp;
		while (ii < path.size() && path.at(ii) != '/') {
			temp.push_back(path.at(ii));
			ii += 1;
		}
		if (temp.size() > 0) {
			rawPath.push_back(temp);
		}
		ii += 1;
	}
}

int Wad::getDirectory(const string& path, vector<string>* directory) {
	if (isDirectory(path)) {
		vector<string> rawPath;
		removeDelimiters(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		Node* file = searchDirectory(fileName, root);

		for (int ii = 0; ii < file->children.size(); ii += 1) {
			directory->push_back(file->children.at(ii)->name);
		}
		return file->children.size();
	}
	else {
		return -1;
	}
}

int Wad::getContents(const string& path, char* buffer, int length, int offset) {
	if (isContent(path)) {
		vector<string> rawPath;
		removeDelimiters(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		Node* file = searchDirectory(fileName, root);

		if (offset >= file->length) {
			return -1;
		}

		memcpy(buffer, file->contents + offset, length);
		return length;
	}
	else {
		return -1;
	}
}





