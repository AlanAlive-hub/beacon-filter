#include "beaconfilter.h"

#include <assert.h>
#include <math.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

using beaconfilter::BeaconFilter;


int main(int argc, char **argv) {

  size_t total_number = 4000000;
  clock_t start_time, end_time;
  // Create a beacon filter where each item is of type size_t and
  // use 12 bits for each item:
  // BeaconFilter<size_t, 12> filter(total_items);
  BeaconFilter<size_t, 2> filter(total_number);
  // Insert items to this beacon filter
  size_t num_inserted = 0;
  // start_time = clock();
  // for (size_t i = total_number; i < 2*total_number; i++, num_inserted++) {
  //   if(filter.lf() < 0.8) filter.Add(i);
  //   else{
  //     printf("%.2f", filter.lf());
  //     break;
  //   }
  // }
  // end_time = clock();
  // printf( "%f\n", ((double)(end_time - start_time) / CLOCKS_PER_SEC));
  for(size_t i = total_number; i< 2*total_number; ++i)
  {
    if(filter.Add(i) != beaconfilter::Ok)
      break;
  }
  std::cout << filter.Info() << std::endl;
  // Check if previously inserted items are in the filter, expected
  // true for all items
  for (size_t i = 0; i < num_inserted; i++) {
    assert(filter.Contain(i) == beaconfilter::Ok);
  }

  // Check non-existing items, a few false positives expected
  size_t total_queries = 0;
  size_t false_queries = 0;
  for (size_t i = 0; i < total_number; i++) {
    if (filter.Contain(i) == beaconfilter::Ok) {
      false_queries++;
    }
    total_queries++;
  }

  // Output the measured false positive rate
  std::cout << "false positive rate is "
            << 100.0 * false_queries / total_queries << "%\n";
  return 0;
}
