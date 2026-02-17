/******************************************************************************
 * AUTHORS : Samuel Garcia Lopez, Graham Glazner
 * ONIDS   : garcsamu, glaznerg
 * EMAILS  : garcsamu@oregonstate.edu, glazner@oregonstate.edu
 ******************************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cmath>

using namespace std;

class Record {
public:
    //long long is 8 bytes so we use that 
    long long id, manager_id; // Employee ID and their manager's ID
    string bio, name; // Fixed length string to store employee name and biography

    Record(vector<string> &fields) {
        id = stoll(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoll(fields[3]);
    }
	
	// Function to get the size of the record
    int get_size() const {
        // sizeof(int) is for name/bio size() in serialize function
        return (int)(sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size()); 
    }

    // Function to serialize the record for writing to file
    string serialize() const {
        ostringstream oss;
        oss.write(reinterpret_cast<const char *>(&id), sizeof(id));
        oss.write(reinterpret_cast<const char *>(&manager_id), sizeof(manager_id));
        int name_len = name.size();
        int bio_len = bio.size();
        oss.write(reinterpret_cast<const char *>(&name_len), sizeof(name_len));
        oss.write(name.c_str(), name.size());
        oss.write(reinterpret_cast<const char *>(&bio_len), sizeof(bio_len));
        oss.write(bio.c_str(), bio.size());
        return oss.str();
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};

class Page {
public:
    static const int PAGE_SIZE = 4096; // Size of each page in bytes
    
    vector<Record> records; // Data_Area containing the records
    vector<pair<int, int>> slot_directory; // Slot directory containing offset and size of each record
    int cur_size = sizeof(int); // Current size of the page including the overflow page pointer. if you also write the length of slot directory change it accordingly.
    int overflowPointerIndex;  // Initially set to -1, indicating the page has no overflow page. 
							   // Update it to the position of the overflow page when one is created.

    // Constructor 
    Page() : cur_size(sizeof(int)), overflowPointerIndex(-1) {}
    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > PAGE_SIZE) { // Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            records.push_back(r);
            
            int offset = 0;
            for (const auto &record: records) {
              offset += record.get_size();
            }
            slot_directory.push_back({offset, record_size}); // Add offset and size to slot directory
            cur_size += record_size + slot_size;
			// TODO: DONE
            //update slot directory information
            return true;
        }
    }

    // Function to write the page to a binary output stream. You may use
    void write_into_data_file(ostream &out) const {
        char page_data[PAGE_SIZE] = {0}; // Buffer to hold page data
        int offset = 0;

        // Write records into page_data buffer
        for (const auto &record: records) {
            string serialized = record.serialize();
            memcpy(page_data + offset, serialized.c_str(), serialized.size());
            offset += serialized.size();
        }

        // TODO: DONE.
        //  - Write slot_directory in reverse order into page_data buffer.
        //  - Write overflowPointerIndex into page_data buffer.
        //  You should write the first entry of the slot_directory, 
        //  which have the info about the first record at the bottom of the page, before overflowPointerIndex.

        int bottom_offset = PAGE_SIZE - sizeof(int); // Start from the bottom of the page for slot directory and overflow pointer
            
        memcpy(page_data + bottom_offset, &overflowPointerIndex, sizeof(int)); // Write overflowPointerIndex at the end of the page

        for (int i = slot_directory.size() - 1; i >= 0; i--)
        {
            bottom_offset -= sizeof(int);
            memcpy(page_data + bottom_offset, &slot_directory[i].second, sizeof(int)); // Write record size
            bottom_offset -= sizeof(int);
            memcpy(page_data + bottom_offset, &slot_directory[i].first, sizeof(int)); // Write record offset        
        }
        
        
        // Write the page_data buffer to the output stream
        out.write(page_data, sizeof(page_data));
    }

    // Function to read a page from a binary input stream
    bool read_from_data_file(istream &in) {
        char page_data[PAGE_SIZE] = {0}; // Buffer to hold page data
        in.read(page_data, PAGE_SIZE); // Read data from input stream

        streamsize bytes_read = in.gcount();
        if (bytes_read != PAGE_SIZE) {
            // TODO: Done
            // Process data to fill the records, slot_directory, and overflowPointerIndex
            if (bytes_read > 0) 
                cerr << "incomplete read: " << in.gcount() << "bytes\n";
            return false;
        }

        records.clear();
        slot_directory.clear();

        int bottom_offset = PAGE_SIZE - (int)(sizeof(int)); // Start from the bottom of the page to read overflowPointerIndex and slot directory
        memcpy(&overflowPointerIndex, page_data + bottom_offset, sizeof(int)); // Read overflowPointerIndex from the bottom of the page

        vector<pair<int, int>> temp_slots; // Temporary vector to hold slot directory entries while reading
        while (bottom_offset >= (int)(sizeof(int) * 2)) {
            bottom_offset -= (int)(sizeof(int) * 2); // Move to the start of the next slot directory entry
            int record_offset, record_size;

            memcpy(&record_offset, page_data + bottom_offset, sizeof(int)); // Read record offset
            memcpy(&record_size, page_data + bottom_offset + sizeof(int), sizeof(int)); // Read record size


            if (record_size <= 0 || record_offset <= 0 || record_offset > PAGE_SIZE || record_size > record_offset) {
                break; // No more valid slot directory entries
            }
            temp_slots.push_back({record_offset, record_size}); // Store slot directory entry
        }

        for (int i = (int)temp_slots.size() - 1; i >= 0; --i) {
            slot_directory.push_back(temp_slots[i]); // Add valid slot directory entries to the page's slot_directory
        }


        for (const auto &slots: slot_directory) {
            int record_end_offset = slots.first;
            int record_size = slots.second;
            int record_offset = record_end_offset - record_size; // Calculate start offset from end offset

            long long id, manager_id;
            memcpy(&id, page_data + record_offset, sizeof(long long)); // Read ID
            record_offset += sizeof(long long); // Move offset to read manager ID
            memcpy(&manager_id, page_data + record_offset, sizeof(long long)); // Read manager ID
            record_offset += sizeof(long long); // Move offset to read name length


            int name_len, bio_len;
            memcpy(&name_len, page_data + record_offset, sizeof(int)); // Read name length
            record_offset += sizeof(int); // Move offset to read name

            string name(page_data + record_offset, name_len); // Read name
            record_offset += name_len; // Move offset to read bio length

            memcpy(&bio_len, page_data + record_offset, sizeof(int)); // Read bio length
            record_offset += sizeof(int); // Move offset to read bio

            string bio(page_data + record_offset, bio_len); // Read bio

            vector<string> fields = {to_string(id), name, bio, to_string(manager_id)}; // Create fields vector
            records.emplace_back(fields);
        }
            cur_size = (int)(sizeof(int));
            for (const auto &record: records)
                cur_size += record.get_size() + (int)(sizeof(int) * 2); // Update current size of the page

            return true;
        }   
    };

class HashIndex {
private:
    const size_t maxCacheSize = 1; // Maximum number of pages in the buffer
    const int Page_SIZE = 4096; // Size of each page in bytes
    vector<int> PageDirectory; // Map h(id) to a bucket location in EmployeeIndex(e.g., the jth bucket)
    // can scan to correct bucket using j*Page_SIZE as offset (using seek function)
    // can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int nextFreePage; // Next place to write a bucket
    string fileName;

    // Function to compute hash value for a given ID
    int compute_hash_value(long long id) {
        int hash_value;

        // TODO:DONE Implement the hash function h = id mod 2^8
        hash_value = (int)(id % 256);
        return hash_value;
    }

    // Function to add a new record to an existing page in the index file
    void addRecordToIndex(int pageIndex, Record &record) {
        // Open index file in binary mode for updating
        fstream indexFile(fileName, ios::binary | ios::in | ios::out);

        if (!indexFile) {
            cerr << "Error: Unable to open index file for adding record." << endl;
            return;
        }
		
		// TODO:DONE
        //  - Use seekp() to seek to the offset of the correct page in the index file
		//		indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
        indexFile.seekg(pageIndex * Page_SIZE, ios::beg);
        Page page;
        bool ok = page.read_from_data_file(indexFile);
        if (!ok) {
            cerr << "Error: Failed to read page at index " << pageIndex << endl;
            return;
        }
        indexFile.close();

        if (page.insert_record_into_page(record)) {
            // write updated page back to index file
            fstream indexFileWrite(fileName, ios::binary | ios::in | ios::out);
            indexFileWrite.seekp(pageIndex * Page_SIZE, ios::beg);
            page.write_into_data_file(indexFileWrite);
            indexFileWrite.close();
        } 
        else 
        {
            if (page.overflowPointerIndex != -1) {
                addRecordToIndex(page.overflowPointerIndex, record);
            }
            else{
                int overflow_index = nextFreePage++;
                page.overflowPointerIndex = overflow_index;

                Page overflow_page;
                overflow_page.insert_record_into_page(record);

                fstream indexFileWrite(fileName, ios::binary | ios::in | ios::out);
                indexFileWrite.seekp(overflow_index * Page_SIZE, ios::beg);
                overflow_page.write_into_data_file(indexFileWrite);

                indexFileWrite.seekp(pageIndex * Page_SIZE, ios::beg);
                page.write_into_data_file(indexFileWrite);
                indexFileWrite.close();
            }
        }
    }

    // Function to search for a record by ID in a given page of the index file
    void searchRecordByIdInPage(int pageIndex, int id) {
        // Open index file in binary mode for reading

        cout << "Searching for record with ID " << id << " in page index " << pageIndex << "..." << endl;
        ifstream indexFile(fileName, ios::binary | ios::in);

        if(!indexFile) {
            cerr << "Error: Unable to open index file for searching record." << endl;
            return;
        }

        // TODO: DONE
        //  - Search for the record by ID in the page
        //  - Check for overflow pages and report if record with given ID is not found

        // Seek to the appropriate position in the index file
        indexFile.seekg(pageIndex * Page_SIZE, ios::beg);
        cout << "  [DEBUG] Seeking to byte offset: " << (pageIndex * Page_SIZE) << endl;

        // Read the page from the index file
        Page page;
        bool page_read = page.read_from_data_file(indexFile);

        if (!page_read) {
            cerr << "  [ERROR] Failed to read page at index " << pageIndex << endl;
            return;
        }

        for (auto &record: page.records) {
            if (record.id == id) {
                cout << "Record found in page index " << pageIndex << ":" << endl;
                record.print();
                return;
            }
        }

        if (page.overflowPointerIndex != -1) {
            cout << "Record with ID " << id << " not found in page index " << pageIndex << ". Checking overflow page at index " << page.overflowPointerIndex << "..." << endl;
            searchRecordByIdInPage(page.overflowPointerIndex, id);
        } else {
            cout << "Record with ID " << id << " not found in the index." << endl;
        }
    }

public:
    HashIndex(string indexFileName) : nextFreePage(0), fileName(indexFileName), PageDirectory(256, -1) {
    }

    // Function to create hash index from Employee CSV file
    void createFromFile(string csvFileName) {
        //Truncate and create the index file if it already exists, otherwise create a new file
        {
            ofstream indexFile(fileName, ios::binary | ios::out | ios::trunc);
            if (!indexFile) {
                cerr << "Error: Unable to create index file." << endl;
                return;
            }
        }


        // Read CSV file and add records to index
        // Open the CSV file for reading
        ifstream csvFile(csvFileName);
        if (!csvFile.is_open()) {
            cerr << "Error: Unable to open CSV file." << endl;
            return;
        }

        string line;
        int recordCount = 0;
        // Read each line from the CSV file
        while (getline(csvFile, line)) {
            // Parse the line and create a Record object
            stringstream ss(line);
            string item;
            vector<string> fields;
            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record record(fields);

            // TODO:DONE
            //   - Compute hash value for the record's ID using compute_hash_value() function.
            //   - Get the page index from PageDirectory. If it's not in PageDirectory, define a new page using nextFreePage.
            //   - Insert the record into the appropriate page in the index file using addRecordToIndex() function.
            int hash_value = compute_hash_value(record.id);

            //   - Check if the page index is already in PageDirectory. If not, initialize it with nextFreePage and increment nextFreePage.
            if (PageDirectory[hash_value] == -1) {
                int pageIndex = nextFreePage++;
                PageDirectory[hash_value] = pageIndex;
                
                Page newPage;
                newPage.insert_record_into_page(record);

                // Only truncate on first write, otherwise append
                fstream indexFileWrite(fileName, ios::binary | ios::in | ios::out);
                indexFileWrite.seekp(pageIndex * Page_SIZE, ios::beg);
                newPage.write_into_data_file(indexFileWrite);
                indexFileWrite.close();
            }
            else {
                    addRecordToIndex(PageDirectory[hash_value], record);
                }
                recordCount++;
            }
        cout << "Index creation from CSV file completed." << endl;
        cout << "Total pages used: " << nextFreePage << endl;
        cout << "Total records processed: " << recordCount << endl;
        // Close the CSV file
        csvFile.close();
    }

    // Function to search for a record by ID in the hash index
    void findAndPrintEmployee(int id) {
        // Open index file in binary mode for reading
        int h = compute_hash_value(id);
        if (PageDirectory[h] == -1) {
            cout << "Record with ID " << id << " not found.\n";
            return;
        }
        searchRecordByIdInPage(PageDirectory[h], id);
    }
};