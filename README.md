# Quick implementation of the Lampery-Bakery algorithm

This program is meant to be runned on Arm-based CPUs.
It implements a basic lampery-bakery algorithm, using DMBs.

## Compile

make without args compile code without DMBs and 4 threads.

add -D followed by:

    * DMB\_ST\_ENABLED: Use DMB ST to order writes

    * DMB\_SY\_ENABLED: Use DMB SY to order writes and reads

    * NB\_THREADS=N   : Configure the number of threads

    * GET\_ADDRESSES  : print the addresses of entering/number variables

    * PADDING         : Use padding to force these variables to not be on continous memory

## Run

./lamport
