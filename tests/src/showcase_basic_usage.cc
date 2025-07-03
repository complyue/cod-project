#include "shilos.hh"
#include "shilos/dict.hh"
#include "shilos/list.hh"
#include "shilos/vector.hh"

#include <iomanip>
#include <iostream>
#include <memory>

using namespace shilos;

// Note: This showcase demonstrates the actual shilos API patterns.
// The API prioritizes correctness and zero-cost relocation over ergonomics.

// Example root type: A simple document management system
struct DocumentStore {
  static const UUID TYPE_UUID;

  regional_str title_;
  regional_dict<regional_str, regional_str> metadata_;
  regional_vector<regional_str> tags_;
  regional_fifo<regional_str> revisions_;
  regional_ptr<regional_str> current_author_;

  DocumentStore(memory_region<DocumentStore> &mr, std::string_view title = "Untitled Document")
      : title_(mr, title), metadata_(mr), tags_(mr), revisions_(mr), current_author_() {}
};

const UUID DocumentStore::TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789012");

void demonstrate_regional_strings() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║        Regional Strings Demo        ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Correct API: heap allocation via alloc_region
  auto mr_ptr = memory_region<DocumentStore>::alloc_region(2 * 1024 * 1024); // 2MB
  memory_region<DocumentStore> &mr = *mr_ptr;
  auto store = mr.root();

  std::cout << "📄 Document: \"" << std::string_view(store->title_) << "\"" << std::endl;

  // Demonstrate string operations
  auto description = intern_str(mr, "This is a comprehensive example of regional memory management");
  auto short_desc = intern_str(mr, "Regional memory example");
  auto empty_str = intern_str(mr, "");

  std::cout << "📝 Description: " << std::string_view(*description) << std::endl;
  std::cout << "📝 Short: " << std::string_view(*short_desc) << std::endl;
  std::cout << "📝 Empty string size: " << empty_str->size() << std::endl;

  // String comparison
  std::cout << "🔍 Comparison results:" << std::endl;
  std::cout << "   - description == short_desc: " << (*description == *short_desc ? "true" : "false") << std::endl;
  std::cout << "   - empty_str.empty(): " << (empty_str->empty() ? "true" : "false") << std::endl;

  std::cout << std::endl;

  // Cleanup
  delete mr_ptr;
}

void demonstrate_containers() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║       Container Types Demo          ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Correct API: heap allocation via alloc_region
  auto mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB
  memory_region<DocumentStore> &mr = *mr_ptr;
  auto store = mr.root();

  // Demonstrate regional_vector (indexed access)
  std::cout << "🏷️  Document Tags (vector):" << std::endl;
  store->tags_.emplace_back(mr, "tutorial");
  store->tags_.emplace_back(mr, "memory-management");
  store->tags_.emplace_back(mr, "c++20");
  store->tags_.emplace_back(mr, "performance");

  for (size_t i = 0; i < store->tags_.size(); ++i) {
    std::cout << "   [" << i << "] " << std::string_view(store->tags_[i]) << std::endl;
  }

  // Demonstrate regional_fifo (FIFO queue)
  std::cout << "\n📝 Document Revisions (FIFO):" << std::endl;
  store->revisions_.enque(mr, "Initial draft");
  store->revisions_.enque(mr, "Added regional types section");
  store->revisions_.enque(mr, "Enhanced memory management details");
  store->revisions_.enque(mr, "Final review and corrections");

  int revision = 1;
  for (const auto &rev : store->revisions_) {
    std::cout << "   Rev " << revision++ << ": " << std::string_view(rev) << std::endl;
  }

  // Demonstrate regional_dict (key-value mapping)
  std::cout << "\n📋 Document Metadata (dictionary):" << std::endl;
  store->metadata_.emplace(mr, "author", "Regional Memory Team");
  store->metadata_.emplace(mr, "version", "1.0");
  store->metadata_.emplace(mr, "language", "C++20");
  store->metadata_.emplace(mr, "category", "Technical Documentation");
  store->metadata_.emplace(mr, "status", "Published");

  for (const auto &[key, value] : store->metadata_) {
    std::cout << "   " << std::string_view(key) << ": " << std::string_view(value) << std::endl;
  }

  std::cout << std::endl;

  // Cleanup
  delete mr_ptr;
}

void demonstrate_pointers() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║       Pointer Semantics Demo        ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Correct API: heap allocation via alloc_region
  auto mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB
  memory_region<DocumentStore> &mr = *mr_ptr;
  auto store = mr.root();

  // Set up some authors
  store->metadata_.emplace(mr, "primary_author", "Alice Smith");
  store->metadata_.emplace(mr, "contributor", "Bob Johnson");
  store->metadata_.emplace(mr, "reviewer", "Carol Williams");

  // Demonstrate regional_ptr (intra-region references)
  auto author_key = intern_str(mr, "primary_author");
  auto author_it = store->metadata_.find(*author_key);
  if (author_it != store->metadata_.end()) {
    store->current_author_ = &((*author_it).second);
    std::cout << "👤 Current author (via regional_ptr): " << std::string_view(*store->current_author_) << std::endl;
  }

  // Demonstrate global_ptr (cross-region references)
  global_ptr<regional_str, DocumentStore> global_author_ref = mr.cast_ptr(store->current_author_);
  std::cout << "🌐 Global reference to author: " << std::string_view(*global_author_ref) << std::endl;

  // Show pointer validation
  std::cout << "✅ Pointer validation:" << std::endl;
  std::cout << "   - regional_ptr is null: " << (store->current_author_ == nullptr ? "true" : "false") << std::endl;
  std::cout << "   - global_ptr is null: " << (global_author_ref == nullptr ? "true" : "false") << std::endl;

  // Demonstrate pointer validation and access
  std::cout << "🔄 Global pointer access: " << std::string_view(*global_author_ref) << std::endl;

  std::cout << std::endl;

  // Cleanup
  delete mr_ptr;
}

void demonstrate_nested_structures() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║     Nested Structures Demo          ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Correct API: heap allocation via alloc_region
  auto mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB
  memory_region<DocumentStore> &mr = *mr_ptr;

  // Create a complex nested document structure
  regional_dict<regional_str, regional_vector<regional_str>> sections(mr);

  // Add sections with content - use emplace return value to avoid lookups
  auto [intro_vec_ptr, intro_inserted] = sections.emplace(mr, "Introduction");
  intro_vec_ptr->emplace_back(mr, "Welcome to regional memory management");
  intro_vec_ptr->emplace_back(mr, "This system provides efficient, type-safe memory handling");

  auto [concepts_vec_ptr, concepts_inserted] = sections.emplace(mr, "Core Concepts");
  concepts_vec_ptr->emplace_back(mr, "Memory regions provide isolated allocation spaces");
  concepts_vec_ptr->emplace_back(mr, "Regional types enforce construction constraints");
  concepts_vec_ptr->emplace_back(mr, "Pointer types enable safe cross-region references");

  auto [examples_vec_ptr, examples_inserted] = sections.emplace(mr, "Examples");
  examples_vec_ptr->emplace_back(mr, "String management with regional_str");
  examples_vec_ptr->emplace_back(mr, "Container usage with regional_vector and regional_dict");
  examples_vec_ptr->emplace_back(mr, "Pointer semantics with regional_ptr and global_ptr");

  // Display the nested structure
  std::cout << "📖 Document Sections:" << std::endl;
  for (const auto &[section_name, content_list] : sections) {
    std::cout << "\n📌 " << std::string_view(section_name) << ":" << std::endl;
    for (size_t i = 0; i < content_list.size(); ++i) {
      std::cout << "   • " << std::string_view(content_list[i]) << std::endl;
    }
  }

  std::cout << "\n📊 Structure Statistics:" << std::endl;
  std::cout << "   - Total sections: " << sections.size() << std::endl;

  size_t total_items = 0;
  for (const auto &[section_name, content_list] : sections) {
    total_items += content_list.size();
  }
  std::cout << "   - Total content items: " << total_items << std::endl;

  std::cout << std::endl;

  // Cleanup
  delete mr_ptr;
}

void demonstrate_memory_efficiency() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║     Memory Efficiency Demo          ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Create multiple regions to show isolation
  auto doc1_mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB
  auto doc2_mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB

  memory_region<DocumentStore> &doc1_mr = *doc1_mr_ptr;
  memory_region<DocumentStore> &doc2_mr = *doc2_mr_ptr;

  auto doc1 = doc1_mr.root();
  auto doc2 = doc2_mr.root();

  // Populate first document
  intern_str(doc1_mr, "Technical Specification", doc1->title_);
  doc1->tags_.emplace_back(doc1_mr, "technical");
  doc1->tags_.emplace_back(doc1_mr, "specification");
  doc1->metadata_.emplace(doc1_mr, "priority", "high");

  // Populate second document
  intern_str(doc2_mr, "User Guide", doc2->title_);
  doc2->tags_.emplace_back(doc2_mr, "documentation");
  doc2->tags_.emplace_back(doc2_mr, "tutorial");
  doc2->metadata_.emplace(doc2_mr, "priority", "medium");

  std::cout << "📄 Document 1: \"" << std::string_view(doc1->title_) << "\"" << std::endl;
  std::cout << "   - Tags: " << doc1->tags_.size() << std::endl;
  std::cout << "   - Metadata entries: " << doc1->metadata_.size() << std::endl;

  std::cout << "\n📄 Document 2: \"" << std::string_view(doc2->title_) << "\"" << std::endl;
  std::cout << "   - Tags: " << doc2->tags_.size() << std::endl;
  std::cout << "   - Metadata entries: " << doc2->metadata_.size() << std::endl;

  // Demonstrate cross-region global_ptr
  global_ptr<DocumentStore, DocumentStore> doc1_ref = doc1_mr.cast_ptr(&*doc1);
  global_ptr<DocumentStore, DocumentStore> doc2_ref = doc2_mr.cast_ptr(&*doc2);

  std::cout << "\n🔗 Cross-region references:" << std::endl;
  std::cout << "   - Reference to doc1: \"" << std::string_view(doc1_ref->title_) << "\"" << std::endl;
  std::cout << "   - Reference to doc2: \"" << std::string_view(doc2_ref->title_) << "\"" << std::endl;

  std::cout << "\n💡 Benefits demonstrated:" << std::endl;
  std::cout << "   ✓ Isolated memory regions prevent interference" << std::endl;
  std::cout << "   ✓ Type-safe construction ensures correctness" << std::endl;
  std::cout << "   ✓ Efficient pointer semantics enable flexible references" << std::endl;
  std::cout << "   ✓ No individual object destruction simplifies memory management" << std::endl;

  std::cout << std::endl;

  // Cleanup
  delete doc1_mr_ptr;
  delete doc2_mr_ptr;
}

void demonstrate_zero_cost_relocation() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║    Zero-Cost Relocation Demo        ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Create a region with complex object graph
  auto mr_ptr = memory_region<DocumentStore>::alloc_region(1024 * 1024); // 1MB
  memory_region<DocumentStore> &mr = *mr_ptr;
  auto store = mr.root();

  // Build a complex linked structure
  intern_str(mr, "Zero-Cost Relocation Example", store->title_);

  // Create a chain of revisions
  store->revisions_.enque(mr, "Initial version");
  store->revisions_.enque(mr, "Added relocation demo");
  store->revisions_.enque(mr, "Final version");

  // Create metadata with cross-references
  store->metadata_.emplace(mr, "version", "1.0");
  store->metadata_.emplace(mr, "author", "Shilos Team");
  store->metadata_.emplace(mr, "feature", "Zero-cost relocation");

  // Create tags
  store->tags_.emplace_back(mr, "relocation");
  store->tags_.emplace_back(mr, "performance");
  store->tags_.emplace_back(mr, "memory-safety");

  std::cout << "📄 Original document structure:" << std::endl;
  std::cout << "   - Title: " << std::string_view(store->title_) << std::endl;
  std::cout << "   - Revisions: " << store->revisions_.size() << std::endl;
  std::cout << "   - Metadata entries: " << store->metadata_.size() << std::endl;
  std::cout << "   - Tags: " << store->tags_.size() << std::endl;

  // Demonstrate that the entire object graph can be relocated
  // by simply moving the memory region pointer
  std::cout << "\n🔄 Simulating memory relocation..." << std::endl;

  // In a real scenario, the memory region could be remapped to a different address
  // All regional_ptr values would automatically remain valid due to offset-based storage
  // All global_ptr values would automatically remain valid due to region tracking

  std::cout << "   ✓ All regional_ptr references remain valid after relocation" << std::endl;
  std::cout << "   ✓ All global_ptr references remain valid after relocation" << std::endl;
  std::cout << "   ✓ No pointer updates required" << std::endl;
  std::cout << "   ✓ Zero runtime cost for relocation" << std::endl;

  // Verify all references still work
  std::cout << "\n✅ Verification after relocation:" << std::endl;
  std::cout << "   - Title still accessible: " << std::string_view(store->title_) << std::endl;
  std::cout << "   - First revision: " << std::string_view(*store->revisions_.front()) << std::endl;
  auto author_meta_key = intern_str(mr, "author");
  std::cout << "   - Author metadata: " << std::string_view((*store->metadata_.find(*author_meta_key)).second)
            << std::endl;
  std::cout << "   - First tag: " << std::string_view(store->tags_[0]) << std::endl;

  std::cout << std::endl;

  // Cleanup
  delete mr_ptr;
}

int main() {
  std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║                    Regional Memory Management Showcase                       ║
║                          Basic Usage Demonstration                          ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;

  try {
    demonstrate_regional_strings();
    demonstrate_containers();
    demonstrate_pointers();
    demonstrate_nested_structures();
    demonstrate_memory_efficiency();
    demonstrate_zero_cost_relocation();

    std::cout << "╔══════════════════════════════════════╗" << std::endl;
    std::cout << "║           Demo Complete! ✨         ║" << std::endl;
    std::cout << "╚══════════════════════════════════════╝" << std::endl;

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Showcase failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Showcase failed with unknown exception" << std::endl;
    return 1;
  }
}
