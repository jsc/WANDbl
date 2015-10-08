#include <algorithm>
#include <cstring>
#include <string>
#include <cctype>
#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>

#include "indri/FileTreeIterator.hpp"
#include "indri/WARCDocumentIterator.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/TextTokenizer.hpp"
#include "indri/TaggedTextParser.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/KrovetzStemmer.hpp"

using namespace indri::parse;
using namespace indri::api;
using namespace std;

int 
main (int argc, char **argv)
{
  char line[8096];
  char word[1024];
  char *rv = NULL;
  KrovetzStemmer *stemmer = new KrovetzStemmer();

  while (fgets(line, 8096, stdin)) {
    char *lptr = line;
    int offset = 0;
    while (sscanf (lptr, "%s%n", word, &offset) == 1) {
      rv = stemmer->kstem_stemmer (word);
      printf ("%s ", rv);
      lptr += offset;
    }
    printf ("\n");
  }
  delete stemmer;
  return (0);
}
