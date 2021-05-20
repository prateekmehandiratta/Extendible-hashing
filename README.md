# Extendible-hashing
Implementation of extendible hashing to store the dataset. Dataset is the sales record of a department store. Dataset is being given as a input in the file where each line in 
the file represents a record and contains the four fields: transaction id(unique integer), transaction sale amount(integer), customer name(string of size 3), and item category(integer). Hashing is done on the transaction id. Ideally, the buckets in the extendible hash should be stored in the secondary memory. However, for the purpose of
this project, they are stored in “Simulated Secondary Memory". The directory or bucket address table of the extendible hash contain the hash prefix and pointer to the
bucket sitting in “Simulated Secondary Memory".

### Simulated Secondary Memory
The secondary memory is simulated through an array of the abstract data-type “bucket”. It is a very large size array. The bucket capacity is fixed in terms of number of records it can contain and is being given as an user-input. Locations (indices) in this array form our “bucket address / hardware address”.

### Bucket abstract data type
It contains the following information:
1. Number of empty spaces.
2. An array of structures to store the records. Length of this array is fixed according to the parameter “bucket-size” specified.
3. Link to the next bucket (valid only if this bucket is overflowing).

### Main Memory
1. Enough Main Memory for “bringing” in the buckets to be rehashed.
2. “Main memory” can hold upto 1024 directory entries. The rest resides in “Simulated Secondary memory”.
3. Directory entries overflowing into secondary memory is using the same bucket abstract data type which was previously declared in item “Simulated Secondary Memory”. 

### Other Details
1. The most significant bits are extracted to find the directory entry.
2. Only one directory expansion is done per record insertion. Following the directory expansion, if the collision still persists, it is being resolved by increasing the local depth(if local depth < global depth). In case the collision is still not resolved, an overflow bucket is being created.
3. The number of records and the number of directory entries which can be stored in the bucket can be different. Both the values are being given as user-input.
4. Extendible hash has a separate insert function which would insert any given arbitrary “index record” into the extendible hash.
5. Extendible hash has a seperate intuitive print function which would print the whole print all the records in the bucket with their hash prefix.

To know about how to execute the code, have a look at README.txt file.

