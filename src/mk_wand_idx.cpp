#include <iostream>

#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "sdsl/int_vector_buffer.hpp"
#include "include/block_postings_list.hpp"
#include "include/bm25.hpp"


#define INIT_SZ 4096 

bool
directory_exists(std::string dir)
{
  struct stat sb;
  const char* pathname = dir.c_str();
  if (stat(pathname, &sb) == 0 && S_ISDIR(sb.st_mode)) {
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


int 
main (int argc, char** argv) 
{
  if (argc != 3) {
    std::cout << "USAGE: " << argv[0];
    std::cout << " <indri repository> <collection folder>" << std::endl;
        return EXIT_FAILURE;
  }

  using clock = std::chrono::high_resolution_clock;

  // parse cmd line
  std::string repository_name = argv[1];
  std::string collection_folder = argv[2];
  create_directory(collection_folder);
  std::string dict_file = collection_folder + "/dict.txt";
  std::string doc_names_file = collection_folder + "/doc_names.txt";
  std::string postings_file = collection_folder + "/WANDbl_postings.idx";
  std::string ft_file = collection_folder + "/WANDbl_F_t.idx";
  std::string dft_file = collection_folder + "/WANDbl_df_t.idx";
  std::string global_info_file = collection_folder + "/global.txt";
  std::string doclen_tfile = collection_folder + "/doc_lens.txt";

  std::ofstream doclen_out(doclen_tfile);

  auto build_start = clock::now();

  // load stuff
  indri::collection::Repository repo;
  repo.openRead(repository_name);

  // Keep track of term ordering
  unordered_map<string, uint64_t> map;

  //Vector of doc lengths
  vector<uint64_t> doc_lengths;
  uint64_t num_terms = 0;
 
  std::cout << "Writing global info to " << global_info_file << "."
            << std::endl;
  std::vector<std::string> document_names;
  indri::collection::Repository::index_state state = repo.indexes();
  const auto& index = (*state)[0];

  // dump global info; num documents in collection, num of all terms
  std::ofstream of_globalinfo(global_info_file);
  of_globalinfo << index->documentCount() << " "
                << index->termCount() << std::endl;

  std::cout << "Writing document lengths to " << doclen_tfile << "."
            << std::endl;
  uint64_t uniq_terms = index->uniqueTermCount();
  // Shift all IDs from Indri by 2 so \0 and \1 are free.
  uniq_terms += 2; 
  indri::collection::CompressedCollection* collection = repo.collection();
  int64_t document_id = index->documentBase();
  indri::index::TermListFileIterator* iter = index->termListFileIterator();
  iter->startIteration();
  while( !iter->finished() ) {
    indri::index::TermList* list = iter->currentEntry();

    // find document name
    std::string doc_name = collection->retrieveMetadatum( document_id , "docno" );
    document_names.push_back(doc_name);

    // Add doclens
    doclen_out << list->terms().size() << std::endl;
    doc_lengths.push_back(list->terms().size());
    num_terms += list->terms().size();
    document_id++;
    iter->nextEntry();
  }

  // write document names
  {
    std::cout << "Writing document names to " << doc_names_file << "." 
              << std::endl;
    std::ofstream of_doc_names(doc_names_file);
    for(const auto& doc_name : document_names) {
      of_doc_names << doc_name << std::endl;
    }
  }

  // write dictionary
  {
    std::cout << "Writing dictionary to " << dict_file << "." << std::endl;
    const auto& index = (*state)[0];
    std::ofstream of_dict(dict_file);

    indri::index::VocabularyIterator* iter = index->vocabularyIterator();
    iter->startIteration();

    size_t j = 2;
    while( !iter->finished() ) {
      indri::index::DiskTermData* entry = iter->currentEntry();
      indri::index::TermData* termData = entry->termData;

      map.emplace(termData->term, j);

      of_dict << termData->term << " " << j << " "
              << termData->corpus.documentCount << " "
              << termData->corpus.totalCount << " "
              <<  std::endl;

      iter->nextEntry();
      j++;
    }
    delete iter;
  }



  // write inverted files
  {
    using plist_type = block_postings_list<128>;
    vector<plist_type> m_postings_lists; 
    uint64_t a = 0, b = 0;
    uint64_t n_terms = index->uniqueTermCount();

    vector<pair<uint64_t, uint64_t>> post; 
    post.reserve(INIT_SZ);

    // Open the files
    filebuf post_file;
    post_file.open(postings_file, std::ios::out);
    ostream ofs(&post_file);

    filebuf F_t_file;
    F_t_file.open(ft_file, std::ios::out);
    ostream Ft(&F_t_file);

    filebuf f_t_file;
    f_t_file.open(dft_file, std::ios::out);
    ostream ft(&f_t_file); 

    std::cerr << "Writing postings lists ..." << std::endl;

    m_postings_lists.resize(n_terms + 2);
    my_rank_bm25<90,40> ranker(doc_lengths, num_terms);
    sdsl::int_vector<> F_t_list(n_terms + 2);
    sdsl::int_vector<> f_t_list(n_terms + 2);

    const auto& index = (*state)[0];
    indri::index::DocListFileIterator* iter = index->docListFileIterator();
    iter->startIteration();

    while( !iter->finished() ) {
      indri::index::DocListFileIterator::DocListData* entry = 
        iter->currentEntry();
      indri::index::TermData* termData = entry->termData;

      entry->iterator->startIteration();
      post.clear();

      F_t_list[map[termData->term]] = termData->corpus.totalCount;
      f_t_list[map[termData->term]] = termData->corpus.documentCount;
      while( !entry->iterator->finished() ) {
        indri::index::DocListIterator::DocumentData* doc = 
          entry->iterator->currentEntry();

        a = doc->document - 1;
        b = doc->positions.size();
        post.emplace_back(a,b);
        entry->iterator->nextEntry();
      }
      plist_type pl(ranker, post);
      m_postings_lists[map[termData->term]] = pl;
      iter->nextEntry();
    }
    delete iter;

    size_t num_lists = m_postings_lists.size();
    cout << "Writing " << num_lists << " postings lists." << endl;
    sdsl::serialize(num_lists, ofs);
    for(const auto& pl : m_postings_lists) {
      sdsl::serialize(pl, ofs);
    }
  
    //Write F_t data to file, skip 0 and 1
    cout << "Writing F_t lists." << endl;
    F_t_list.serialize(Ft);

    //Write out document frequency (num docs that term appears in), skip 0 and 1
    cout << "Writing f_t lists." << endl;
    f_t_list.serialize(ft);

    //close output files
    post_file.close();
    F_t_file.close();
    f_t_file.close();
  }

  auto build_stop = clock::now();
  auto build_time_sec = std::chrono::duration_cast<std::chrono::seconds>(build_stop-build_start);
  std::cout << "Index built in " << build_time_sec.count() << " seconds." << std::endl;

  return (EXIT_SUCCESS);
}


