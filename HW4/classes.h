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
    int id, manager_id; // Employee ID and their manager's ID
    string bio, name; // Fixed length string to store employee name and biography

    Record(vector<string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }
	
	// Function to get the size of the record
    int get_size() {
        // sizeof(int) is for name/bio size() in serialize function
        return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size(); 
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
    vector<Record> records; // Data_Area containing the records
    vector<pair<int, int>> slot_directory; // Slot directory containing offset and size of each record
    int cur_size = sizeof(int); // Current size of the page including the overflow page pointer. if you also write the length of slot directory change it accordingly.
    int overflowPointerIndex;  // Initially set to -1, indicating the page has no overflow page. 
							   // Update it to the position of the overflow page when one is created.

    // Constructor
    Page() : overflowPointerIndex(-1) {}

    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > 4096) { // Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            records.push_back(r);
            cur_size += record_size + slot_size;
			// TODO: update slot directory information
            return true;
        }
    }

    // Function to write the page to a binary output stream. You may use
    void write_into_data_file(ostream &out) const {
        char page_data[4096] = {0}; // Buffer to hold page data
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

        int bottom_offset = 4096 - sizeof(int); // Start from the bottom of the page for slot directory and overflow pointer
            
        memcpy(page_data + bottom_offset, &overflowPointerIndex, sizeof(overflowPointerIndex)); // Write overflowPointerIndex at the end of the page

        for (size_t i = slot_directory.size(); i >= 0; i--)
        {
            bottom_offset -= sizeof(int);
            //we use long long here because we 8 bytes for the id and manger id
            memcpy(page_data + bottom_offset, &slot_directory[i].second, sizeof(int))); // Write record size
            bottom_offset -= sizeof(int);
            memcpy(page_data + bottom_offset, &slot_directory[i].first, sizeof(int)); // Write record offset        
        }
        
        
        // Write the page_data buffer to the output stream
        out.write(page_data, sizeof(page_data));
    }

    // Function to read a page from a binary input stream
    bool read_from_data_file(istream &in) {
        char page_data[4096] = {0}; // Buffer to hold page data
        in.read(page_data, 4096); // Read data from input stream

        streamsize bytes_read = in.gcount();
        if (bytes_read == 4096) {
            // TODO: Done
            // Process data to fill the records, slot_directory, and overflowPointerIndex
            
            int bottom_offset = 4096 - sizeof(int); // Start from the bottom of the page to read overflowPointerIndex and slot directory
            
            memcpy(&overflowPointerIndex, page_data + bottom_offset, sizeof(int)); // Read overflowPointerIndex from the bottom of the page

            slot_directory.clear();
            records.clear();    


            bottom_offset -= sizeof(int); // Move up to read slot directory entries


            vector<pair<int, int>> temp_slots; // Temporary vector to hold slot directory entries while reading
            while (bottom_offset >= 0) {
                int record_size, record_offset;

                memcpy(&record_size, page_data + bottom_offset, sizeof(int)); // Read record size
                memcpy(&record_offset, page_data + bottom_offset + sizeof(int), sizeof(int)); // Read record offset

                if (record_size >= 0 && record_offset > 0 && record_offset + record_size <= 4096) {
                    temp_slots.push_back({record_offset, record_size}); // Store slot directory entry
                    bottom_offset -= sizeof(int) * 2; // Move up for the next slot directory entry
                }
                else {
                    break; // No more valid slot directory entries
                }
            }

            for (int i = temp_slots.size() - 1; i >= 0; i--) {
                slot_directory.push_back(temp_slots[i]); // Add valid slot directory entries to the page's slot_directory
            }


            for (const auto &slots: slot_directory) {
                int record_offset = slots.first;
                int record_size = slots.second;

                long long id, manager_id;
                memcpy(&id, page_data + record_offset, sizeof(long long)); // Read ID
                record_offset += sizeof(long long); // Move offset to read manager ID
                memcpy(&manager_id, page_data + record_offset, sizeof(long long)); // Read manager ID
                record_offset += sizeof(long long); // Move offset to read name length



                int name_len, bio_len;
                memcpy(&name_len, page_data + record_offset, sizeof(int)); // Read name length
                record_offset += sizeof(int); // Move offset to read name

                string name(page_data + record_offset, name_len); // Read name
                record_offset += name_len * sizeof(char); // Move offset to read bio length

                =memcpy(&bio_len, page_data + record_offset, sizeof(int)); // Read bio length
                record_offset += sizeof(int); // Move offset to read bio

                string bio(page_data + record_offset, bio_len); // Read bio

                vector<string> fields = {to_string(id), name, bio, to_string(manager_id)}; // Create fields vector
                Record record(fields); // Construct Record object
                records.push_back(r); // Add record to the page's records vector
            }

            cur_size = sizeof(int);
            for (auto &record: records) {
                cur_size += record.get_size() + sizeof(int) * 2; // Update current size of the page
            }

            return true;
        }

        if (bytes_read > 0) {
            cerr << "Incomplete read: Expected 4096 bytes, but only read " << bytes_read << " bytes." << endl;
        }

        return false;
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
    int compute_hash_value(int id) {
        int hash_value;

        // TODO:DONE Implement the hash function h = id mod 2^8
        hash_value = id % 256;
        return hash_value;
    }

    // Function to add a new record to an existing page in the index file
    void addRecordToIndex(int pageIndex, Page &page, Record &record) {
        // Open index file in binary mode for updating
        fstream indexFile(fileName, ios::binary | ios::in | ios::out);

        if (!indexFile) {
            cerr << "Error: Unable to open index file for adding record." << endl;
            return;
        }
		
		// TODO:DONE
        //  - Use seekp() to seek to the offset of the correct page in the index file
		//		indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
        indexFile.seekp(pageIndex * Page_SIZE, ios::beg);
		//  - try insert_record_into_page()
		//     - if it fails, then you'll need to either...
		//			- go to next overflow page and try inserting there (keep doing this until you find a spot for the record)
		//			- create an overflow page (if page.overflowPointerIndex == -1) using nextFreePage. update nextFreePage index and pageIndex.
        if (!page.insert_record_into_page(record)) {
            // If the record cannot be inserted into the current page, check for overflow page
            if (page.overflowPointerIndex != -1) {
                // There is an overflow page, try inserting there
                addRecordToIndex(page.overflowPointerIndex, page, record);
            } else {
                // No overflow page, create a new one
                Page overflowPage = Page(); // Create a new overflow page
                overflowPage.insert_record_into_page(record); // Insert the record into the new overflow page
                page.overflowPointerIndex = nextFreePage; // Update the overflow pointer to point to the new page
                nextFreePage++; // Increment nextFreePage for the next available page index

                // Write the new overflow page to the index file
                indexFile.seekp(page.overflowPointerIndex * Page_SIZE, ios::beg); // Seek to the appropriate position in the index file
                overflowPage.write_into_data_file(indexFile); // After inserting the record, write the modified page back to the index file.
            }
        } else {
            // If the record was successfully inserted into the current page, write it back to the index file
            indexFile.seekp(pageIndex * Page_SIZE, ios::beg); // Seek to the appropriate position in the index file
            // TODO:DONE After inserting the record, write the modified page back to the index file. 
            page.write_into_data_file(indexFile);
        }

        // Close the index file
        indexFile.close();
    }

    // Function to search for a record by ID in a given page of the index file
    void searchRecordByIdInPage(int pageIndex, int id) {
        // Open index file in binary mode for reading
        ifstream indexFile(fileName, ios::binary | ios::in);

        if(!indexFile) {
            cerr << "Error: Unable to open index file for searching record." << endl;
            return;
        }

        // TODO:
        //  - Search for the record by ID in the page
        //  - Check for overflow pages and report if record with given ID is not found

        // Seek to the appropriate position in the index file
        indexFile.seekg(pageIndex * Page_SIZE, ios::beg);

        // Read the page from the index file
        Page page;
        bool page_read = page.read_from_data_file(indexFile);

        if (!page_read) {
            cerr << "Record with ID " << id << " not found in the index." << endl;
            indexFile.close();
            return;
        }
        
        bool found = false;
        for (auto &record: page.records) {
            if (record.id == id) {
                cout << "Record found in page index " << pageIndex << ":" << endl;
                record.print();
                found = true;
                break;
            }
        }

        if (!found && page.overflowPointerIndex != -1) {
            // If the record was not found in the current page, check the overflow page
            searchRecordByIdInPage(page.overflowPointerIndex, id);
        } else if (!found) {
            cout << "Record with ID " << id << " not found in the index." << endl;
        }  
        indexFile.close();
    }

public:
    HashIndex(string indexFileName) : nextFreePage(0), fileName(indexFileName) {
    }

    // Function to create hash index from Employee CSV file
    void createFromFile(string csvFileName) {
        // Read CSV file and add records to index
        // Open the CSV file for reading
        ifstream csvFile(csvFileName);

        string line;
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

            // TODO:
            //   - Compute hash value for the record's ID using compute_hash_value() function.
            //   - Get the page index from PageDirectory. If it's not in PageDirectory, define a new page using nextFreePage.
            //   - Insert the record into the appropriate page in the index file using addRecordToIndex() function.


        }

        // Close the CSV file
        csvFile.close();
    }

    // Function to search for a record by ID in the hash index
    void findAndPrintEmployee(int id) {
        // Open index file in binary mode for reading
        ifstream indexFile(fileName, ios::binary | ios::in);

        // TODO: DONE
        //  - Compute hash value for the given ID using compute_hash_value() function
        //  - Search for the record in the page corresponding to the hash value using searchRecordByIdInPage() function

        int hash_value = compute_hash_value(id);
        if (PageDirectory[hash_value] != -1) {
            searchRecordByIdInPage(PageDirectory[hash_value], id);
        } else {
            cout << "Record with ID " << id << " not found in the index." << endl;
        }

        // Close the index file
        indexFile.close();
    }
};

