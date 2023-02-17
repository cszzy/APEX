#pragma once

#include "tree.h"
#include "util/utils.h"

#include "tbb/tbb.h"
#include <utility>
#include <map>
#include <cstdint>
#include "nali_alloc.h"
#include "../src/apex.h"

template<class T, class P>
class Tree;

//used to define the interface of all benchmarking trees
// the param `epoch` is uesd in apex
template <class T, class P>
class Tree {
 public:
  typedef std::pair<T, P> V;
  virtual void bulk_load(const V[], int) = 0;
  virtual bool insert(const T&, const P&) = 0;
  virtual bool search(const T&, P&) = 0;
  virtual bool erase(const T&, uint64_t *log_offset = nullptr) = 0;
  virtual bool update(const T&, const P&, uint64_t *log_offset = nullptr) = 0;
  // Return #keys really scanned
  virtual int range_scan_by_size(const T&, uint32_t, V*& resul) = 0;

  virtual void get_info() = 0;
};

extern int parallel_merge_worker_num; // param for dptree
extern PMEMobjpool **pop;
namespace nali {
    template <class T, class P>
    class apex_db : public Tree<T, P> {
        public:
            typedef std::pair<T, P> V;
            apex_db() {
                init_numa_map();
                db_ = new alex::Apex<T, P>();
            }

            ~apex_db() {
                delete db_;
            }

            void bulk_load(const V values[], int num_keys) {
                db_->bulk_load(values, num_keys);
            }

            bool insert(const T& key, const P& payload) {
                return db_->insert(key, payload);
            }

            bool search(const T& key, P &payload) {
                return db_->search(key, &payload);
            }

            bool erase(const T& key, uint64_t *log_offset = nullptr) {
                return db_->erase(key);
            }

            bool update(const T& key, const P& payload, uint64_t *log_offset = nullptr) {
                return db_->update(key, payload);
            }

            int range_scan_by_size(const T& key, uint32_t to_scan, V* &result = nullptr) {
                return db_->range_scan_by_size(key, to_scan, result);
            }

            void get_info() {
            }

        private:
            alex::Apex<T, P> *db_;
    };
}

