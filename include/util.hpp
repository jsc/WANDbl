#ifndef UTIL_HPP
#define UTIL_HPP

#include "sdsl/io.hpp"

#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const std::string DICT_FILENAME = "dict.txt";
const std::string DOCNAMES_FILENAME = "doc_names.txt";

// If the maximum possible score of a term is less than this theshold, the
// term will not be used in the score computation
const double SCORE_THRESHOLD = 0.01;

// Knuth trick for comparing floating numbers
// check if a and b are equal with respect to the defined tolerance epsilon
inline bool float_equals( float a, float b) { 
  const float epsilon = 0.0001; // or some other small number 
  if ((std::fabs(a-b) <= epsilon*std::fabs(a)) && (std::fabs(a-b) <= epsilon*std::fabs(b))) return true;
  else return false; 
}

// compare two floats ONLY for greater or smaller and NOT for equality
inline bool float_ltgt( float a, float b) { 
  if (a > b) return true;
  else return false; 
}

// Arguments a,b and returns a) 0 in case of equality, b) 1 in case a > b, and c) -1 in case a < b
inline int fcompare( float a, float b) {
  if (float_equals(a, b)) {
    return 0; // equal
  } else { 
    if (float_ltgt(a, b)) return 1;  // a > b
    else return -1; 
  }      // a < b
}


bool
directory_exists(std::string dir)
{
  struct stat sb;
  const char* pathname = dir.c_str();
  if (stat(pathname, &sb) == 0 && (S_IFDIR&sb.st_mode)) {
    return true;
  }
  return false;
}

bool
file_exists(std::string file_name)
{
  sdsl::isfstream in(file_name);
  if (in) {
    in.close();
    return true;
  }
  return false;
}

bool
symlink_exists(std::string file)
{
  struct stat sb;
  const char* filename = file.c_str();
  if (stat(filename, &sb) == 0 && (S_IFLNK&sb.st_mode) ) {
    return true;
  }
  return false;
}

void
create_directory(std::string dir)
{
  if (!directory_exists(dir)) {
    if (mkdir(dir.c_str(),0777) == -1) {
      perror("could not create directory");
      exit(EXIT_FAILURE);
    }
  }
}

#endif
