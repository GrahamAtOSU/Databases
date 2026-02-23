/* This is a skeleton code for two-pass multi-way sorting. You can make modifications as long as you meet 
   all question requirements. You are also free to change the return type and arguments as needed. */

#include <bits/stdc++.h>
#include "record_class4.h"

using namespace std;

#define buffer_size 250 //defines how many pages are available in the Main Memory 


Records buffers[buffer_size]; 

/***TODO: You may need to modify the return type and arguments of the following functions based on your implementation.***/


//Function for PASS 1
// TODO: Complete the following function to sort the buffers and store the sorted records into a temporary file (Runs).
void Sort_Buffer(fstream empin){
    // check if empin is open, if not flash error message
    if (!empin.is_open()) {
        cerr << "Error: Employee.csv file is not open for sort_buffer." << endl;
        return;
    }
    // while there are more records to read from empin
    Records rec;
    int buffer_index = 0;
    while(buffer_index < buffer_size) {
    //     read Records of R into buffer
        rec = Grab_Emp_Record(empin);
    }
    //     sort buffer
        sort(buffers[0], buffers[buffer_size - 1], Compare_Emp_Record);
    //     write sorted run to disk

    return;
}

//Function for PASS 2
// TODO: Complete the following function to merge the sorted temporary files ('runs') and store the final result in EmpSorted.csv using PrintSorted().
void Merge_Runs(fstream runs, fstream SortOut){
    // check if runs is open, if not flash error message
    // for pages of each run
    //     read one page from each run into memory
    //     merge into sorted output
    //     write to output
    return;
}

// TODO: Complete the following function to store the sorted results from PASS 2 into EmpSorted.csv.
void PrintSorted(){
    // for each record in sorted output
    //     print record to output
    return;
}

int main() {

    fstream empin;     //Open file streams to read and write the Employee.csv
    empin.open("Employee.csv", ios::in);  //Opening out the Employee.csv that we want to sort

   
    fstream SortOut; //Open file streams to read and write the EmpSorted.csv
    SortOut.open("EmpSorted.csv", ios::out | ios::app);  //Creating the EmpSorted.csv file where we will store our sorted results


    //TO DO: PASS 1, Create sorted runs for Employee.csv using Sort_Buffer()


    //TO DO: PASS 2, Use Merge_Runs() to sort the runs and generate EmpSorted.csv


    //Please delete the temporary files (runs) after you've sorted the Employee.csv

    return 0;
}
