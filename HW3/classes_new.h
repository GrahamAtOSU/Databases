/*** This is just a Skeleton/Starter Code for the External Storage Assignment. This is by no means absolute, in terms of assignment approach/ used functions, etc. ***/
/*** You may modify any part of the code, as long as you stick to the assignments requirements we do not have any issue ***/

// Include necessary standard library headers
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
using namespace std; // Include the standard namespace

class Record {
public:
    int id, manager_id; // Employee ID and their manager's ID
    std::string bio, name; // Fixed length string to store employee name and biography

    Record(vector<std::string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    //You may use this for debugging / showing the record to standard output. 
    void print() const {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }

    int get_size(){ // Returns size of the record
        return int(sizeof(int) * 2 + bio.length() + name.length());
    }
    
    // Take a look at Figure 9.9 and read the Section 9.7.2 [Record Organization for Variable Length Records]
    // TO_DO: Consider using a delimiter in the serialize function to separate these items for easier parsing.
    string serialize() const {
        ostringstream oss;

        oss.write(reinterpret_cast<const char*>(&id), sizeof(id));                  // Writes the binary representation of the ID.
        oss.write(reinterpret_cast<const char*>(&manager_id), sizeof(manager_id));  // Writes the binary representation of the Manager id
        int name_len = name.size();
        int bio_len = bio.size();
        oss.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));      // Writes the size of the Name in binary format.
        oss.write(name.c_str(), name.size());                                       // Writes the name in binary form
        oss.write(reinterpret_cast<const char*>(&bio_len), sizeof(bio_len));        // Writes the size of the Bio in binary format. 
        oss.write(bio.c_str(), bio.size());                                         // Writes bio in binary form

    }
};

class page{ // Take a look at Figure 9.7 and read Section 9.6.2 [Page organization for variable length records] 
public:
    vector <Record> records; // Data Area: Stores records. 
    vector <pair <int, int>> slot_directory; // This slot directory contains the starting position (offset), and size of the record. 
                                        
    int cur_size = 0; // holds the current size of the page

    void clear(){ // Clear the page for re-use
        records.clear();
        slot_directory.clear();
        cur_size = 0;
    }

    bool insert_record_into_page(Record r){  // Write a Record into your page (main memory)
        
        if(cur_size + r.get_size() >= 4096){  // Checking if the current Record can be entered or not   

            return false; // You cannot insert the current record into this page
        }
        else{
            records.push_back(r); // Record stored in current page
            
            // DONE: update slot directory information            
            slot_directory.push_back({cur_size, r.get_size()}); // Updating slot directory with offset and size

            cur_size += r.get_size(); // Updating page size

            return true;
        }
        

    }

    // Function to write the page to a binary file, i.e., EmployeeRelation.dat file
    void write_into_data_file(ostream& out) const { 
        
        char page_data[4096] = {0}; // Write the page contents (records and slot directory) into this char array so that the page can be written to the data file in one go.

        int offset = 0; // Used as an iterator to indicate where the next item should be stored. Section 9.6.2 contains information that will help you with the implementation.

        for (const auto& record : records) { // Writing the records into the page_data
            string serialized = record.serialize();

            memcpy(page_data + offset, serialized.c_str(), serialized.size());

            offset += serialized.size();
        }

        // DONE: Put a delimiter here to indicate slot directory starts from here 
        const char slot_directory_delimiter = '#'; // Delimiter to indicate start of slot directory
        memcpy(page_data + offset, reinterpret_cast<const char*>(&slot_directory_delimiter), sizeof(slot_directory_delimiter));
        offset += sizeof(slot_directory_delimiter);

        for (const auto& slots : slot_directory) { // DONE: Write the slot-directory information into page_data. You'll use slot-directory to retrieve record(s).
            memcpy(page_data + offset, reinterpret_cast<const char*>(&slots.first), sizeof(slots.first));
            offset += sizeof(slots.first);
            memcpy(page_data + offset, reinterpret_cast<const char*>(&slots.second), sizeof(slots.second));
            offset += sizeof(slots.second);
        }

        out.write(page_data, sizeof(page_data)); // Write the page_data to the EmployeeRelation.dat file 

    }

    // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to populate a page object
    bool read_from_data_file(istream& in, int employee_id) {
        char page_data[4096] = {0}; // Character array used to read 4 KB from the data file to your main memory. 
        in.read(page_data, 4096); // Read a page of 4 KB from the data file 


        streamsize bytes_read = in.gcount(); // used to check if 4KB was actually read from the data file
        if (bytes_read == 4096) {
            
            // DONE: Process page_data (4 KB page) and put the information into records and slot_directory (main memory).
            clear(); // Clear existing data before populating
            int offset = 0;
            // Read record fields
            while (offset < 4096) {
                // Check for slot directory delimiter
                if (page_data[offset] == '#') {
                    offset += sizeof(char); // Move past the delimiter
                    break; // Exit loop to start reading slot directory
                }
                // Read ID
                int id;
                id = *reinterpret_cast<int*>(page_data + offset);
                offset += sizeof(int); // Move past ID

                // Read Manager ID
                int manager_id;
                manager_id = *reinterpret_cast<int*>(page_data + offset);
                offset += sizeof(int); // Move past Manager ID

                // Read Name length and Name
                int name_len = *reinterpret_cast<int*>(page_data + offset);
                offset += sizeof(int); // Move past Name length
                string name;
                name = string(page_data + offset, name_len);
                offset += name_len; // Move past Name

                // Read Bio length and Bio
                int bio_len = *reinterpret_cast<int*>(page_data + offset);
                offset += sizeof(int); // Move past Bio length
                string bio;
                bio = string(page_data + offset, bio_len);
                offset += bio_len; // Move past Bio

                vector<string> fields = {to_string(id), name, bio, to_string(manager_id)}; // Create fields vector
                
                Record r = Record(fields);  // Construct Record object

                records.push_back(r); // Add record to records vector (of this page)
            }
            // Read slot directory
            while (offset + sizeof(int) * 2 <= 4096) {
                int record_offset = *reinterpret_cast<int*>(page_data + offset);    // Read record offset
                offset += sizeof(int);
                int record_size = *reinterpret_cast<int*>(page_data + offset);      // Read record size
                offset += sizeof(int);
                slot_directory.push_back({record_offset, record_size});             // Add to slot directory
            }
            // TO_DO: Modify this function to search for employee ID in the page you just loaded to main memory.


            for (const Record& rec : records) {
                if (rec.id == employee_id) {
                    rec.print(); // Print the record if found
                    return true; // Record found
                }
            } 
        }

        if (bytes_read > 0) { 
            cerr << "Incomplete read: Expected " << 4096 << " bytes, but only read " << bytes_read << " bytes." << endl;
        }

        return false;
    }
};

class StorageManager {

public:
    string filename;  // Name of the file (EmployeeRelation.dat) where we will store the Pages 
    fstream data_file; // fstream to handle both input and output binary file operations
    vector <page> buffer; // You can have maximum of 3 Pages.
    
    // Constructor that opens a data file for binary input/output; truncates any existing data file
    StorageManager(const string& filename) : filename(filename) {
        data_file.open(filename, ios::binary | ios::out | ios::in | ios::trunc);
        if (!data_file.is_open()) {  // Check if the data_file was successfully opened
            cerr << "Failed to open data_file: " << filename << endl;
            exit(EXIT_FAILURE);  // Exit if the data_file cannot be opened
        }
    }

    // Destructor closes the data file if it is still open
    ~StorageManager() {
        if (data_file.is_open()) {
            data_file.close();
        }
    }

    // Reads data from a CSV file and writes it to EmployeeRelation.dat
    void createFromFile(const string& csvFilename) {
        buffer.resize(3); // You can have maximum of 3 Pages.

        ifstream csvFile(csvFilename);  // Open the Employee.csv file for reading
        
        string line, name, bio;
        int id, manager_id;
        int page_number = 0; // Current page we are working on [at most 3 pages]

        while (getline(csvFile, line)) {   // Read each line from the CSV file, parse it, and create Employee objects
            stringstream ss(line);
            string item;
            vector<string> fields;

            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record r = Record(fields);  //create a record object            

            if (!buffer[page_number].insert_record_into_page(r)) { // inserting that record object to the current page
                
                // Current page is full, move to the next page
                page_number++;
 
                if (page_number >= buffer.size()) {    // Checking if page limit has been reached.
                    
                    for (page& p : buffer) { // using write_into_data_file() to write the pages into the data file from buffer
                        p.write_into_data_file(data_file);
                        p.clear(); // Clear the page after writing to the data file
                    }
                    page_number = 0; // Starting again from page 0 of the buffer   
                }
                buffer[page_number].insert_record_into_page(r); // Reattempting the insertion of record 'r' into the newly created page
            }
            
        }
        csvFile.close();  // Close the CSV file
    }

    // Searches for an Employee ID in EmployeeRelation.dat
    void findAndPrintEmployee(int searchId) {
        
        data_file.seekg(0, ios::beg);  // Rewind the data_file to the beginning for reading

        // DONE: Read pages from your data file (using read_from_data_file) and search for the employee ID in those pages. Be mindful of the page limit in main memory.        
        int page_number = 0;
        while(buffer[page_number].read_from_data_file(data_file)){
            for(const auto& record : buffer[page_number].records){
                if(record.id == searchId){
                    record.print(); // Print the record if found
                    return;
                }
            }
            page_number++;
            if(page_number >= buffer.size()){
                // Reached the page limit in main memory
                page_number = 0; // Reset to first page
                for (page& p : buffer) {
                    p.clear(); // Clear the pages for re-use
                }
            }
        }
        // DONE: Print "Record not found" if no records match.
        cout << "Record not found" << endl;
    }
};
