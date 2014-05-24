#include "segmentor/decoder.h"

namespace ltp {
namespace segmentor {


void Decoder::decode(Instance * inst, bool natural ) {
  init_lattice(inst);
  if(natural) {
   // std::cout<<"natural viterbi"<<std::endl;
    natural_viterbi_decode(inst);
  } else {
   // std::cout<<"normal viterbi"<<std::endl;
    viterbi_decode(inst);
  }
  get_result(inst, natural);
  free_lattice();
}

void Decoder::init_lattice(const Instance * inst) {
  int len = inst->size();
  lattice.resize(len, L);
  lattice = NULL;
}

bool Decoder::segment_constrain(int natural, int l) {
  if(0 == natural) {
    return true; 
  }
  else if(4 == natural) {
    if(2 == l) {
      return true;
    }
  }
  else if(5 == natural) {
    if(0 == l || 2 == l) {
      return true;
    }
  }
  else if(6 == natural) {
    if(1 == l || 2 == l) {
      return true;
    }
  }

  return false;
}

void Decoder::viterbi_decode_inner(const Instance * inst, int i, int l){
  if (false == base.legal_emit(inst->chartypes[i], l)) {
    return;
  }
      if (i == 0) {
        LatticeItem * item = new LatticeItem(i, l, inst->uni_scores[i][l], NULL);
        lattice_insert(lattice[i][l], item);
      } else {
        for (int pl = 0; pl < L; ++ pl) {
          if (false == base.legal_trans(pl, l)) {
            continue;
          }

          double score = 0.;
          const LatticeItem * prev = lattice[i-1][pl];

          if (!prev) {
            continue;
          }

          // std::cout << i << " " << pl << " " << l << std::endl;
          score = inst->uni_scores[i][l] + inst->bi_scores[pl][l] + prev->score;
          const LatticeItem * item = new LatticeItem(i, l, score, prev);
          lattice_insert(lattice[i][l], item);
        }
      }   //  end for if i == 0
 
}
void Decoder::natural_viterbi_decode(const Instance * inst) {
  int len = inst->size();
  for (int i = 0; i < len; ++ i) {
//    std::cout<<inst->forms[i]<<" = "<<inst->natural[i]<<std::endl;
    for (int l = 0; l < L; ++ l) {
      if(segment_constrain(inst->natural[i], l)) {
  //      std::cout<<" label " <<l<<" decode"<<std::endl;
        viterbi_decode_inner(inst, i ,l);
      }
    }
  }
}

void Decoder::viterbi_decode(const Instance * inst) {
  int len = inst->size();
  for (int i = 0; i < len; ++ i) {
    for (int l = 0; l < L; ++ l) {
      if (false == base.legal_emit(inst->chartypes[i], l)) {
        continue;
      }
      if (i == 0) {
        LatticeItem * item = new LatticeItem(i, l, inst->uni_scores[i][l], NULL);
        lattice_insert(lattice[i][l], item);
      } else {
        for (int pl = 0; pl < L; ++ pl) {
          if (false == base.legal_trans(pl, l)) {
            continue;
          }

          double score = 0.;
          const LatticeItem * prev = lattice[i-1][pl];

          if (!prev) {
            continue;
          }

          // std::cout << i << " " << pl << " " << l << std::endl;
          score = inst->uni_scores[i][l] + inst->bi_scores[pl][l] + prev->score;
          const LatticeItem * item = new LatticeItem(i, l, score, prev);
          lattice_insert(lattice[i][l], item);
        }
      }   //  end for if i == 0
    }
  }
}

void Decoder::get_result(Instance * inst, bool natural ) {
  int len = inst->size();
  const LatticeItem * best_item = NULL;
  for (int l = 0; l < L; ++ l) {
    if (!lattice[len-1][l]) {
      continue;
    }
    if (best_item == NULL || (lattice[len-1][l]->score > best_item->score)) {
      best_item = lattice[len - 1][l];
    }
  }

  const LatticeItem * item = best_item;

  if(natural) {

    //std::cout<<"#natural get result"<<std::endl;

    inst->tagsidx.resize(len);
  
    while (item) {
      inst->tagsidx[item->i] = item->l;
      // std::cout << item->i << " " << item->l << std::endl;
      item = item->prev;
    }

   /* std::cout<<"#tags:";
    for(int i = 0; i < len; i++) {
      std::cout<<inst->tagsidx[i]<<"	";
    }
    std::cout<<std::endl;*/

  } else {

//    std::cout<<"#normal get result"<<std::endl;
    inst->predicted_tagsidx.resize(len);
    while (item) {
        inst->predicted_tagsidx[item->i] = item->l;
        // std::cout << item->i << " " << item->l << std::endl;
        item = item->prev;
    }
/*    std::cout<<"#tags:";
    for(int i = 0; i < len; i++) {
      std::cout<<inst->predicted_tagsidx[i]<<"	";
    }
    std::cout<<std::endl;*/
  }
}

void Decoder::free_lattice() {
  for (int i = 0; i < lattice.nrows(); ++ i) {
    for (int j = 0; j < lattice.ncols(); ++ j) {
      if (lattice[i][j]) delete lattice[i][j];
    }
  }
}

/*void KBestDecoder::decode(Instance * inst, KBestDecodeResult & result) {
  init_lattice(inst);
  kbest_viterbi_decode(inst);
  get_result(result);
  free_lattice();
}

void KBestDecoder::init_lattice(const Instance * inst) {
  int len = inst->len();
  lattice.resize(len, L);

  for (int i = 0; i < len; ++ i) {
    for (int l = 0; l < L; ++ l) {
      lattice[i][l] = new KHeap<LatticeItem>(k);
    }
  }
}

void KBestDecoder::kbest_viterbi_decode(const Instance * inst) {
}*/


}     //  end for namespace segmentor
}     //  end for namespace ltp

