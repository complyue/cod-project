#include "yaml_comparison.hh"

#include <iostream>
#include <string>
#include <variant>
#include <vector>

using namespace shilos;

// Test basic Document authoring with the new API
void test_basic_authoring() {
  std::cout << "Testing basic Document authoring..." << std::endl;

  // Author document without writing to disk
  auto result = yaml::Document::Write(
      "test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();
        author.setMapValue(root, "name", author.createString("TestApplication"));
        author.setMapValue(root, "version", author.createString("1.0.0"));
        author.setMapValue(root, "enabled", author.createScalar(true));
        author.setMapValue(root, "port", author.createScalar(8080));
        author.addRoot(root);
      },
      false, false); // Don't write to disk

  // Verify successful creation and compare with expected data
  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/basic_authoring_expected.yaml")) {
      std::cout << "✓ Basic authoring test passed" << std::endl;
    } else {
      std::cerr << "❌ Basic authoring test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("Basic authoring test failed");
    }
  } else {
    std::cerr << "❌ Basic authoring test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Basic authoring test failed");
  }
}

// Test Document constructor with authoring callback
void test_document_constructor() {
  std::cout << "Testing Document constructor with authoring..." << std::endl;

  try {
    // Create document directly with constructor (write=false)
    yaml::Document doc(
        "constructor_test.yaml",
        [](yaml::YamlAuthor &author) {
          auto root = author.createMap();
          author.setMapValue(root, "app", author.createString("ConstructorTest"));
          author.setMapValue(root, "debug", author.createScalar(false));
          author.addRoot(root);
        },
        false, false); // write=false, overwrite=false

    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/document_constructor_expected.yaml")) {
      std::cout << "✓ Document constructor test passed" << std::endl;
    } else {
      std::cerr << "❌ Document constructor test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("Document constructor test failed");
    }
  } catch (const yaml::AuthorError &e) {
    std::cerr << "❌ AuthorError: " << e.what() << std::endl;
    throw;
  }
}

// Test Write() method with write and overwrite parameters
void test_write_method() {
  std::cout << "Testing Write() method with write/overwrite..." << std::endl;

  // Test successful write with overwrite=true
  auto result1 = yaml::Document::Write(
      "/tmp/output_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();
        author.setMapValue(root, "test", author.createString("write_functionality"));
        author.setMapValue(root, "timestamp", author.createScalar(1234567890));
        author.addRoot(root);
      },
      true, true); // write=true, overwrite=true

  if (!std::holds_alternative<yaml::Document>(result1)) {
    std::cerr << "❌ Write test failed - expected Document but got AuthorError" << std::endl;
    throw std::runtime_error("Write test failed");
  }

  // Test write without overwrite to existing file (should fail)
  auto result2 = yaml::Document::Write(
      "/tmp/output_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();
        author.setMapValue(root, "test", author.createString("second_attempt"));
        author.addRoot(root);
      },
      true, false); // write=true, overwrite=false

  if (!std::holds_alternative<yaml::AuthorError>(result2)) {
    std::cerr << "❌ Write overwrite test failed - expected AuthorError but got Document" << std::endl;
    throw std::runtime_error("Write overwrite test failed");
  }

  std::cout << "✓ Write write/overwrite test passed" << std::endl;
}

// Test nested structures with the new API
void test_nested_structures() {
  std::cout << "Testing nested structures..." << std::endl;

  auto result = yaml::Document::Write(
      "nested_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();

        // Create nested configuration
        auto config = author.createMap();
        author.setMapValue(config, "host", author.createString("localhost"));
        author.setMapValue(config, "port", author.createScalar(5432));
        author.setMapValue(config, "ssl", author.createScalar(true));

        auto pool = author.createMap();
        author.setMapValue(pool, "min_connections", author.createScalar(5));
        author.setMapValue(pool, "max_connections", author.createScalar(20));
        author.setMapValue(config, "pool", pool);

        author.setMapValue(root, "database", config);

        // Create sequence of features
        auto features = author.createSequence();
        author.pushToSequence(features, author.createString("authentication"));
        author.pushToSequence(features, author.createString("logging"));
        author.pushToSequence(features, author.createString("monitoring"));
        author.setMapValue(root, "features", features);

        // Create sequence of service configurations
        auto services = author.createSequence();
        std::vector<std::string> service_names = {"api", "worker", "scheduler"};
        std::vector<int> service_ports = {8001, 8002, 8003};

        for (size_t i = 0; i < service_names.size(); ++i) {
          auto service = author.createMap();
          author.setMapValue(service, "name", author.createString(service_names[i]));
          author.setMapValue(service, "port", author.createScalar(service_ports[i]));
          author.setMapValue(service, "enabled", author.createScalar(true));
          author.pushToSequence(services, service);
        }
        author.setMapValue(root, "services", services);

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/nested_structures_expected.yaml")) {
      std::cout << "✓ Nested structures test passed" << std::endl;
    } else {
      std::cerr << "❌ Nested structures test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("Nested structures test failed");
    }
  } else {
    std::cerr << "❌ Nested structures test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Nested structures test failed");
  }
}

// Test string lifetime management with the new API
void test_string_lifetime() {
  std::cout << "Testing string lifetime management..." << std::endl;

  std::string external_string = "external_value";
  std::string temp_string = "temporary_value";

  auto result = yaml::Document::Write(
      "lifetime_test.yaml",
      [&](yaml::YamlAuthor &author) {
        auto root = author.createMap();

        // Test various string creation methods
        author.setMapValue(root, "literal", author.createString("literal_string"));
        author.setMapValue(root, "external", author.createString(external_string));
        author.setMapValue(root, "temp", author.createString(std::move(temp_string)));
        author.setMapValue(root, "view", author.createString(std::string_view("view_string")));
        author.setMapValue(root, "cstr", author.createString("cstring_literal"));

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  // Modify external strings after document creation
  external_string = "modified_external";
  temp_string = "modified_temp";

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/string_lifetime_expected.yaml")) {
      std::cout << "✓ String lifetime test passed" << std::endl;
    } else {
      std::cerr << "❌ String lifetime test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("String lifetime test failed");
    }
  } else {
    std::cerr << "❌ String lifetime test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("String lifetime test failed");
  }
}

// Test scalar type creation with the new API
void test_scalar_types() {
  std::cout << "Testing scalar type creation..." << std::endl;

  auto result = yaml::Document::Write(
      "scalars_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();

        // Test basic scalar types (simplified for comparison)
        author.setMapValue(root, "string", author.createString("hello"));
        author.setMapValue(root, "int", author.createScalar(42));
        author.setMapValue(root, "double", author.createScalar(3.14));
        author.setMapValue(root, "bool", author.createScalar(true));

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/scalar_types_expected.yaml")) {
      std::cout << "✓ Scalar types test passed" << std::endl;
    } else {
      std::cerr << "❌ Scalar types test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("Scalar types test failed");
    }
  } else {
    std::cerr << "❌ Scalar types test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Scalar types test failed");
  }
}

// Test error handling with the new API
void test_error_handling() {
  std::cout << "Testing error handling..." << std::endl;

  // Test callback that throws exception
  auto result = yaml::Document::Write(
      "error_test.yaml", [](yaml::YamlAuthor &author) { throw std::runtime_error("Intentional test error"); });

  // Should return AuthorError, not throw
  if (!std::holds_alternative<yaml::AuthorError>(result)) {
    std::cerr << "❌ Error handling test failed - expected AuthorError but got Document" << std::endl;
    throw std::runtime_error("Error handling test failed");
  }

  auto error = std::get<yaml::AuthorError>(result);
  if (error.message().find("Intentional test error") == std::string::npos) {
    std::cerr << "❌ Error handling test failed - error message doesn't contain expected text" << std::endl;
    throw std::runtime_error("Error handling test failed");
  }

  // Test AuthorError from constructor
  try {
    yaml::Document bad_doc(
        "bad_test.yaml", [](yaml::YamlAuthor &author) { throw std::runtime_error("Constructor error"); }, true, true);
    std::cerr << "❌ Error handling test failed - constructor should have thrown AuthorError" << std::endl;
    throw std::runtime_error("Error handling test failed");
  } catch (const yaml::AuthorError &e) {
    if (std::string(e.what()).find("Constructor error") == std::string::npos) {
      std::cerr << "❌ Error handling test failed - constructor error message doesn't contain expected text"
                << std::endl;
      throw std::runtime_error("Error handling test failed");
    }
  }

  std::cout << "✓ Error handling test passed" << std::endl;
}

// Test empty containers with the new API
void test_empty_containers() {
  std::cout << "Testing empty containers..." << std::endl;

  auto result = yaml::Document::Write(
      "empty_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();

        author.setMapValue(root, "empty_map", author.createMap());
        author.setMapValue(root, "empty_sequence", author.createSequence());

        // Test that we can still add to containers after creation
        auto populated_map = author.createMap();
        author.setMapValue(populated_map, "key", author.createString("value"));
        author.setMapValue(root, "populated_map", populated_map);

        auto populated_seq = author.createSequence();
        author.pushToSequence(populated_seq, author.createString("item"));
        author.setMapValue(root, "populated_sequence", populated_seq);

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    auto root = doc.root();
    auto map = root.asMap();

    // Test empty containers
    if (map["empty_map"].IsMap() && map["empty_map"].asMap().empty() && map["empty_sequence"].IsSequence() &&
        map["empty_sequence"].asSequence().empty() && map["populated_map"].IsMap() &&
        map["populated_map"].asMap().size() == 1 && map["populated_map"].asMap().at("key").asString() == "value" &&
        map["populated_sequence"].IsSequence() && map["populated_sequence"].asSequence().size() == 1 &&
        map["populated_sequence"].asSequence()[0].asString() == "item") {
      std::cout << "✓ Empty containers test passed" << std::endl;
    } else {
      std::cerr << "❌ Empty containers test failed - structure validation failed" << std::endl;
      throw std::runtime_error("Empty containers test failed");
    }
  } else {
    std::cerr << "❌ Empty containers test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Empty containers test failed");
  }
}

// Test comparison between authoring and parsing APIs
void test_authoring_vs_parsing() {
  std::cout << "Testing authoring vs parsing consistency..." << std::endl;

  // Create document using authoring API
  auto authored_result = yaml::Document::Write(
      "comparison_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();
        author.setMapValue(root, "name", author.createString("TestApp"));
        author.setMapValue(root, "version", author.createString("2.0.0"));

        auto config = author.createMap();
        author.setMapValue(config, "debug", author.createScalar(true));
        author.setMapValue(config, "port", author.createScalar(9000));
        author.setMapValue(config, "timeout", author.createScalar(30.5));
        author.setMapValue(root, "config", config);

        auto tags = author.createSequence();
        author.pushToSequence(tags, author.createString("production"));
        author.pushToSequence(tags, author.createString("stable"));
        author.setMapValue(root, "tags", tags);

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  if (!std::holds_alternative<yaml::Document>(authored_result)) {
    std::cerr << "❌ Authoring vs parsing test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Authoring vs parsing test failed");
  }

  // Create equivalent YAML string
  std::string yaml_content = R"(name: TestApp
version: "2.0.0"
config:
  debug: true
  port: 9000
  timeout: 30.5
tags:
  - production
  - stable
)";

  // Parse the equivalent content
  auto parsed_result = yaml::Document::Parse("comparison_parsed.yaml", yaml_content);
  if (!std::holds_alternative<yaml::Document>(parsed_result)) {
    std::cerr << "❌ Authoring vs parsing test failed - Parse error occurred" << std::endl;
    throw std::runtime_error("Authoring vs parsing test failed");
  }

  // Compare the two documents
  auto authored_doc = std::get<yaml::Document>(std::move(authored_result));
  auto parsed_doc = std::get<yaml::Document>(std::move(parsed_result));

  if (yaml_cmp::yaml_equal(authored_doc.root(), parsed_doc.root())) {
    std::cout << "✓ Authoring vs parsing consistency test passed" << std::endl;
  } else {
    std::cerr << "❌ Authoring vs parsing test failed - documents not equal" << std::endl;
    throw std::runtime_error("Authoring vs parsing test failed");
  }
}

// Test complex document creation showcasing the new API
void test_complex_document() {
  std::cout << "Testing complex document creation..." << std::endl;

  auto result = yaml::Document::Write(
      "complex_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.createMap();

        // Simplified complex document for comparison
        auto metadata = author.createMap();
        author.setMapValue(metadata, "version", author.createString("1.0"));
        author.setMapValue(metadata, "author", author.createString("Test Author"));
        author.setMapValue(root, "metadata", metadata);

        auto data = author.createSequence();
        auto item1 = author.createMap();
        author.setMapValue(item1, "name", author.createString("item1"));
        author.setMapValue(item1, "value", author.createScalar(100));
        author.pushToSequence(data, item1);

        auto item2 = author.createMap();
        author.setMapValue(item2, "name", author.createString("item2"));
        author.setMapValue(item2, "value", author.createScalar(200));
        author.pushToSequence(data, item2);

        author.setMapValue(root, "data", data);

        auto config = author.createMap();
        author.setMapValue(config, "debug", author.createScalar(true));
        author.setMapValue(config, "timeout", author.createScalar(30));
        author.setMapValue(root, "config", config);

        author.addRoot(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));
    if (yaml_cmp::compare_authored_with_expected(doc, "test-data/complex_document_expected.yaml")) {
      std::cout << "✓ Complex document test passed" << std::endl;
    } else {
      std::cerr << "❌ Complex document test failed - comparison mismatch" << std::endl;
      throw std::runtime_error("Complex document test failed");
    }
  } else {
    std::cerr << "❌ Complex document test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Complex document test failed");
  }
}
// Test multi-root functionality
void test_multi_root() {
  std::cout << "Testing multi-root functionality..." << std::endl;

  auto result = yaml::Document::Write(
      "multi_root_test.yaml",
      [](yaml::YamlAuthor &author) {
        // First document
        auto doc1 = author.createMap();
        author.setMapValue(doc1, "count", author.createScalar(1));
        author.addRoot(doc1);

        // Second document
        auto doc2 = author.createMap();
        author.setMapValue(doc2, "count", author.createScalar(2));
        author.addRoot(doc2);

        // Third document
        auto doc3 = author.createMap();
        author.setMapValue(doc3, "count", author.createScalar(3));
        author.addRoot(doc3);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::Document>(result)) {
    auto doc = std::get<yaml::Document>(std::move(result));

    // Verify document count
    if (doc.documentCount() != 3) {
      std::cerr << "❌ Multi-root test failed - expected 3 documents, got " << doc.documentCount() << std::endl;
      throw std::runtime_error("Multi-root test failed");
    }

    // Verify each document structure
    for (size_t i = 0; i < 3; ++i) {
      const auto &root = doc.root(i);
      if (!root.IsMap()) {
        std::cerr << "❌ Multi-root test failed - document " << i << " is not a map" << std::endl;
        throw std::runtime_error("Multi-root test failed");
      }

      const auto &map = root.asMap();
      if (map.size() != 1 || map.find("count") == map.end()) {
        std::cerr << "❌ Multi-root test failed - document " << i << " doesn't have count key" << std::endl;
        throw std::runtime_error("Multi-root test failed");
      }

      const auto &count_node = map.find("count")->value;
      if (!count_node.IsScalar() || count_node.asInt() != static_cast<int>(i + 1)) {
        std::cerr << "❌ Multi-root test failed - document " << i << " has wrong count value" << std::endl;
        throw std::runtime_error("Multi-root test failed");
      }
    }

    std::cout << "✓ Multi-root test passed" << std::endl;
  } else {
    std::cerr << "❌ Multi-root test failed - AuthorError occurred" << std::endl;
    throw std::runtime_error("Multi-root test failed");
  }
}

int main() {
  std::cout << "=== Document Authoring API Test Suite ===" << std::endl;
  std::cout << "Testing Document authoring API with yaml-cmp comparison..." << std::endl;

  try {
    test_basic_authoring();
    test_document_constructor();
    test_write_method();
    test_nested_structures();
    test_string_lifetime();
    test_scalar_types();
    test_error_handling();
    test_empty_containers();
    test_authoring_vs_parsing();
    test_complex_document();
    test_multi_root();

    std::cout << "\n🎉 All Document authoring API tests passed!" << std::endl;
    std::cout << "The authoring API with yaml-cmp comparison is working correctly." << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
