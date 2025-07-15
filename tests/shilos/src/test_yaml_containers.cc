#include "shilos.hh"
#include "shilos/dict_yaml.hh"
#include "shilos/list_yaml.hh"
#include "shilos/str_yaml.hh"
#include "shilos/vector_yaml.hh"

#include <optional>

#include <cassert>
#include <iostream>

using namespace shilos;

// Non-default-constructible element type
struct ND {
  int v;
  explicit ND(int value) : v(value) {}
};

// YAML support for ND (local to test)
inline yaml::Node to_yaml(const ND &obj) noexcept { return yaml::Node(static_cast<int64_t>(obj.v)); }

template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, ND *raw_ptr) {
  if (!node.IsScalar())
    throw yaml::TypeError("ND expects scalar");
  new (raw_ptr) ND(node.as<int>());
}
// -----------------------------------------------------------------------------
// Roots for memory regions
// -----------------------------------------------------------------------------
struct ListRoot {
  static const UUID TYPE_UUID;
  regional_fifo<int> fifo_;
  explicit ListRoot(memory_region<ListRoot> &mr) : fifo_(mr) {}
};
const UUID ListRoot::TYPE_UUID = UUID("aaaaaaaa-bbbb-cccc-dddd-111111111111");

struct DictRoot {
  static const UUID TYPE_UUID;
  regional_dict<regional_str, int> dict_;
  explicit DictRoot(memory_region<DictRoot> &mr) : dict_(mr) {}
};
const UUID DictRoot::TYPE_UUID = UUID("bbbbbbbb-cccc-dddd-eeee-222222222222");

// Root for ND tests
struct NDRoot {
  static const UUID TYPE_UUID;
  regional_fifo<ND> fifo_;
  regional_dict<regional_str, ND> dict_;
  explicit NDRoot(memory_region<NDRoot> &mr) : fifo_(mr), dict_(mr) {}
};
const UUID NDRoot::TYPE_UUID = UUID("cccccccc-dddd-eeee-ffff-333333333333");
// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static void assert_fifo_equals(const regional_fifo<int> &fifo, std::initializer_list<int> expected) {
  assert(fifo.size() == expected.size());
  auto it = fifo.begin();
  for (int v : expected) {
    assert(*it == v);
    ++it;
  }
}

// -----------------------------------------------------------------------------
int main() {
  std::cout << "Starting YAML container serialisation tests..." << std::endl;
  try {
    // ======================= FIFO<int> =========================
    yaml::Node int_seq(yaml::Sequence{});
    int_seq.push_back(10);
    int_seq.push_back(20);
    int_seq.push_back(30);

    auto_region<ListRoot> list_region(1024 * 1024);
    auto fifo_gp = fifo_from_yaml<int>(*list_region, int_seq);

    assert_fifo_equals(*fifo_gp, {10, 20, 30});

    yaml::Node fifo_out = to_yaml(*fifo_gp);
    assert(fifo_out.IsSequence() && fifo_out.size() == 3);
    assert(fifo_out[0].as<int>() == 10);
    assert(fifo_out[1].as<int>() == 20);
    assert(fifo_out[2].as<int>() == 30);

    std::cout << "FIFO<int> YAML tests passed!" << std::endl;

    // ======================= Dict<regional_str,int> =========================
    yaml::Node map_node(yaml::Map{});
    map_node[std::string_view("alice")] = 1;
    map_node[std::string_view("bob")] = 2;

    auto_region<DictRoot> dict_region(1024 * 1024);
    auto dict_gp = dict_from_yaml<regional_str, int, std::hash<regional_str>>(*dict_region, map_node);

    assert(dict_gp->size() == 2);
    assert(dict_gp->at("alice") == 1);
    assert(dict_gp->at("bob") == 2);

    yaml::Node dict_out = to_yaml(*dict_gp);
    assert(dict_out.IsMap());
    assert(dict_out["alice"].as<int>() == 1);
    assert(dict_out["bob"].as<int>() == 2);

    std::cout << "Dict<regional_str,int> YAML tests passed!" << std::endl;

    // ===================== FIFO<ND> non-default =====================
    yaml::Node nd_seq(yaml::Sequence{});
    nd_seq.push_back(7);
    nd_seq.push_back(9);

    auto_region<NDRoot> nd_region(1024 * 1024);
    auto nd_fifo_gp = fifo_from_yaml<ND>(*nd_region, nd_seq);
    assert(nd_fifo_gp->size() == 2);
    {
      auto it = nd_fifo_gp->begin();
      assert(it->v == 7);
      ++it;
      assert(it->v == 9);
    }

    yaml::Node nd_fifo_out = to_yaml(*nd_fifo_gp);
    assert(nd_fifo_out.IsSequence() && nd_fifo_out.size() == 2);
    assert(nd_fifo_out[0].as<int>() == 7);
    assert(nd_fifo_out[1].as<int>() == 9);

    // ===================== Dict<regional_str,ND> =====================
    yaml::Node nd_map(yaml::Map{});
    nd_map["x"] = 5;
    nd_map["y"] = 8;

    auto nd_dict_gp = dict_from_yaml<regional_str, ND, std::hash<regional_str>>(*nd_region, nd_map);
    assert(nd_dict_gp->size() == 2);
    assert(nd_dict_gp->at("x").v == 5);
    assert(nd_dict_gp->at("y").v == 8);

    yaml::Node nd_dict_out = to_yaml(*nd_dict_gp);
    assert(nd_dict_out.IsMap());
    assert(nd_dict_out["x"].as<int>() == 5);
    assert(nd_dict_out["y"].as<int>() == 8);

    std::cout << "Non-default-constructible ND YAML tests passed!" << std::endl;

    std::cout << "✅ YAML container serialisation tests passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
