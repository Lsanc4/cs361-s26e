## Homework 10: hands-on performance diagnostics

In this assignment, you are provided the source code, and a couple of 
binary object files, for a log analysis tool. 

It works, but it's not as fast as we would like.

### Step 1: Analyze 10k.txt

Build the analyzer with `make`, then run `./log_analyzer 10k.txt`. You should get some output indicating the number of events per hour, and the severity distribution. 

Use the `time` tool, to see what's going on. What do the three lines mean? Make sure you understand the significance of each measurement here.

Then, use `strace` to see what we're doing. The `-k` parameter gives you a backtrace for each system call, and the `-e` parameter lets you filter on a specific system call name. Find, and eliminate the problem that's taking several seconds. 






