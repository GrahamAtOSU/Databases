#include <bits/stdc++.h>
#include <vector>
#include <string>
#include <cstdio>

#include "record_class4.h"

using namespace std;

#define buffer_size 250 //defines how many pages are available in the Main Memory 


vector<Records> buffers;
vector<string> run_filenames;

/***TODO:DONE You may need to modify the return type and arguments of the following functions based on your implementation.***/


//Function for PASS 1
// TODO:DONE Complete the following function to sort the buffers and store the sorted records into a temporary file (Runs).
void Sort_Buffer(fstream& empin){
    // check if empin is open, if not open it
    if (!empin.is_open()) {
        cerr << "Error: Employee.csv file not open, opening in sort_buffer." << endl;
        empin.open("Employee.csv");
        if (!empin.is_open()){
            cerr << "Error: Open failed." << endl;
            return;
        }
    }

    Records rec;
    int run_index = 0;
    while(true){
        int num_records = 0;

        while(num_records < buffer_size) { // while there is room in the buffer
            // read Records of R into buffer
            rec = Grab_Emp_Record(empin);
            if (rec.no_values != -1){
                num_records++;
                buffers.push_back(rec);
            }
            else break;
        }
        
        // sort buffer
        sort(buffers.begin(), buffers.begin() + num_records, Compare_Emp_Records);
        
        string run_filename = "run_" + to_string(run_index) + ".csv";
        fstream run;
        run.open(run_filename, ios::out);

        // write sorted run to disk
        for (int i = 0; i < num_records; i++){
            run << buffers[i].emp_record.id << ",";
            run << buffers[i].emp_record.name << ",";
            run << buffers[i].emp_record.bio << ",";
            run << buffers[i].emp_record.manager_id << endl;
        }

        run_filenames.push_back(run_filename);
        run_index++;
        
        run.close();

        while(!buffers.empty()){
            buffers.pop_back();
        }

        // stop if last record was already read
        if (rec.no_values == -1) break;
    }

    return;
}

//Function for PASS 2
// TODO:DONE Complete the following function to merge the sorted temporary files ('runs') and store the final result in EmpSorted.csv using PrintSorted().
void Merge_Runs(fstream& sort_out){
    // check if output file is open, if not flash error message
    if (!sort_out.is_open()){
        cerr << "Errror: EmpSorted.csv file is not open." << endl;
    }
    
    vector<fstream> runs(run_filenames.size());
    
    for (int i = 0; i < run_filenames.size(); i++){ // for each run
        // read one page from each run into memory
        
        runs[i].open(run_filenames[i], ios::in);

        Records rec;
        rec = Grab_Emp_Record(runs[i]);
        buffers.push_back(rec);
    }
    
    // merge into sorted output
    while (true)
    {   
        // lowest gets reset each cycle
        int lowest_id = INT_MAX;
        int lowest_id_index = -1;
        
        // search for lowest id
        for (int i = 0; i < buffers.size(); i++){
            if (buffers[i].no_values != -1){
                if (buffers[i].emp_record.id < lowest_id){
                    lowest_id = buffers[i].emp_record.id;
                    lowest_id_index = i;
                }
            }
        }

        if (lowest_id_index == -1) break; // if all buffers contain no value records, end loop
        
        // write to output
        sort_out << buffers[lowest_id_index].emp_record.id << ",";
        sort_out << buffers[lowest_id_index].emp_record.name << ",";
        sort_out << buffers[lowest_id_index].emp_record.bio << ",";
        sort_out << buffers[lowest_id_index].emp_record.manager_id << endl;

        // get new page into buffer
        Records rec;
        rec = Grab_Emp_Record(runs[lowest_id_index]);
        buffers[lowest_id_index] = rec;
    }
    
    return;
}

int main() {

    fstream emp_in;     //Open file streams to read and write the Employee.csv
    emp_in.open("Employee.csv", ios::in);  //Opening out the Employee.csv that we want to sort

   
    fstream sort_out; //Open file streams to read and write the EmpSorted.csv
    sort_out.open("EmpSorted.csv", ios::out | ios::app);  //Creating the EmpSorted.csv file where we will store our sorted results

    //TO DO: PASS 1, Create sorted runs for Employee.csv using Sort_Buffer()
    Sort_Buffer(emp_in);

    //TO DO: PASS 2, Use Merge_Runs() to sort the runs and generate EmpSorted.csv
    Merge_Runs(sort_out);

    //Please delete the temporary files (runs) after you've sorted the Employee.csv
    
    for (const string& filename : run_filenames) {
        if (remove(filename.c_str()) != 0) {
            cerr << "Error: Could not delete " << filename << endl;
        }
    }
    run_filenames.clear();
    
    return 0;
}