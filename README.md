# Virtual-Memory-Simulator

Sometimes some sort algorithms don't work correctly.I am already checking this in the main function and pressing whether there is a sort on the screen. (start 227. line)
The program runs very long for large inputs. I usually tried with numbers around 5.

I didn't implement WSClock and SC.

Here are the commands I tried as an example :

First make.

./sortArrays 4 3 5 FIFO global  10000 diskFileName.dat
./sortArrays 4 3 5 FIFO local  10000 diskFileName.dat
./sortArrays 4 3 5 NRU local  10000 diskFileName.dat
./sortArrays 4 3 5 NRU global  10000 diskFileName.dat
./sortArrays 4 3 5 LRU global  10000 diskFileName.dat
./sortArrays 4 3 5 LRU local  10000 diskFileName.dat
