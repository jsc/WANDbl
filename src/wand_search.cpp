#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "query.hpp"
#include "invidx.hpp"
#include "bm25.hpp"
    
typedef struct cmdargs {
    std::string collection_dir;
    std::string query_file;
    std::string postings_file;
    std::string F_t_file;
    std::string df_t_file;
    std::string doclen_file;
    std::string global_file;
    std::string output_prefix;

    bool is_exhaustive;
    uint64_t k;
} cmdargs_t;

void
print_usage (char* program)
{
  fprintf(stdout,"%s -c <collection> -q <query file> -k <top-k>",program);
  fprintf(stdout," -o <output>\n");
  fprintf(stdout,"where\n");
  fprintf(stdout,"  -c <collection>  : the collection directory.\n");
  fprintf(stdout,"  -q <query file>  : the queries to process.\n");
  fprintf(stdout,"  -k <top-k>  : the number of documents to be retrieved.\n");
  fprintf(stdout,"  -o <output> : prefix for output files.\n");
  fprintf(stdout,"  -e   : turn on exhaustive processing, defaults to wand\n");
  exit(EXIT_FAILURE);
};

cmdargs_t
parse_args(int argc,char* const argv[])
{
  cmdargs_t args;
  int op;
  args.collection_dir = "";
  args.output_prefix = "wand";
  args.is_exhaustive = false;
  args.k = 10;
  while ((op=getopt(argc,argv,"c:q:k:o:e")) != -1) {
    switch (op) {
      case 'c':
        args.collection_dir = optarg;
        args.postings_file = args.collection_dir + "/WANDbl_postings.idx";
        args.F_t_file = args.collection_dir +"/WANDbl_F_t.idx";
        args.df_t_file = args.collection_dir +"/WANDbl_df_t.idx";
        args.doclen_file = args.collection_dir +"/doc_lens.txt";
        args.global_file = args.collection_dir +"/global.txt";
        break;
      case 'o':
        args.output_prefix = optarg;
        break;
      case 'q':
        args.query_file = optarg;
        break;
      case 'k':
        args.k = std::strtoul(optarg,NULL,10);
        break;
      case 'e':
        args.is_exhaustive = true;
        break;
      case '?':
      default:
        print_usage(argv[0]);
    }
  }
  if (args.collection_dir==""||args.query_file=="") {
    std::cerr << "Missing command line parameters.\n";
    print_usage(argv[0]);
  }
  return args;
}

int 
main (int argc,char* const argv[])
{
  /* define types */
  using plist_type = block_postings_list<128>;
  using my_index_t = idx_invfile<plist_type,my_rank_bm25<> >;
  using clock = std::chrono::high_resolution_clock;
  /* parse command line */
  cmdargs_t args = parse_args(argc,argv);

  /* parse repo */
  auto cc = parse_collection(args.collection_dir);

  // read warm-up queries if specified
  std::vector<query_t> warm_queries;

  /* parse queries */
  std::cout << "Parsing query file '" << args.query_file << "'" << std::endl;
  auto queries = query_parser::parse_queries(args.collection_dir,args.query_file);
  std::cout << "Found " << queries.size() << " queries." << std::endl;

  std::string index_name(basename(strdup(args.collection_dir.c_str())));

  /* load the index */
  my_index_t index;
  auto load_start = clock::now();
  // Construct index instance.
  construct(index, args.postings_file, args.F_t_file, args.df_t_file);

  // Get vector of doc lengths and uint64 term count using asc file
  uint64_t term_count = 0, temp;
  std::vector<uint64_t>doc_lens;
  doc_lens.reserve(131072); // Speed up load.
  ifstream doclen_file(args.doclen_file);
  if(doclen_file.is_open() != true){
    std::cerr << "Couldn't open: " << args.doclen_file << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Reading document lengths." << std::endl;
  /*Read the lengths of each document from asc file into vector*/
  while(doclen_file >> temp){
    doc_lens.push_back(temp);
    term_count += temp;
  }

  if(args.global_file != "") {
    ifstream global_file(args.global_file);
    if(global_file.is_open() != true){
      std::cerr << "Couldn't open: " << args.global_file << std::endl;
      exit(EXIT_FAILURE);
    }

    uint64_t total_docs, total_terms;
    global_file >> total_docs >> total_terms;

    index.load(doc_lens, total_terms, total_docs);
  }

  auto load_stop = clock::now();
  auto load_time_sec = std::chrono::duration_cast<std::chrono::seconds>(load_stop-load_start);
  std::cout << "Index loaded in " << load_time_sec.count() << " seconds." << std::endl;

  /* process the queries */
  std::map<uint64_t,std::chrono::microseconds> query_times;
  std::map<uint64_t,result> query_results;
  std::map<uint64_t,uint64_t> query_lengths;

  size_t num_runs = 1;
  for(size_t i=0;i<num_runs;i++) {
    for(const auto& query: queries) {
      auto id = std::get<0>(query);
      auto qry_tokens = std::get<1>(query);
      std::cout << "[" << id << "] |Q|=" << qry_tokens.size(); 
      std::cout.flush();

      // run the query
      auto qry_start = clock::now();
      auto results = index.search(qry_tokens,args.k, false, true, 
                                  args.is_exhaustive);
      auto qry_stop = clock::now();

      auto query_time = std::chrono::duration_cast<std::chrono::microseconds>(qry_stop-qry_start);
      std::cout << " TIME = " << std::setprecision(5)
                << query_time.count() / 1000.0 
                << " ms" << std::endl;

      auto itr = query_times.find(id);
      if(itr != query_times.end()) {
        itr->second += query_time;
      } else {
        query_times[id] = query_time;
      }

      if(i==0) {
        query_results[id] = results;
        query_lengths[id] = qry_tokens.size();
      }
    }
  }

  /* output results to csv */
  char time_buffer [80] = {0};
  std::time_t t = std::time(NULL);
  auto timeinfo = localtime (&t);
  strftime (time_buffer,80,"%F-%H:%M:%S",timeinfo);
  std::string search_type = (args.is_exhaustive) ? "exhaustive" : "wand";
  std::string qfile(basename(strdup(args.query_file.c_str())));
  std::string time_output_file = args.collection_dir + "/results/" 
             + search_type+"-timings-" + qfile + "-k" + std::to_string(args.k) 
             + "-" + std::string(time_buffer) + ".csv";
  std::string res_output_file = args.collection_dir + "/results/" 
             + search_type+"-results-" + qfile + "-k" + std::to_string(args.k) 
             + "-" + std::string(time_buffer) + ".csv";

  /* calc average */
  for(auto& timing : query_times) {
    timing.second = timing.second / num_runs;
  }

  std::string time_file = args.output_prefix + "-time.log";

  /* output */
  std::cout << "Writing timing results to '" << time_file << "'" << std::endl;     
  std::ofstream resfs(time_file);
  if(resfs.is_open()) {
    resfs << "query;num_results;postings_eval;docs_fully_eval;docs_added_to_heap;threshold;num_terms;time_ms;" << std::endl;
    for(const auto& timing: query_times) {
      auto qry_id = timing.first;
      auto qry_time = timing.second;
      auto results = query_results[qry_id];
      resfs << qry_id << ";" << results.list.size() << ";" 
            << results.postings_evaluated << ";"
            << results.docs_fully_evaluated << ";" 
            << results.docs_added_to_heap << ";" 
            << results.final_threshold << ";" 
            << query_lengths[qry_id] << ";" 
            << qry_time.count() / 1000.0 << std::endl;
    }
  } else {
    perror ("Could not output results to file.");
  }

  // Write TREC output file.

  /* load the docnames map */
  std::unordered_map<uint64_t,std::string> id_mapping;
  std::string doc_names_file = args.collection_dir + "/doc_names.txt";
  std::ifstream dfs(doc_names_file);
  size_t j=0;
  std::string name_mapping;
  while( std::getline(dfs,name_mapping) ) {
    id_mapping[j] = name_mapping;
    j++;
  }
 

  std::string trec_file = args.output_prefix + "-trec.run";
  std::cout << "Writing trec output to " << trec_file << std::endl;
  std::ofstream trec_out(trec_file);
  if(trec_out.is_open()) {
    for(const auto& result: query_results) {
      auto qry_id = result.first;
      auto qry_res = result.second.list;
      for(size_t i=1;i<=qry_res.size();i++) {
        trec_out << qry_id << "\t"
                 << "Q0" << "\t"
                 << id_mapping[qry_res[i-1].doc_id] << "\t"
                 << i << "\t"
                 << qry_res[i-1].score << "\t"  
                 << "WANDbl" << std::endl;
      }
    }
  } else {
    perror ("Could not output results to file.");
  }

  return EXIT_SUCCESS;
}
