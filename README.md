# project-2
Word Frequency Project

Janhavi Narain jn579

Priya Patel prp78

Testing Process: We began our testing in stages. First, one person developed the directory queues while the other person developed the computation process via linked lists of the words in a file and files being tested. Each method was tested separately with the test case shown on the pdf (file1: hi there hi there, file2: hi hi out there) before including a bunch of other examples (since it's supposed to be in lexicographic order, we included words that would be before hi, after there, etc.). Once the separate files would work, we combined them and tested the threads. We started with one directory, file, and analysis thread. After that worked, we tried with two of one type of thread and one of the other types of threads. Then, we tested with more than one of each thread. Once we were certain the threads were working, we tested with more than two files, including directories with files. While reading through the piazza, we also came up with a bunch of other test cases, including cases where we had more threads than comparison, cases where the files were empty, and cases where the file only had one word. Once all of these cases worked, we created the make file and tested our code with valgrind. After making some more edits to fix any memory errors, our code was complete.
