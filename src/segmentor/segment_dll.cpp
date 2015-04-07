#include "segmentor/segment_dll.h"
#include "segmentor/customized_segmentor.h"
#include "segmentor/settings.h"
//#include "instance.h"
#include "utils/logging.hpp"
#include "utils/codecs.hpp"
#include "utils/cache.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <stdio.h>

namespace seg = ltp::segmentor;

static ltp::utility::LRUCache<std::string, std::shared_ptr<seg::Model> > cache(20);
class SegmentorWrapper : public seg::CustomizedSegmentor{
public:
  SegmentorWrapper()
    : beg_tag0(-1),
    beg_tag1(-1),
    rule(0) {}

  ~SegmentorWrapper() {
    if (rule) { delete rule; }
  }

  bool load_baseline(const char * model_path, const char * lexicon_path = NULL) {
    if ((seg::CustomizedSegmentor::baseline_model = load_model( model_path, lexicon_path))==NULL) {
      return false;
    }

    beg_tag0 = seg::CustomizedSegmentor::baseline_model->labels.index( seg::__b__ );
    beg_tag1 = seg::CustomizedSegmentor::baseline_model->labels.index( seg::__s__ );

    if (!rule) {
      rule = new seg::rulebase::RuleBase(seg::CustomizedSegmentor::baseline_model->labels);
    }

    return true;
  }

  bool load_customized(const char * model_path, const char * lexicon_path = NULL) {
    if ((seg::CustomizedSegmentor::model = load_model(model_path, lexicon_path))==NULL) {
      return false;
    }

    beg_tag0 = seg::CustomizedSegmentor::model->labels.index( seg::__b__ );
    beg_tag1 = seg::CustomizedSegmentor::model->labels.index( seg::__s__ );

    if (!rule) {
      rule = new seg::rulebase::RuleBase(seg::CustomizedSegmentor::model->labels);
    }

    return true;
  }

  int segment(const char * str,
      std::vector<std::string> & words) {
    seg::Instance * inst = new seg::Instance;
    // ltp::strutils::codecs::decode(str, inst->forms);
    int ret = seg::rulebase::preprocess(str,
        inst->raw_forms,
        inst->forms,
        inst->chartypes);

    if (-1 == ret || 0 == ret) {
      delete inst;
      words.clear();
      return 0;
    }

    seg::DecodeContext* ctx = new seg::DecodeContext;
    seg::ScoreMatrix* scm = new seg::ScoreMatrix;
    seg::Segmentor::build_lexicon_match_state(seg::CustomizedSegmentor::baseline_model, inst);
    seg::Segmentor::extract_features(inst, seg::CustomizedSegmentor::baseline_model, ctx);
    seg::Segmentor::calculate_scores(inst, seg::CustomizedSegmentor::baseline_model, ctx, true, scm, seg::CustomizedSegmentor::baseline_model->end_time);

    // allocate a new decoder so that the segmentor support multithreaded
    // decoding. this modification was committed by niuox
    seg::Decoder decoder(seg::CustomizedSegmentor::baseline_model->num_labels(), *rule);
    decoder.decode(inst, scm);
    seg::Segmentor::build_words(inst, inst->predicted_tagsidx,
        words, beg_tag0, beg_tag1);

    delete ctx;
    delete scm;
    delete inst;
    return words.size();
  }

  int customized_segment(seg::Model * customized_model,
              const char * str,
              std::vector<std::string> & words) {
    seg::Instance * inst = new seg::Instance;
    int ret = seg::rulebase::preprocess(str,
        inst->raw_forms,
        inst->forms,
        inst->chartypes);

    if (-1 == ret || 0 == ret) {
      delete inst;
      words.clear();
      return 0;
    }

    seg::DecodeContext* ctx = new seg::DecodeContext;
    seg::DecodeContext* base_ctx = new seg::DecodeContext;
    seg::ScoreMatrix* scm = new seg::ScoreMatrix;
    seg::ScoreMatrix* base_scm = new seg::ScoreMatrix;
    seg::CustomizedSegmentor::build_lexicon_match_state(customized_model, seg::CustomizedSegmentor::baseline_model, inst);
    seg::Segmentor::extract_features(inst, customized_model, ctx);
    seg::Segmentor::extract_features(inst, seg::CustomizedSegmentor::baseline_model, base_ctx);
    seg::CustomizedSegmentor::calculate_scores(customized_model,
        seg::CustomizedSegmentor::baseline_model,
        inst,
        ctx,
        base_ctx,
        true,
        scm,
        base_scm);

    seg::Decoder decoder(seg::CustomizedSegmentor::baseline_model->num_labels(), *rule);
    decoder.decode(inst, scm);
    seg::Segmentor::build_words(inst, inst->predicted_tagsidx, words, beg_tag0, beg_tag1);

    delete ctx;
    delete base_ctx;
    delete scm;
    delete base_scm;
    delete inst;
    return words.size();
  }

  int segment(const std::string & str,
      std::vector<std::string> & words) {
    return segment(str.c_str(), words);
  }

  int customized_segment(const std::string & str,
                         std::vector<std::string> &words) {
    return customized_segment(seg::CustomizedSegmentor::model, str.c_str(), words);
  }

  int customized_segment(const char * model_path,
              const char * lexicon_path,
              const std::string & str,
              std::vector<std::string> & words) {
    seg::Model * customized_model = NULL;
    if ((customized_model = load_model(model_path, lexicon_path))==NULL) {
      return 0;
    }
    int len = customized_segment(customized_model, str.c_str(), words);
    delete customized_model;

    return len;
  }
  static seg::Model* load_model(const char * model_path, const char * lexicon_path = NULL) {
    if ((NULL == model_path)&&(NULL == lexicon_path)) {
      return NULL;
    }

    seg::Model *mdl = new seg::Model;

    if (NULL!=model_path) {
      std::ifstream mfs(model_path, std::ifstream::binary);

      if (mfs) {
        if (!mdl->load(mfs)) {
          delete mdl;
          mdl = 0;
          return NULL;
        }
      } else {
          delete mdl;
          mdl = 0;
          return NULL;
      }
    }

    if (NULL != lexicon_path) {
      std::ifstream lfs(lexicon_path);

      if (lfs) {
        std::string buffer;
        while (std::getline(lfs, buffer)) {
          buffer = ltp::strutils::chomp(buffer);
          if (buffer.size() == 0) {
            continue;
          }
          mdl->external_lexicon.set(buffer.c_str(), true);
        }
      } else {
        delete mdl;
        mdl = 0;
        return NULL;
      }
    }

    /*if (lexicon_path!=NULL&&model_path==NULL) {
      mdl->end_time = seg::CustomizedSegmentor::baseline_model->end_time;
    }*/

    return mdl;
  }
private:
  int beg_tag0;
  int beg_tag1;

  // don't need to allocate a decoder
  // one sentence, one decoder
  seg::rulebase::RuleBase* rule;

};

void * segmentor_create_segmentor(const char * path, const char * lexicon_file) {
  SegmentorWrapper * wrapper = new SegmentorWrapper();

  if (!wrapper->load_baseline(path, lexicon_file)) {
    delete wrapper;
    return 0;
  }

  return reinterpret_cast<void *>(wrapper);
}

void * segmentor_create_segmentor(const char * baseline_model_path,
                                  const char * model_path,
                                  const char * lexicon_path) {
  SegmentorWrapper * wrapper = new SegmentorWrapper();

  if (!wrapper->load_baseline(baseline_model_path)) {
    delete wrapper;
    return 0;
  }

  if (!wrapper->load_customized(model_path, lexicon_path)) {
    delete wrapper;
    return 0;
  }

  return reinterpret_cast<void *>(wrapper);
}

int segmentor_release_segmentor(void * segmentor) {
  if (!segmentor) {
    return -1;
  }
  delete reinterpret_cast<SegmentorWrapper *>(segmentor);
  return 0;
}

int segmentor_segment(void * segmentor,
    const std::string & str,
    std::vector<std::string> & words) {
  if (str.empty()) {
    return 0;
  }

  SegmentorWrapper * wrapper = 0;
  wrapper = reinterpret_cast<SegmentorWrapper *>(segmentor);
  return wrapper->segment(str.c_str(), words);
}

int segmentor_customized_segment(void * segmentor,
                                 const std::string & str,
                                 std::vector<std::string> & words) {
  if (str.empty()) {
    return 0;
  }
  SegmentorWrapper * wrapper = reinterpret_cast<SegmentorWrapper*>(segmentor);
  return wrapper->customized_segment(str, words);
}

int segmentor_customized_segment(void * parser,
                      const char * model_path,
                      const char * lexicon_path,
                      const std::string & line,
                      std::vector<std::string> & words) {
  if (line.empty()) {
    return 0;
  }
  char hash[500];
  TRACE_LOG("model_path = %s",model_path);
  TRACE_LOG("lexicon_path = %s",lexicon_path);
  sprintf(hash, "M=%s L=%s", model_path, lexicon_path);
  std::string hash_str(hash);
  TRACE_LOG("hash = %s",hash_str.c_str());
  seg::Model * customized_model = cache.get(hash_str).get();
  if (!customized_model) {
    TRACE_LOG("doesnot have. load now!");
    customized_model = SegmentorWrapper::load_model(model_path, lexicon_path);
    if (!customized_model) {
      return 0;
    }
    cache.put(hash_str, std::shared_ptr<seg::Model>(customized_model));
  } else {
    TRACE_LOG("yes it have");
  }
  SegmentorWrapper * wrapper = 0;
  wrapper = reinterpret_cast<SegmentorWrapper *>(parser);
  return wrapper->customized_segment(customized_model, line.c_str(), words);
}
