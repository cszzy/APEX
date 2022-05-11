#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <future>
#include "../ycsb/ycsb-c.h"

#include "../core/apex.h"
#include "mbrandom.h"

using namespace std;

namespace KV
{
  class Key_t
  {
    typedef std::array<double, 1> model_key_t;

  public:
    static constexpr size_t model_key_size() { return 1; }
    static Key_t max()
    {
      static Key_t max_key(std::numeric_limits<uint64_t>::max());
      return max_key;
    }
    static Key_t min()
    {
      static Key_t min_key(std::numeric_limits<uint64_t>::min());
      return min_key;
    }

    Key_t() : key(0) {}
    Key_t(uint64_t key) : key(key) {}
    Key_t(const Key_t &other) { key = other.key; }
    Key_t &operator=(const Key_t &other)
    {
      key = other.key;
      return *this;
    }

    model_key_t to_model_key() const
    {
      model_key_t model_key;
      model_key[0] = key;
      return model_key;
    }

    friend bool operator<(const Key_t &l, const Key_t &r) { return l.key < r.key; }
    friend bool operator>(const Key_t &l, const Key_t &r) { return l.key > r.key; }
    friend bool operator>=(const Key_t &l, const Key_t &r) { return l.key >= r.key; }
    friend bool operator<=(const Key_t &l, const Key_t &r) { return l.key <= r.key; }
    friend bool operator==(const Key_t &l, const Key_t &r) { return l.key == r.key; }
    friend bool operator!=(const Key_t &l, const Key_t &r) { return l.key != r.key; }

    uint64_t key;
  };
}

namespace dbInter
{

  static inline std::string human_readable(double size)
  {
    static const std::string suffix[] = {
        "B",
        "KB",
        "MB",
        "GB"};
    const int arr_len = 4;

    std::ostringstream out;
    out.precision(2);
    for (int divs = 0; divs < arr_len; ++divs)
    {
      if (size >= 1024.0)
      {
        size /= 1024.0;
      }
      else
      {
        out << std::fixed << size;
        return out.str() + suffix[divs];
      }
    }
    out << std::fixed << size;
    return out.str() + suffix[arr_len - 1];
  }

  static const char* pool_name = "/mnt/AEP0/template.data";
  static const uint64_t pool_size = 80UL * 1024*1024*1024;

  class ApexDB : public ycsbc::KvDB
  {
    typedef Tree<uint64_t, uint64_t> apex_t;

  public:
    ApexDB() : apex_(nullptr) {}
    ApexDB(apex_t *apex) : apex_(apex) {}
    virtual ~ApexDB()
    {
      delete apex_;
    }

    void Init()
    {
        apex_ = generate_index<uint64_t, uint64_t>();
    }

    void Bulk_load(const std::pair<uint64_t, uint64_t> data[], int size)
    {
      apex_->bulk_load(data, size);
    }

    void Info()
    {
      apex_->get_depth_info();
    }

    int Put(uint64_t key, uint64_t value)
    {
      apex_->insert(key, value);
      return 1;
    }
    int Get(uint64_t key, uint64_t &value)
    {
      apex_->search(key, &value);
      // assert(value == key);
      return 1;
    }
    int Update(uint64_t key, uint64_t value)
    {
      apex_->update(key, value);
      return 1;
    }
    int Delete(uint64_t key)
    {
      apex_->erase(key);
      return 1;
    }
    int Scan(uint64_t start_key, int len, std::vector<std::pair<uint64_t, uint64_t>> &results)
    {
        std::pair<uint64_t, uint64_t> *res = nullptr;
        int num_res = apex_->range_scan_by_size(start_key, len, res);
        int i = 0;
        while(num_res--) {
            results.push_back(res[i++]);
        }
        return 1;
    }
    void PrintStatic()
    {
      // std::cerr << "Alevel average cost: " << Common::timers["ABLevel_times"].avg_latency() << std::endl;
      // std::cerr << "Clevel average cost: " << Common::timers["CLevel_times"].avg_latency() << std::endl;
        apex_->get_depth_info();
    }

  private:

  template<class T, class P>
  Tree<T, P>* generate_index(){
    Tree<T, P> *index = nullptr;
    
    bool recover = my_alloc::BasePMPool::Initialize(pool_name, pool_size);
    auto index_ptr = reinterpret_cast<Tree<T, P> **>(my_alloc::BasePMPool::GetRoot(sizeof(Tree<T, P>*)));
    if (recover)
    {
      assert(false);
    }else{ 
      my_alloc::BasePMPool::ZAllocate(reinterpret_cast<void**>(index_ptr), sizeof(apex::Apex<T, P>) + 64);
      index = reinterpret_cast<Tree<T, P>*>(reinterpret_cast<char*>(*index_ptr) + 48);     
      new (index) apex::Apex<T, P>();
    }

    return index;
  }

  private:
    apex_t *apex_;
  };

} //namespace dbInter
