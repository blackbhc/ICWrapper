#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <ctime>

#define LINENUM 6680 // ignore the header row
#define LINEWIDTH 200 // large eough size to restore one row
#define InfoDim 7 // the number of information in each row


using namespace std;

clock_t Begin, End;

int main(int agrc, char* agrv[] ) {
	Begin = clock(); // start point of time
	
	double array[LINENUM][InfoDim]={0.0};
	
	ifstream fin;
	fin.open("/home/bhchen/Pipeline/CPP/ICwrapper/tests/data.dat", ios::in);
	// whether seccussfuly open file
	if (!fin.is_open()) {
		cerr << "File not found." << endl;
		return -1;
	}

	char temp_str[LINEWIDTH];
	fin.getline(temp_str, sizeof(temp_str)); // exclude the first header row

	stringstream ss;
	string cur;
	for(int i=0; i<LINENUM; i++) {
		fin.getline(temp_str, sizeof(temp_str));
		ss.str(temp_str); // set the stringstream
		for (int j=0; j<InfoDim; j++) {
			ss >> cur;
			array[i][j] = stold(cur); //string to double function
		}
		ss.clear(); // clear the stringstream
	}


	for(int i=0; i<9; i++) {
		for (int j=0; j<InfoDim; j++) cout << array[i][j] << " ";
		cout << endl;
	}

		
	End = clock(); // end point of time
	cout << "Program duration: " << double(End - Begin) / CLOCKS_PER_SEC << " s" << endl; // output in unit seconds

	return 0;
}
