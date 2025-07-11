#include "shilos.hh"
#include "shilos/str_yaml.hh"
#include "shilos/vector_yaml.hh"

#include <cassert>
#include <iostream>

using namespace shilos;

// -----------------------------------------------------------------------------
// Test root type required by memory_region
// -----------------------------------------------------------------------------
struct YamlTestRoot {
  static const UUID TYPE_UUID;

  // Data member we are going to (de)serialise
  regional_vector<regional_str> vec_;

  explicit YamlTestRoot(memory_region<YamlTestRoot> &mr) : vec_(mr) {}
};

const UUID YamlTestRoot::TYPE_UUID = UUID("cccccccc-dddd-eeee-ffff-333333333333");

// Nested root for vector-of-vector test
struct NestedRoot {
  static const UUID TYPE_UUID;
  regional_vector<regional_vector<regional_str>> vv_;
  explicit NestedRoot(memory_region<NestedRoot> &mr) : vv_(mr) {}
};

const UUID NestedRoot::TYPE_UUID = UUID("dddddddd-eeee-ffff-aaaa-444444444444");

// Root for bits-type vector test
struct BitsRoot {
  static const UUID TYPE_UUID;
  regional_vector<int> ints_;
  explicit BitsRoot(memory_region<BitsRoot> &mr) : ints_(mr) {}
};

const UUID BitsRoot::TYPE_UUID = UUID("eeeeeeee-ffff-aaaa-bbbb-555555555555");

// -----------------------------------------------------------------------------
// Helper assertions
// -----------------------------------------------------------------------------
static void assert_vector_equals(const regional_vector<regional_str> &vec,
                                 std::initializer_list<std::string_view> expected) {
  assert(vec.size() == expected.size());
  size_t idx = 0;
  for (auto exp : expected) {
    assert(vec[idx] == exp);
    ++idx;
  }
}

// -----------------------------------------------------------------------------
// Main test routine
// -----------------------------------------------------------------------------
int main() {
  std::cout << "Starting YAML serialisation tests..." << std::endl;

  try {
    // 1. Build an input YAML sequence
    yaml::Node seq(yaml::Sequence{});
    seq.push_back("alpha");
    seq.push_back("beta");
    seq.push_back("gamma");

    // 2. Create a region and deserialise the vector via helper
    auto_region<YamlTestRoot> region(1024 * 1024);

    // Convenience helper returning global_ptr
    auto vec_gp = vector_from_yaml<regional_str>(*region, seq);

    // Verify content
    assert_vector_equals(*vec_gp, {"alpha", "beta", "gamma"});

    // 3. Round-trip: serialise back to YAML and compare
    yaml::Node out = to_yaml(*vec_gp);
    assert(out.IsSequence());
    assert(out.size() == 3);
    assert(out[0].as<std::string>() == "alpha");
    assert(out[1].as<std::string>() == "beta");
    assert(out[2].as<std::string>() == "gamma");

    // ======================= Nested vector test =========================

    // Build YAML sequence of sequences: [["x","y"], ["z"]]
    yaml::Node nested(yaml::Sequence{});
    yaml::Node sub1(yaml::Sequence{});
    sub1.push_back("x");
    sub1.push_back("y");
    yaml::Node sub2(yaml::Sequence{});
    sub2.push_back("z");
    nested.push_back(sub1);
    nested.push_back(sub2);

    // Use NestedRoot defined above
    auto_region<NestedRoot> nested_region(1024 * 1024);
    auto outer_gp = vector_from_yaml<regional_vector<regional_str>>(*nested_region, nested);

    // Validate sizes and content
    assert(outer_gp->size() == 2);
    assert((*outer_gp)[0].size() == 2);
    assert((*outer_gp)[1].size() == 1);
    assert((*outer_gp)[0][0] == "x");
    assert((*outer_gp)[0][1] == "y");
    assert((*outer_gp)[1][0] == "z");

    // Round-trip again
    yaml::Node nested_out = to_yaml(*outer_gp);
    assert(nested_out.IsSequence());
    assert(nested_out.size() == 2);
    assert(nested_out[0].IsSequence() && nested_out[0].size() == 2);
    assert(nested_out[1].IsSequence() && nested_out[1].size() == 1);
    assert(nested_out[0][0].as<std::string>() == "x");
    assert(nested_out[0][1].as<std::string>() == "y");
    assert(nested_out[1][0].as<std::string>() == "z");

    std::cout << "Nested vector YAML tests passed!" << std::endl;

    // ======================= Bits-type vector test =========================

    yaml::Node int_seq(yaml::Sequence{});
    int_seq.push_back(1);
    int_seq.push_back(2);
    int_seq.push_back(3);

    auto_region<BitsRoot> bits_region(1024 * 1024);
    auto ints_gp = vector_from_yaml<int>(*bits_region, int_seq);

    assert(ints_gp->size() == 3);
    assert((*ints_gp)[0] == 1);
    assert((*ints_gp)[1] == 2);
    assert((*ints_gp)[2] == 3);

    yaml::Node int_out = to_yaml(*ints_gp);
    assert(int_out.IsSequence() && int_out.size() == 3);
    assert(int_out[0].as<int>() == 1);
    assert(int_out[1].as<int>() == 2);
    assert(int_out[2].as<int>() == 3);

    std::cout << "Bits-type vector YAML tests passed!" << std::endl;

    std::cout << "✅ YAML serialisation tests passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
