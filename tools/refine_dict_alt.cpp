//Tool to remove all words with less than 3 or more than 7 letters

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

using namespace std;

int main(){
	string ifpath, ofpath;//Input and output file path
	
	cout << "Enter input file path: " << flush;
	getline(cin, ifpath);//Gets path
	
	cout << "Enter output file path: " << flush;
	getline(cin, ofpath);//Gets path
	
	ifstream i (ifpath.c_str());
	ofstream o (ofpath.c_str());
	
	string tmpstr;//Temporary string
	while (i.good()){
		getline(i, tmpstr);//Gets string
		
		if (tmpstr.size() >= 3 && tmpstr.size() <= 7) o << tmpstr << "\n";
	}
	
	return 0;
}