# beaconfilter

## Overview

Without expanding the fingerprint length or filter size, the BeF achieves the lowest false positive rate at the same space with the CF (and its variants) and the smallest space overhead at the same false positive rate. the BeF compares the fingerprint of items and the positions with which the items are inserted. The work is recorded as a 1-bit flag in an external structure named ``beacon matrix''. We evaluate the performance of BeF in large datasets. The results show that BeF can improve lookup accuracy to 100\% in most cases with similar load capacity, and achieve ~71% less space than CF for the same accuracy. Moreover, we present an optimization technique to compress the space requirement of the beacon matrix by transforming the data structure of flags. The space cost is around 1.59x to 5.88x smaller than the beacon matrix.

## API
A beacon filter supports the following operations:

*  `Add(item)`: insert an item into the filter
*  `Contain(item)`: return if an item is already in the filter.
*  `Delete(item)`: delete the given item from the filter. Just so you know, to use this method, it must be made sure that this item is in the filter (e.g., based on records on external storage); otherwise, a false item may be deleted.

Setting 'zip=1' if you wanna use the Zipping-Beacon optimization.  

## Build

$ sudo apt-get install openssl

To build the example (`src\test.cc`):
```bash
$ make 
$ ./test
```
