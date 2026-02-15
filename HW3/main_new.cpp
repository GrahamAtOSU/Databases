/*
AUTHORS: Samuel Garcia Lopez, Graham Glazner
ONIDS: garcsamu, glaznerg
EMAILS: garcsamu@oregonstate.edu, glazner@oregonstate.edu

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
    int run = 1;

    while (run)
    {
        int choice = 0;
        cout << "Enter 1 to exit or enter an employee ID: ";
        if (!(cin >> choice)) {
            break;
        }
        if (choice == 1) {
            run = 0;
        } else {
            manager.findAndPrintEmployee(choice);
        }
    } 

    return 0;
}
