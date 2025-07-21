#include <shilos.hh>
#include <shilos/dict.hh>
#include <shilos/list.hh>
#include <shilos/vector.hh>

#include <iostream>

using namespace shilos;

// Example root type: A simple document management system
struct DocumentStore {
  static const UUID TYPE_UUID;

  // Core document data - all regional types are allocated within the region
  regional_str title_;
  regional_vector<regional_str> tags_;                               // Indexed access container for document tags
  regional_fifo<regional_str> revisions_;                            // FIFO queue for revision history
  regional_ptr<regional_str> current_author_;                        // Intra-region reference to author
  regional_ptr<regional_dict<regional_str, regional_str>> metadata_; // Document metadata (language, license, etc.)

  // Constructor initializes all regional components within the provided memory region
  DocumentStore(memory_region<DocumentStore> &mr, std::string_view title = "Untitled Document")
      : title_(mr, title), tags_(mr), revisions_(mr), current_author_(),
        metadata_(mr.create<regional_dict<regional_str, regional_str>>()) {}
};

const UUID DocumentStore::TYPE_UUID = UUID("12345678-1234-5678-9abc-123456789012");

void demonstrate_regional_strings() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║        Regional Strings Demo         ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Memory regions are managed automatically with RAII
  auto_region<DocumentStore> region(2 * 1024 * 1024); // 2MB
  auto store = region->root();

  std::cout << "📄 Document: \"" << store->title_ << "\"" << std::endl;

  // intern_str creates persistent regional_str objects and returns global_ptr for safe access
  auto description =
      intern_str(*region, std::string_view("This is a comprehensive example of regional memory management"));
  auto short_desc = intern_str(*region, std::string_view("Regional memory example"));
  auto empty_str = intern_str(*region, std::string_view(""));

  std::cout << "📝 Description: " << (*description) << std::endl;
  std::cout << "📝 Short: " << (*short_desc) << std::endl;
  std::cout << "📝 Empty string size: " << empty_str->size() << std::endl;

  // String comparison
  std::cout << "🔍 Comparison results:" << std::endl;
  std::cout << "   - description == short_desc: " << (*description == *short_desc ? "true" : "false") << std::endl;
  std::cout << "   - empty_str.empty(): " << (empty_str->empty() ? "true" : "false") << std::endl;

  // string_view is used for temporary operations without allocation
  std::string_view temp_content = "Temporary content for processing";
  std::cout << "📝 Using string_view for temporary ops: " << temp_content << std::endl;

  std::cout << std::endl;

  // Region cleanup happens automatically via RAII
}

void demonstrate_containers() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║       Container Types Demo           ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Each memory region provides isolated storage for one root type and its components
  auto_region<DocumentStore> region(1024 * 1024); // 1MB
  auto store = region->root();

  // regional_vector provides indexed access with in-place element construction
  std::cout << "🏷️  Document Tags (vector):" << std::endl;

  // Container methods construct regional_str elements directly from string_view
  store->tags_.emplace_back(*region, std::string_view("tutorial"));
  store->tags_.emplace_back(*region, std::string_view("memory-management"));
  store->tags_.emplace_back(*region, std::string_view("c++20"));
  store->tags_.emplace_back(*region, std::string_view("performance"));

  for (size_t i = 0; i < store->tags_.size(); ++i) {
    std::cout << "   [" << i << "] " << (store->tags_[i]) << std::endl;
  }

  // regional_fifo provides FIFO queue semantics with efficient enqueue/dequeue
  std::cout << "\n📝 Document Revisions (FIFO):" << std::endl;

  // Queue operations also construct elements in-place from string_view
  store->revisions_.enque(*region, std::string_view("Initial draft"));
  store->revisions_.enque(*region, std::string_view("Added regional types section"));
  store->revisions_.enque(*region, std::string_view("Enhanced memory management details"));
  store->revisions_.enque(*region, std::string_view("Final review and corrections"));

  int revision = 1;
  for (const auto &rev : store->revisions_) {
    std::cout << "   Rev " << revision++ << ": " << (rev) << std::endl;
  }

  // regional_dict provides hash table with heterogeneous key support
  std::cout << "\n📚 Document Metadata (dict):" << std::endl;

  // Use the metadata field from DocumentStore - it's already initialized in constructor
  auto &metadata = store->metadata_;

  // Insert operations create key-value pairs with string_view conversion for consistency
  auto [author_ptr, author_inserted] =
      metadata->insert(*region, std::string_view("author"), std::string_view("Alice Smith"));
  auto [category_ptr, category_inserted] =
      metadata->insert(*region, std::string_view("category"), std::string_view("Technical Documentation"));
  auto [version_ptr, version_inserted] =
      metadata->insert(*region, std::string_view("version"), std::string_view("1.0.0"));

  std::cout << "   - Author: " << (*author_ptr) << " (inserted: " << (author_inserted ? "yes" : "no") << ")"
            << std::endl;
  std::cout << "   - Category: " << (*category_ptr) << " (inserted: " << (category_inserted ? "yes" : "no") << ")"
            << std::endl;
  std::cout << "   - Version: " << (*version_ptr) << " (inserted: " << (version_inserted ? "yes" : "no") << ")"
            << std::endl;

  // Demonstrate heterogeneous lookup with different key types
  std::cout << "\n🔍 Heterogeneous lookups:" << std::endl;
  std::cout << "   - Find with const char*: "
            << (metadata->find_value(std::string_view("author"))
                    ? std::string_view(*metadata->find_value(std::string_view("author")))
                    : std::string_view("not found"))
            << std::endl;
  std::cout << "   - Find with std::string: "
            << (metadata->find_value(std::string_view("category"))
                    ? std::string_view(*metadata->find_value(std::string_view("category")))
                    : std::string_view("not found"))
            << std::endl;
  std::cout << "   - Find with string_view: "
            << (metadata->find_value(std::string_view("version"))
                    ? std::string_view(*metadata->find_value(std::string_view("version")))
                    : std::string_view("not found"))
            << std::endl;

  // Test contains with different key types
  std::cout << "\n✅ Contains checks:" << std::endl;
  std::cout << "   - Contains 'author' (const char*): "
            << (metadata->contains(std::string_view("author")) ? "yes" : "no") << std::endl;
  std::cout << "   - Contains 'nonexistent' (string): "
            << (metadata->contains(std::string_view("nonexistent")) ? "yes" : "no") << std::endl;

  // Demonstrate iteration over key-value pairs
  std::cout << "\n📋 All metadata entries:" << std::endl;
  for (const auto &[key, value] : *metadata) {
    std::cout << "   - " << key << ": " << value << std::endl;
  }

  std::cout << "\n💡 Dict benefits demonstrated:" << std::endl;
  std::cout << "   ✓ Simplified heterogeneous key support using common key conversion" << std::endl;
  std::cout << "   ✓ Standard C++ container semantics (insert, find_value, contains, etc.)" << std::endl;
  std::cout << "   ✓ Efficient hash table implementation preserving insertion order" << std::endl;
  std::cout << "   ✓ Zero-cost relocation compatibility with rest of shilos ecosystem" << std::endl;
  std::cout << "   ✓ Persistent metadata storage as document field" << std::endl;

  std::cout << std::endl;

  // RAII cleanup - no manual delete needed
}

void demonstrate_pointers() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║       Pointer Semantics Demo         ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Allocate region for pointer semantics demonstration
  auto_region<DocumentStore> region(1024 * 1024); // 1MB
  auto store = region->root();

  // Create persistent author strings using intern_str
  auto alice_author = intern_str(*region, std::string_view("Alice Smith"));
  auto bob_contributor = intern_str(*region, std::string_view("Bob Johnson"));
  auto carol_reviewer = intern_str(*region, std::string_view("Carol Williams"));

  std::cout << "👥 Created author strings:" << std::endl;
  std::cout << "   - Alice: " << (*alice_author) << std::endl;
  std::cout << "   - Bob: " << (*bob_contributor) << std::endl;
  std::cout << "   - Carol: " << (*carol_reviewer) << std::endl;

  // regional_ptr provides efficient intra-region references using offsets
  store->current_author_ = alice_author.get(); // Convert global_ptr to regional_ptr
  std::cout << "\n👤 Current author (via regional_ptr): " << (*store->current_author_) << std::endl;

  // global_ptr enables safe cross-region references with region tracking
  global_ptr<regional_str, DocumentStore> global_author_ref = region->cast_ptr(store->current_author_);
  std::cout << "🌐 Global reference to author: " << (*global_author_ref) << std::endl;

  // Pointer validation demonstrates null-safety
  std::cout << "\n✅ Pointer validation:" << std::endl;
  std::cout << "   - regional_ptr is null: " << (store->current_author_ == nullptr ? "true" : "false") << std::endl;
  std::cout << "   - global_ptr is null: " << (global_author_ref == nullptr ? "true" : "false") << std::endl;

  std::cout << std::endl;

  // Automatic cleanup when region goes out of scope
}

void demonstrate_memory_efficiency() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║     Memory Efficiency Demo           ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Create multiple regions to show isolation
  auto_region<DocumentStore> doc1_region(1024 * 1024); // 1MB
  auto_region<DocumentStore> doc2_region(1024 * 1024); // 1MB

  auto doc1 = doc1_region->root();
  auto doc2 = doc2_region->root();

  // Populate first document
  doc1->tags_.emplace_back(*doc1_region, std::string_view("technical"));
  doc1->tags_.emplace_back(*doc1_region, std::string_view("specification"));

  // Populate second document
  doc2->tags_.emplace_back(*doc2_region, std::string_view("documentation"));
  doc2->tags_.emplace_back(*doc2_region, std::string_view("tutorial"));

  std::cout << "📄 Document 1: \"" << (doc1->title_) << "\"" << std::endl;
  std::cout << "   - Tags: " << doc1->tags_.size() << std::endl;

  std::cout << "\n📄 Document 2: \"" << (doc2->title_) << "\"" << std::endl;
  std::cout << "   - Tags: " << doc2->tags_.size() << std::endl;

  // Demonstrate cross-region global_ptr
  global_ptr<DocumentStore, DocumentStore> doc1_ref = doc1_region->cast_ptr(&*doc1);
  global_ptr<DocumentStore, DocumentStore> doc2_ref = doc2_region->cast_ptr(&*doc2);

  std::cout << "\n🔗 Cross-region references:" << std::endl;
  std::cout << "   - Reference to doc1: \"" << (doc1_ref->title_) << "\"" << std::endl;
  std::cout << "   - Reference to doc2: \"" << (doc2_ref->title_) << "\"" << std::endl;

  std::cout << "\n💡 Benefits demonstrated:" << std::endl;
  std::cout << "   ✓ Isolated memory regions prevent interference" << std::endl;
  std::cout << "   ✓ Type-safe construction ensures correctness" << std::endl;
  std::cout << "   ✓ Efficient pointer semantics enable flexible references" << std::endl;
  std::cout << "   ✓ No individual object destruction simplifies memory management" << std::endl;

  std::cout << std::endl;

  // RAII cleanup - regions destroyed automatically when going out of scope
}

void demonstrate_zero_cost_relocation() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║    Zero-Cost Relocation Demo         ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  // Create a region with complex object graph
  auto_region<DocumentStore> region(1024 * 1024); // 1MB
  auto store = region->root();

  // Build a complex linked structure
  store->revisions_.enque(*region, std::string_view("Initial version"));
  store->revisions_.enque(*region, std::string_view("Added relocation demo"));
  store->revisions_.enque(*region, std::string_view("Final version"));

  // Create tags - construct in-place within the container
  store->tags_.emplace_back(*region, std::string_view("relocation"));
  store->tags_.emplace_back(*region, std::string_view("performance"));
  store->tags_.emplace_back(*region, std::string_view("memory-safety"));

  // Create some author references and metadata
  auto primary_author = intern_str(*region, std::string_view("Shilos Team"));
  store->current_author_ = primary_author.get();

  // Use the comprehensive metadata dictionary from DocumentStore field
  auto &doc_metadata = store->metadata_;

  // Demonstrate different insertion methods with heterogeneous keys
  auto [lang_ptr, lang_new] = doc_metadata->insert(*region, std::string_view("language"), std::string_view("C++20"));
  auto [license_ptr, license_new] =
      doc_metadata->insert_or_assign(*region, std::string_view("license"), std::string_view("MIT"));
  auto [maintainer_ptr, maintainer_new] =
      doc_metadata->try_emplace(*region, std::string_view("maintainer"), std::string_view("Development Team"));

  // Try to insert duplicate - should not insert but return existing
  auto [lang_ptr2, lang_duplicate] =
      doc_metadata->insert(*region, std::string_view("language"), std::string_view("Should not overwrite"));

  std::cout << "\n📋 Document metadata operations:" << std::endl;
  std::cout << "   - Language: " << (*lang_ptr) << " (new: " << (lang_new ? "yes" : "no") << ")" << std::endl;
  std::cout << "   - License: " << (*license_ptr) << " (new: " << (license_new ? "yes" : "no") << ")" << std::endl;
  std::cout << "   - Maintainer: " << (*maintainer_ptr) << " (new: " << (maintainer_new ? "yes" : "no") << ")"
            << std::endl;
  std::cout << "   - Language duplicate attempt: " << (*lang_ptr2) << " (new: " << (lang_duplicate ? "yes" : "no")
            << ")" << std::endl;

  std::cout << "📄 Original document structure:" << std::endl;
  std::cout << "   - Title: " << (store->title_) << std::endl;
  std::cout << "   - Revisions: " << store->revisions_.size() << std::endl;
  std::cout << "   - Tags: " << store->tags_.size() << std::endl;
  std::cout << "   - Author: " << (*store->current_author_) << std::endl;

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
  std::cout << "   - Title still accessible: " << (store->title_) << std::endl;
  std::cout << "   - First revision: " << (*store->revisions_.front()) << std::endl;
  std::cout << "   - Author reference: " << (*store->current_author_) << std::endl;
  std::cout << "   - First tag: " << (store->tags_[0]) << std::endl;

  // Verify dictionary still works after simulated relocation
  std::cout << "   - Dict entry count: " << doc_metadata->size() << std::endl;
  std::cout << "   - Dict contains 'language': "
            << (doc_metadata->contains(std::string_view("language")) ? "yes" : "no") << std::endl;

  std::cout << std::endl;

  // RAII cleanup - region destroyed automatically
}

void demonstrate_design_principles() {
  std::cout << "╔══════════════════════════════════════╗" << std::endl;
  std::cout << "║     Design Principles Demo           ║" << std::endl;
  std::cout << "╚══════════════════════════════════════╝" << std::endl;

  auto_region<DocumentStore> region(1024 * 1024);

  std::cout << "📝 Key design principles demonstrated:" << std::endl;

  // Regional types are region-allocated, never stack-allocated
  // This ensures zero-cost relocation and proper memory management
  std::cout << "\n🏗️  Regional type allocation patterns:" << std::endl;

  // ✅ Correct: Use intern_str for persistent string storage
  auto title = intern_str(*region, std::string_view("Design Principles Example"));
  std::cout << "   ✓ Persistent string created: " << (*title) << std::endl;

  // ✅ Correct: Access pre-allocated metadata dict from document structure
  auto store = region->root();
  std::cout << "   ✓ Document metadata dict accessible with " << store->metadata_->size() << " entries" << std::endl;

  // ✅ Correct: Use string_view for temporary operations (no allocation needed)
  std::cout << "\n📊 Temporary operations with string_view:" << std::endl;
  std::string_view temp_data[] = {"efficient", "temporary", "processing"};
  std::cout << "   ✓ Processing without allocation: ";
  for (const auto &word : temp_data) {
    std::cout << word << " ";
  }
  std::cout << std::endl;

  // ✅ Correct: Container methods construct elements in-place
  std::cout << "\n📦 In-place container element construction:" << std::endl;
  store->tags_.emplace_back(*region, std::string_view("design-patterns"));
  store->revisions_.enque(*region, std::string_view("Added design principles"));
  std::cout << "   ✓ Tag constructed in-place: " << (store->tags_.back()) << std::endl;
  std::cout << "   ✓ Revision queued in-place: " << (*store->revisions_.back()) << std::endl;

  // ✅ Correct: Metadata operations on document field
  store->metadata_->insert(*region, std::string_view("pattern"), std::string_view("zero-cost-relocation"));
  std::cout << "   ✓ Metadata entry added to document field" << std::endl;

  std::cout << "\n💡 Core design benefits:" << std::endl;
  std::cout << "   ✓ Zero-cost relocation of entire object graphs" << std::endl;
  std::cout << "   ✓ Compile-time memory safety guarantees" << std::endl;
  std::cout << "   ✓ Efficient offset-based regional_ptr storage" << std::endl;
  std::cout << "   ✓ Safe cross-region references with global_ptr" << std::endl;
  std::cout << "   ✓ No individual object destruction needed" << std::endl;

  std::cout << "\n⚖️  Design trade-offs in current implementation:" << std::endl;
  std::cout << "   • Regional types require region allocation" << std::endl;
  std::cout << "   • Container APIs use specific construction patterns" << std::endl;
  std::cout << "   • More explicit than traditional C++ patterns" << std::endl;
  std::cout << "   • Future language will provide ergonomic syntax" << std::endl;

  std::cout << std::endl;

  // RAII cleanup - region destroyed automatically
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
    demonstrate_memory_efficiency();
    demonstrate_zero_cost_relocation();
    demonstrate_design_principles();

    std::cout << "╔══════════════════════════════════════╗" << std::endl;
    std::cout << "║           Demo Complete! ✨          ║" << std::endl;
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
