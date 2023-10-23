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

size_t ipToInteger(const std::string& ipAddress);

int main(int argc, char **argv) {

  std::ifstream file("../data/listed_ip_90.txt"); 
  std::string ipaddr;
  std::set<size_t> ipset_uniq;
  
  if (file.is_open()) {
      while(std::getline(file, ipaddr))
      {
        ipset_uniq.insert(ipToInteger(ipaddr));
      }
      file.close(); 
  } else {
      std::cerr << "Failed to open the file.";
  }

  int insert_items = ipset_uniq.size()*0.8;
  clock_t start_time, end_time;
  // Create a beacon filter where each item is of type size_t and
  // use 12 bits for each item:
  BeaconFilter<size_t, 12> filter(insert_items);
  // Insert items to this beacon filter
  size_t num_inserted = 0;
  std::vector<size_t> ipsets(ipset_uniq.begin(), ipset_uniq.end());

  // ipset
  for (size_t i = 0; i < insert_items; i++, num_inserted++) {
    if (filter.Add(ipsets[i]) != beaconfilter::Ok) {
      break;
    }
  }
 
  // Check if previously inserted items are in the filter, expected
  // true for all items
  for (size_t i = 0; i < num_inserted; i++) {
    assert(filter.Contain(ipsets[i]) == beaconfilter::Ok);
  }

  // Check non-existing items, a few false positives expected
  size_t total_queries = 0;
  size_t false_queries = 0;
  for (size_t i = insert_items; i < ipset_uniq.size(); i++) {
    if (filter.Contain(ipsets[i]) == beaconfilter::Ok) {
      false_queries++;
    }
    total_queries++;
  }

  // Output the measured false positive rate
  std::cout << "false positive rate is "
            << 100.0 * false_queries / total_queries << "%\n";

  // Delete items
  // for (size_t i = 0; i < num_inserted; i++)
  // {
  //   assert(filter.Contain(ipsets[i]) == beaconfilter::Ok);
  //   filter.Delete(ipsets[i]);
  // }
  return 0;
}


size_t ipToInteger(const std::string& ipAddress) {
    size_t result = 0;
    std::istringstream iss(ipAddress);
    std::string segment;
    int shift = 24;

    while (std::getline(iss, segment, '.')) {
        unsigned int value = std::stoi(segment);
        result |= (value << shift);
        shift -= 8;
    }

    return result;
}