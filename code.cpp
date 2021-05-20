#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>
#include <vector>
using namespace std;

class record{
    public:
    int transaction_id;
    int transaction_amnt;
    string customer_name;
    int item_category;
    record()
    {

    }
    record(int id,int amnt,string name,int category)
    {
        transaction_id=id;
        transaction_amnt=amnt;
        customer_name=name;
        item_category=category;
    }
};

class directory_entry{
    public:
    int hash_prefix;
    int bucket_pointer;
    directory_entry()
    {
        hash_prefix=-1;
        bucket_pointer=-1;
    }
    directory_entry(int prefix,int address)
    {
        hash_prefix=prefix;
        bucket_pointer=address;
    }
};

class bucket{
    public:
    bool flag;
    int empty_spaces;
    int next_bucket_link;
    int local_depth;
    void* storage;
    bucket()
    {
        flag=true;
        local_depth=0;
        next_bucket_link=-1;
    }
    bucket(bool type,int bucket_size,int depth)
    {
        if(type==true)      //record
        {
            flag=true;
            storage = new record[bucket_size];
        }
        else                //directory_entry
        {
            flag=false;
            storage = new directory_entry[bucket_size];
        }
        empty_spaces=bucket_size;
        next_bucket_link=-1;
        local_depth=depth;
    }
};

bucket SSM[10000000];       //simulated secondary memory;
directory_entry table[1024];    //Main memory
int available_bucket_index = 0;
int available_overflow_bucket_index = 5000000;
int global_depth=0;
int data_bucket_size,directory_bucket_size;
int bits_to_take;
int directory_bucket_start = -1;

int find_bucket_pointer(int hash_prefix)
{
    if(hash_prefix<1024)
        return table[hash_prefix].bucket_pointer;
    int forward = hash_prefix-1024;
    int counter = forward/directory_bucket_size;
    int DBA = directory_bucket_start;       //Directory Bucket Address
    for(int i=0;i<counter;i++)
        DBA = SSM[DBA].next_bucket_link;
    forward = forward % directory_bucket_size;
    return ((directory_entry*)(SSM[DBA].storage))[forward].bucket_pointer;
}

int find_directory_bucket(int hash_prefix)
{
    int counter = (hash_prefix-1024)/directory_bucket_size;
    if(counter==0)
    {
        if(directory_bucket_start==-1){
            bucket b0(false,directory_bucket_size,0);
            SSM[available_overflow_bucket_index++] = b0;
            directory_bucket_start = available_overflow_bucket_index-1;
        }
        return directory_bucket_start;
    }
    int DBA = directory_bucket_start;
    for(int i=0;i<(counter-1);i++)
        DBA = SSM[DBA].next_bucket_link;
    if(SSM[DBA].next_bucket_link==-1)
    {
        bucket b0(false,directory_bucket_size,0);
        SSM[available_overflow_bucket_index++] = b0;
        SSM[DBA].next_bucket_link = available_overflow_bucket_index-1;
    }
    return SSM[DBA].next_bucket_link;
}

void insert(record rec,int expansion_counter)
{
    int trans_id = rec.transaction_id;
    int hash_prefix = trans_id >> (bits_to_take-global_depth);
    int bucket_address = find_bucket_pointer(hash_prefix);
    int BA = bucket_address;
    while(SSM[BA].empty_spaces!=0 || SSM[BA].next_bucket_link!=-1)
    {
        if(SSM[BA].empty_spaces!=0)
        {
            int es = SSM[BA].empty_spaces;
            ((record*)(SSM[BA].storage))[data_bucket_size-es] = rec;
            SSM[BA].empty_spaces--;
            return;
        }
        BA = SSM[BA].next_bucket_link;
    }
    
    /*SITUATION OF COLLISION*/
    bool is_collision_resolved=false;
    while(!is_collision_resolved)
    {
        if(global_depth==SSM[bucket_address].local_depth)
        {
            if(expansion_counter==0)    //directory expansion
            {
                int num_entries = pow(2,global_depth);
                for(int i=num_entries;i<num_entries*2;i++)
                {
                    int bp = find_bucket_pointer(i-num_entries);
                    directory_entry d0(i,bp);
                    if(i<1024)
                        table[i] = d0;
                    else
                    {
                        int DBA = find_directory_bucket(i);     //Directory Bucket Address
                        int es = SSM[DBA].empty_spaces;
                        ((directory_entry*)(SSM[DBA].storage))[directory_bucket_size-es] = d0;
                        SSM[DBA].empty_spaces--;
                    }
                }
                for(int i=0;i<num_entries*2;i++)
                {
                    int j = i/2;
                    if(i<1024)
                        table[i].bucket_pointer = find_bucket_pointer(j+num_entries);
                    else
                    {
                        int DBA = find_directory_bucket(i);
                        int index = (i-1024)%directory_bucket_size;
                        int bp = find_bucket_pointer(j+num_entries);
                        directory_entry d0(i,bp);
                        ((directory_entry*)(SSM[DBA].storage))[index] = d0;
                    }
                }
                global_depth++;
                expansion_counter++;
            }
            else        // add overflow bucket
            {
                bucket b0(true,data_bucket_size,global_depth);
                SSM[available_overflow_bucket_index++] = b0;
                int es = SSM[available_overflow_bucket_index-1].empty_spaces;
                ((record*)(SSM[available_overflow_bucket_index-1].storage))[data_bucket_size-es] = rec;
                SSM[available_overflow_bucket_index-1].empty_spaces--;
                is_collision_resolved=true;
                BA = bucket_address;
                while(SSM[BA].next_bucket_link!=-1)
                    BA = SSM[BA].next_bucket_link;
                SSM[BA].next_bucket_link = available_overflow_bucket_index-1;
            }
        }

        else        //rehashing
        {
            int diff = global_depth - SSM[bucket_address].local_depth;
            int total_count = pow(2,diff);
            hash_prefix = trans_id >> (bits_to_take-global_depth);
            int temp_prefix=hash_prefix;
            while(temp_prefix>=0 && find_bucket_pointer(temp_prefix)==bucket_address)
                temp_prefix--;
            temp_prefix+=((total_count/2)+1);
            SSM[bucket_address].local_depth++;
            bucket b0(true,data_bucket_size,SSM[bucket_address].local_depth);
            SSM[available_bucket_index++] = b0;
            for(int i=0;i<(total_count/2);i++)
            {
                if(temp_prefix<1024)
                    table[temp_prefix].bucket_pointer=(available_bucket_index-1);
                else{
                    int DBA = find_directory_bucket(temp_prefix);
                    int index = (temp_prefix-1024)%directory_bucket_size;
                    int bp = available_bucket_index-1;
                    directory_entry d0(temp_prefix,bp);
                    ((directory_entry*)(SSM[DBA].storage))[index] = d0;
                }
                temp_prefix++;
            }

            vector<record> rehashing_storage;
            BA = bucket_address;
            while(BA!=-1)
            {
                for(int i=0;i<data_bucket_size-SSM[BA].empty_spaces;i++)
                    rehashing_storage.push_back( ((record*)(SSM[BA].storage))[i] );
                SSM[BA].empty_spaces=data_bucket_size;
                BA = SSM[BA].next_bucket_link;
            }
            rehashing_storage.push_back(rec);
            SSM[bucket_address].next_bucket_link=-1;

            for(int i=0;i<rehashing_storage.size();i++)
                insert(rehashing_storage[i],expansion_counter);
            is_collision_resolved=true;
        }
    }
}

int main()
{
    /* Creation of Dataset */
    ofstream file_pointer("data.txt");
    srand(time(0));
    int transaction_id=1;
    char customer_name[3];
    for(int i=1;i<=200;i++)
    {
        int transaction_amnt = (rand()%500000)+1;
        for(int j=0;j<3;j++)
        {
            int ascii = (rand()%26);
            ascii+=97;
            char ch = ascii;
            customer_name[j]=ch;
        }
        int item_category = (rand()%1500)+1;
        file_pointer << transaction_id << " " << transaction_amnt << " " << customer_name[0] << customer_name[1] << customer_name[2] << " " << item_category << endl;
        transaction_id++;
    }
    file_pointer.close();

    /* Read the Dataset */
    cout << "Enter the Dataset file name: ";
    string file_name;
    cin >> file_name;
    cout << "Enter the data bucket size: ";
    cin >> data_bucket_size;
    cout << "Enter the directory bucket size: ";
    cin >> directory_bucket_size;
    cout << "Enter bits: ";
    cin >> bits_to_take;

    bucket b0(true,data_bucket_size,0);     //first record bucket created
    SSM[available_bucket_index++] = b0;

    directory_entry d0(0,0);            //first directory entry created, hash prefix = 0
    table[0] = d0;

    ifstream file_pointer1(file_name);
    int id,amnt,category;
    string name;
    while(1)
    {
        file_pointer1 >> id;
        if(file_pointer1.eof())
            break;
        file_pointer1 >> amnt >> name >> category;
        record rec(id,amnt,name,category);
        insert(rec,0);
    }
    file_pointer1.close();

    cout << "All Records are inserted into the extendible hash" << endl;

    bool flag=true;
    while(flag)
    {
        int action;
        cout << "Enter 1 to print the records with the hash prefix" << endl;
        cout << "Enter 2 to print the overflow directory buckets" << endl;
        cout << "Enter 3 to exit" << endl;
        cin >> action;
        switch(action)
        {
            case 1:{
                /*Print the Extendible hashing*/
                cout << "\n\n";
                int total = pow(2,bits_to_take);
                int printed_bucket=-1,printed_prefix=-1;
                for(int i=0;i<1024;i++)
                {
                    if(table[i].bucket_pointer!=-1)
                    {
                        int BA = table[i].bucket_pointer;
                        if(BA==printed_bucket)
                        {
                            cout << "prefix " << i << " : " << printed_prefix << endl;
                            cout << "\n";
                            continue;
                        }
                        cout << "prefix " << i << endl;
                        printed_bucket=BA;
                        printed_prefix=i;
                        while(BA!=-1)
                        {
                            for(int j=0;j<(data_bucket_size-SSM[BA].empty_spaces);j++)
                            {
                                record rc = ((record*)(SSM[BA].storage))[j];
                                cout << rc.transaction_id << " " << rc.transaction_amnt << " " << rc.customer_name << " " << rc.item_category << endl;
                            }
                            BA = SSM[BA].next_bucket_link;
                            cout << "\n" << endl;
                        }
                        cout << "\n";
                    }
                }
                int DBA = directory_bucket_start;
                while(DBA!=-1)
                {
                    int es = SSM[DBA].empty_spaces;
                    for(int i=0;i<directory_bucket_size-es;i++)
                    {
                        directory_entry d0 = ((directory_entry*)(SSM[DBA].storage))[i];
                        int BA = d0.bucket_pointer;
                        cout << "prefix " << d0.hash_prefix << endl;
                        while(BA!=-1)
                        {
                            for(int j=0;j<(data_bucket_size-SSM[BA].empty_spaces);j++)
                            {
                                record rc = ((record*)(SSM[BA].storage))[j];
                                cout << rc.transaction_id << " " << rc.transaction_amnt << " " << rc.customer_name << " " << rc.item_category << endl;
                            }
                            BA = SSM[BA].next_bucket_link;
                            cout << "\n" << endl;
                        }
                        cout << "\n";
                    }
                    DBA = SSM[DBA].next_bucket_link;
                }
                break;
            }
            case 2: {
                int DBA = directory_bucket_start;
                while(DBA!=-1)
                {
                    cout << "DBA " << DBA << endl;
                    for(int i=0;i<directory_bucket_size-SSM[DBA].empty_spaces;i++)
                    {
                        directory_entry d0 = ((directory_entry*)(SSM[DBA].storage))[i];
                        cout << d0.hash_prefix << " " << d0.bucket_pointer << endl;
                    }
                    cout << "\n";
                    DBA = SSM[DBA].next_bucket_link;
                }
                break;
            }
            default: {flag=false;}
        }
    }
    return 0;
}