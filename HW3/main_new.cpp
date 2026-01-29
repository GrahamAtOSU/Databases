/*
Skeleton code for External storage management
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes_new.h"
using namespace std;


int main(int argc, char* const argv[]) {

    // Initialize the Storage Manager Class with the Binary .dat file name we want to create
    StorageManager manager("EmployeeRelation.dat");

    // Assuming the Employee.CSV file is in the same directory, 
    // we want to read from the Employee.csv and write into the new data_file
    manager.createFromFile("Employee.csv");

    // TODO: You'll receive employee IDs as arguments, process them to retrieve the record, or display a message if not found. 
    for(int i = 1; i < argc; i++){
        int searchId = stoi(argv[i]);
        cout << "Searching for Employee ID: " << searchId << endl;
        manager.findAndPrintEmployee(searchId);
    }





    return 0;
}
