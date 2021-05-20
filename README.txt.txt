Steps to run the code:

> Go to the respective directory
> Enter the 1st command: g++ code.cpp
> Enter the 2nd command: ./a

Assumptions:

> While performing rehashing, I am removing the overflow chain (if it exists) from the existing bucket.
> I am taking the transaction id itself as the hash value and then extracting the hash prefix. In simple words, my hash functions is h(transaction_id)=transaction_id.
