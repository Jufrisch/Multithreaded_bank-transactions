# Multithreaded_bank-transactions
Using C code to create a multithreaded program that utilizes synchronization APIs for processing bank transaction requests concurrently.


1)  Run make in the directory containing files:
        eft.c
        input_transfers.txt
        Makefile
2) In ubuntu or other linux shell and while in the same directory run command:

         ./eft < input_transfers.txt 10

    Where the last argument can be changed to the number of threads desired
    to run concurrently while processing transactions.
