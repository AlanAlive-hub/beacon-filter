#ifndef BEACON_FILTER_SINGLE_TABLE_H_
#define BEACON_FILTER_SINGLE_TABLE_H_

#include <assert.h>

#include <sstream>
#include <vector>
#include <bitset>
#include <string.h>



namespace beaconfilter {

// the most naive table implementation: one huge bit array
template <size_t bits_per_tag>
class SingleTable {
  static const size_t kTagsPerBucket = 4;
  static const size_t kBytesPerBucket =
      (bits_per_tag * kTagsPerBucket + 7) >> 3;
  static const uint32_t kTagMask = (1ULL << bits_per_tag) - 1;
  // NOTE: accomodate extra buckets if necessary to avoid overrun
  // as we always read a uint64
  static const size_t kPaddingBuckets =
    ((((kBytesPerBucket + 7) / 8) * 8) - 1) / kBytesPerBucket;

  struct Bucket {
    char bits_[kBytesPerBucket];
  } __attribute__((__packed__));
  std::vector<std::bitset<kTagsPerBucket>> beacon;
  // using a pointer adds one more indirection
  Bucket *buckets_;
  size_t num_buckets_;

 public:
  explicit SingleTable(const size_t num) : num_buckets_(num) {
    buckets_ = new Bucket[num_buckets_ + kPaddingBuckets];
    memset(buckets_, 0, kBytesPerBucket * (num_buckets_ + kPaddingBuckets));
    beacon.reserve(num_buckets_ + kPaddingBuckets);
  }

  ~SingleTable() { 
    delete[] buckets_;
  }

  size_t NumBuckets() const {
    return num_buckets_;
  }

  size_t SizeInBytes() const { 
    return kBytesPerBucket * num_buckets_; 
  }

  size_t SizeInTags() const { 
    return kTagsPerBucket * num_buckets_; 
  }

  std::string Info() const {
    std::stringstream ss;
    ss << "SingleHashtable with tag size: " << bits_per_tag << " bits \n";
    ss << "\t\tAssociativity: " << kTagsPerBucket << "\n";
    ss << "\t\tTotal # of rows: " << num_buckets_ << "\n";
    ss << "\t\tTotal # slots: " << SizeInTags() << "\n";
    return ss.str();
  }

  // read tag from pos(i,j)
  inline uint32_t ReadTag(const size_t i, const size_t j) const {
    const char *p = buckets_[i].bits_;
    uint32_t tag;
    /* following code only works for little-endian */
    if (bits_per_tag == 2) {
      tag = *((uint8_t *)p) >> (j * 2);
    } else if (bits_per_tag == 4) {
      p += (j >> 1);
      tag = *((uint8_t *)p) >> ((j & 1) << 2);
    } else if (bits_per_tag == 8) {
      p += j;
      tag = *((uint8_t *)p);
    } else if (bits_per_tag == 12) {
      p += j + (j >> 1);
      tag = *((uint16_t *)p) >> ((j & 1) << 2);
    } else if (bits_per_tag == 16) {
      p += (j << 1);
      tag = *((uint16_t *)p);
    } else if (bits_per_tag == 32) {
      tag = ((uint32_t *)p)[j];
    }
    return tag & kTagMask;
  }

  // write tag to pos(i,j)
  inline void WriteTag(const size_t i, const size_t j, const uint32_t t) {
    char *p = buckets_[i].bits_;
    uint32_t tag = t & kTagMask;
    /* following code only works for little-endian */
    if (bits_per_tag == 2) {
      *((uint8_t *)p) |= tag << (2 * j);
    } else if (bits_per_tag == 4) {
      p += (j >> 1);
      if ((j & 1) == 0) {
        *((uint8_t *)p) &= 0xf0;
        *((uint8_t *)p) |= tag;
      } else {
        *((uint8_t *)p) &= 0x0f;
        *((uint8_t *)p) |= (tag << 4);
      }
    } else if (bits_per_tag == 8) {
      ((uint8_t *)p)[j] = tag;
    } else if (bits_per_tag == 12) {
      p += (j + (j >> 1));
      if ((j & 1) == 0) {
        ((uint16_t *)p)[0] &= 0xf000;
        ((uint16_t *)p)[0] |= tag;
      } else {
        ((uint16_t *)p)[0] &= 0x000f;
        ((uint16_t *)p)[0] |= (tag << 4);
      }
    } else if (bits_per_tag == 16) {
      ((uint16_t *)p)[j] = tag;
    } else if (bits_per_tag == 32) {
      ((uint32_t *)p)[j] = tag;
    }
  }

  inline bool FindTagInBuckets(const size_t i1, const size_t i2,
                               const uint32_t tag) const {
    for (size_t j = 0; j < kTagsPerBucket; j++) {
      if ((ReadTag(i1, j) == tag && beacon[i1][kTagsPerBucket-j-1] == 0) || 
          (ReadTag(i2, j) == tag && beacon[i2][kTagsPerBucket-j-1] == 1))
        return true;
    }
    return false;
  }

  inline bool FindTagInBucket(const size_t i, const uint32_t tag, uint8_t flag) const {

      for (size_t j = 0; j < kTagsPerBucket; j++) {
        if (ReadTag(i, j) == tag && beacon[i][kTagsPerBucket-j-1] == flag) {
          return true;
        }
      }
      return false;
  }

  inline bool DeleteTagFromBucket(const size_t i, const uint32_t tag, uint8_t flag) {
    for (size_t j = 0; j < kTagsPerBucket; j++) {
      if (ReadTag(i, j) == tag) {
        assert(FindTagInBucket(i, tag, flag) == true);
        WriteTag(i, j, 0);
        if(flag == 1) beacon[i][kTagsPerBucket-j-1] = 0;
        return true;
      }
    }
    return false;
  }

  inline bool InsertTagToBucket(const size_t i, const uint32_t tag, uint8_t flag, 
                                const bool kickout, uint32_t &oldtag, uint8_t &oldflag) {
    for (size_t j = 0; j < kTagsPerBucket; j++) {
      if (ReadTag(i, j) == 0) {
        WriteTag(i, j, tag);
        if(flag) beacon[i][kTagsPerBucket-j-1] = flag; 
        return true;
      }
    }
    if (kickout) {
      size_t r = rand() % kTagsPerBucket;
      oldtag = ReadTag(i, r);
      WriteTag(i, r, tag);
      oldflag = beacon[i][kTagsPerBucket-r-1];
      beacon[i][kTagsPerBucket-r-1] = flag;
    }
    return false;
  }

  inline size_t NumTagsInBucket(const size_t i) const {
    size_t num = 0;
    for (size_t j = 0; j < kTagsPerBucket; j++) {
      if (ReadTag(i, j) != 0) {
        num++;
      }
    }
    return num;
  }
};
}  // namespace beaconfilter
#endif  // BEACON_FILTER_SINGLE_TABLE_H_
