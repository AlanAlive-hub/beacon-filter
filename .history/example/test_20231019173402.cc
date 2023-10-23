#include "\src\beaconfilter.h"

#include <iostream>

using beaconfilter::BeaconFilter;


int main(int argc, char **argv) {

  size_t total_number = 1000000;

  // Create a beacon filter where each item is of type size_t and
  // use 12 bits for each item:
  BeaconFilter<size_t, 12> filter(total_number);

  // Insert items to this beacon filter
  size_t num_inserted = 0;
  for(size_t i = total_number; i< 2*total_number; ++i)
  {
    if(filter.Add(i) != beaconfilter::Ok)
      break;
  }

  // Check if previously inserted items are in the filter
  for (size_t i = 0; i < num_inserted; i++) {
    assert(filter.Contain(i) == beaconfilter::Ok);
  }

  // Check non-existing items
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
