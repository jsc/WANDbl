#ifndef _BM25_H
#define _BM25_H

template<uint32_t t_k1=90,uint32_t t_b=40>
struct my_rank_bm25 {
  static const double k1;
  static const double b;
  static const double epsilon_score;
  size_t num_docs;
  size_t num_terms;
  double avg_doc_len;
  double min_doc_len;
  std::vector<uint64_t> doc_lengths;
  static std::string name() {
    return "bm25";
  }
  my_rank_bm25(){}
  my_rank_bm25& operator=(const my_rank_bm25&) = default;

  my_rank_bm25(std::vector<uint64_t> doc_len, uint64_t terms) : my_rank_bm25(doc_len, terms, doc_len.size()) { }

  my_rank_bm25(std::vector<uint64_t> doc_len, uint64_t terms, uint64_t numdocs) : num_docs(numdocs), avg_doc_len((double)terms/(double)numdocs) {
    doc_lengths = std::move(doc_len); //Takes ownership of the vector!

    std::cerr<<"num_docs = "<<num_docs<<std::endl;
    std::cerr<<"avg_doc_len = "<<avg_doc_len<<std::endl;
  }
  double doc_length(size_t doc_id) const {
    return (double) doc_lengths[doc_id];
  }
  double calc_doc_weight(double ) const {
    return 0;
  }
  double calculate_docscore(const double f_qt,const double f_dt,
                            const double f_t, const double W_d,bool) const
  {
    double w_qt = std::max(epsilon_score, log((num_docs - f_t + 0.5) / (f_t+0.5)) * f_qt);
    double K_d = k1*((1-b) + (b*(W_d/avg_doc_len)));
    double w_dt = ((k1+1)*f_dt) / (K_d + f_dt);
    return w_dt*w_qt;
  }
};

/*SUPER IMPORTANT*/
template<uint32_t t_k1,uint32_t t_b>
const double my_rank_bm25<t_k1,t_b>::k1 = (double)t_k1/100.0;

template<uint32_t t_k1,uint32_t t_b>
const double my_rank_bm25<t_k1,t_b>::b = (double)t_b/100.0;

template<uint32_t t_k1,uint32_t t_b>
const double my_rank_bm25<t_k1,t_b>::epsilon_score = 1e-6;

#endif
