/* This is a skeleton code for two-pass multi-way sorting. You can make modifications as long as you meet 
   all question requirements. You are also free to change the return type and arguments as needed. */

#include <bits/stdc++.h>
#include <vector>

#include "record_class4.h"

using namespace std;

#define buffer_size 250 //defines how many pages are available in the Main Memory 


vector<Records> buffers; 

/***TODO: You may need to modify the return type and arguments of the following functions based on your implementation.***/


//Function for PASS 1
// TODO: Complete the following function to sort the buffers and store the sorted records into a temporary file (Runs).
void Sort_Buffer(fstream& empin, fstream& runs){
    // check if empin is open, if not open it
    if (!empin.is_open()) {
        cerr << "Error: Employee.csv file not open, opening in sort_buffer." << endl;
        empin.open("Employee.csv");
        if (!empin.is_open()){
            cerr << "Error: Open failed." << endl;
            return;
        }
    }

    // check if runs is open, if not open it
    if (!runs.is_open()) {
        cerr << "Error: Runs.csv file not open, opening in sort_buffer." << endl;
        runs.open("Runs.csv");
        if (!runs.is_open()){
            cerr << "Error: Open failed." << endl;
            return;
        }
    }

    Records rec;
    
    while(true){
        int num_records = 0;
        while(num_records < buffer_size) { // while there is room in the buffer
        //     read Records of R into buffer
            rec = Grab_Emp_Record(empin);
            if (rec.no_values != -1){
                num_records++;
                buffers.push_back(rec);
            }
            else break;
        }
        
        // sort buffer
        sort(buffers.begin(), buffers.begin() + num_records, Compare_Emp_Records);
        
        // write sorted run to disk
        for (int i = 0; i < num_records; i++){
            runs << buffers[i].emp_record.id << ",";
            runs << buffers[i].emp_record.name << ",";
            runs << buffers[i].emp_record.bio << ",";
            runs << buffers[i].emp_record.manager_id << endl;
        }
        // stop if last record was already read
        if (rec.no_values == -1) break;
        runs << "#" << endl; // delimiter between runs
    }
    return;
}

//Function for PASS 2
// TODO: Complete the following function to merge the sorted temporary files ('runs') and store the final result in EmpSorted.csv using PrintSorted().
void Merge_Runs(fstream& runs, fstream& sort_out){
    // check if runs is open, if not flash error message
    if (!runs.is_open()){
        cerr << "Error: Runs.csv file is not open." << endl;
    }
    if (!sort_out.is_open()){
        cerr << "Errror: Sort_Out.csv file is not open." << endl;
    }
    // for pages of each run
    //     read one page from each run into memory
    Records rec;
    
    while (true)
    {
        rec = Grab_Emp_Record(runs);

    }
    
    
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

    fstream emp_in;     //Open file streams to read and write the Employee.csv
    emp_in.open("Employee.csv", ios::in);  //Opening out the Employee.csv that we want to sort

   
    fstream sort_out; //Open file streams to read and write the EmpSorted.csv
    sort_out.open("EmpSorted.csv", ios::out | ios::app);  //Creating the EmpSorted.csv file where we will store our sorted results

    fstream runs;
    runs.open("Runs.csv", ios::in | ios:: out);

    //TO DO: PASS 1, Create sorted runs for Employee.csv using Sort_Buffer()
    Sort_Buffer(emp_in, runs);

    //TO DO: PASS 2, Use Merge_Runs() to sort the runs and generate EmpSorted.csv


    //Please delete the temporary files (runs) after you've sorted the Employee.csv

    return 0;
}
