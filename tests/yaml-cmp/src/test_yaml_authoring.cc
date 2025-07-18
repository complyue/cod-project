#include "shilos.hh"
#include "yaml_comparison.hh"

#include <iostream>
#include <string>
#include <variant>
#include <vector>

using namespace shilos;

// Test basic YamlDocument authoring with the new API
void test_basic_authoring() {
  std::cout << "Testing basic YamlDocument authoring..." << std::endl;

  // Author document without writing to disk
  auto result = yaml::YamlDocument::Write(
      "test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();
        author.set_map_value(root, "name", author.create_string("TestApplication"));
        author.set_map_value(root, "version", author.create_string("1.0.0"));
        author.set_map_value(root, "enabled", author.create_scalar(true));
        author.set_map_value(root, "port", author.create_scalar(8080));
        author.add_root(root);
      },
      false, false); // Don't write to disk

  // Verify successful creation and compare with expected data
  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
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

// Test YamlDocument constructor with authoring callback
void test_document_constructor() {
  std::cout << "Testing YamlDocument constructor with authoring..." << std::endl;

  try {
    // Create document directly with constructor (write=false)
    yaml::YamlDocument doc(
        "constructor_test.yaml",
        [](yaml::YamlAuthor &author) {
          auto root = author.create_map();
          author.set_map_value(root, "app", author.create_string("ConstructorTest"));
          author.set_map_value(root, "debug", author.create_scalar(false));
          author.add_root(root);
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
  auto result1 = yaml::YamlDocument::Write(
      "/tmp/output_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();
        author.set_map_value(root, "test", author.create_string("write_functionality"));
        author.set_map_value(root, "timestamp", author.create_scalar(1234567890));
        author.add_root(root);
      },
      true, true); // write=true, overwrite=true

  if (!std::holds_alternative<yaml::YamlDocument>(result1)) {
    std::cerr << "❌ Write test failed - expected YamlDocument but got AuthorError" << std::endl;
    throw std::runtime_error("Write test failed");
  }

  // Test write without overwrite to existing file (should fail)
  auto result2 = yaml::YamlDocument::Write(
      "/tmp/output_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();
        author.set_map_value(root, "test", author.create_string("second_attempt"));
        author.add_root(root);
      },
      true, false); // write=true, overwrite=false

  if (!std::holds_alternative<yaml::AuthorError>(result2)) {
    std::cerr << "❌ Write overwrite test failed - expected AuthorError but got YamlDocument" << std::endl;
    throw std::runtime_error("Write overwrite test failed");
  }

  std::cout << "✓ Write write/overwrite test passed" << std::endl;
}

// Test nested structures with the new API
void test_nested_structures() {
  std::cout << "Testing nested structures..." << std::endl;

  auto result = yaml::YamlDocument::Write(
      "nested_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();

        // Create nested configuration
        auto config = author.create_map();
        author.set_map_value(config, "host", author.create_string("localhost"));
        author.set_map_value(config, "port", author.create_scalar(5432));
        author.set_map_value(config, "ssl", author.create_scalar(true));

        auto pool = author.create_map();
        author.set_map_value(pool, "min_connections", author.create_scalar(5));
        author.set_map_value(pool, "max_connections", author.create_scalar(20));
        author.set_map_value(config, "pool", pool);

        author.set_map_value(root, "database", config);

        // Create sequence of features
        auto features = author.create_sequence();
        author.push_to_sequence(features, author.create_string("authentication"));
        author.push_to_sequence(features, author.create_string("logging"));
        author.push_to_sequence(features, author.create_string("monitoring"));
        author.set_map_value(root, "features", features);

        // Create sequence of service configurations
        auto services = author.create_sequence();
        std::vector<std::string> service_names = {"api", "worker", "scheduler"};
        std::vector<int> service_ports = {8001, 8002, 8003};

        for (size_t i = 0; i < service_names.size(); ++i) {
          auto service = author.create_map();
          author.set_map_value(service, "name", author.create_string(service_names[i]));
          author.set_map_value(service, "port", author.create_scalar(service_ports[i]));
          author.set_map_value(service, "enabled", author.create_scalar(true));
          author.push_to_sequence(services, service);
        }
        author.set_map_value(root, "services", services);

        author.add_root(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
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

  auto result = yaml::YamlDocument::Write(
      "lifetime_test.yaml",
      [&](yaml::YamlAuthor &author) {
        auto root = author.create_map();

        // Test various string creation methods
        author.set_map_value(root, "literal", author.create_string("literal_string"));
        author.set_map_value(root, "external", author.create_string(external_string));
        author.set_map_value(root, "temp", author.create_string(std::move(temp_string)));
        author.set_map_value(root, "view", author.create_string(std::string_view("view_string")));
        author.set_map_value(root, "cstr", author.create_string("cstring_literal"));

        author.add_root(root);
      },
      false, false); // Don't write to disk

  // Modify external strings after document creation
  external_string = "modified_external";
  temp_string = "modified_temp";

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
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

  auto result = yaml::YamlDocument::Write(
      "scalars_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();

        // Test basic scalar types (simplified for comparison)
        author.set_map_value(root, "string", author.create_string("hello"));
        author.set_map_value(root, "int", author.create_scalar(42));
        author.set_map_value(root, "double", author.create_scalar(3.14));
        author.set_map_value(root, "bool", author.create_scalar(true));

        author.add_root(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
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
  auto result = yaml::YamlDocument::Write(
      "error_test.yaml", [](yaml::YamlAuthor &author) { throw std::runtime_error("Intentional test error"); });

  // Should return AuthorError, not throw
  if (!std::holds_alternative<yaml::AuthorError>(result)) {
    std::cerr << "❌ Error handling test failed - expected AuthorError but got YamlDocument" << std::endl;
    throw std::runtime_error("Error handling test failed");
  }

  auto error = std::get<yaml::AuthorError>(result);
  if (error.message().find("Intentional test error") == std::string::npos) {
    std::cerr << "❌ Error handling test failed - error message doesn't contain expected text" << std::endl;
    throw std::runtime_error("Error handling test failed");
  }

  // Test AuthorError from constructor
  try {
    yaml::YamlDocument bad_doc(
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

  auto result = yaml::YamlDocument::Write(
      "empty_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();

        author.set_map_value(root, "empty_map", author.create_map());
        author.set_map_value(root, "empty_sequence", author.create_sequence());

        // Test that we can still add to containers after creation
        auto populated_map = author.create_map();
        author.set_map_value(populated_map, "key", author.create_string("value"));
        author.set_map_value(root, "populated_map", populated_map);

        auto populated_seq = author.create_sequence();
        author.push_to_sequence(populated_seq, author.create_string("item"));
        author.set_map_value(root, "populated_sequence", populated_seq);

        author.add_root(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
    auto root = doc.root();
    auto map = root.as_map();

    // Test empty containers
    if (map["empty_map"].is_map() && map["empty_map"].as_map().empty() && map["empty_sequence"].is_sequence() &&
        map["empty_sequence"].as_sequence().empty() && map["populated_map"].is_map() &&
        map["populated_map"].as_map().size() == 1 && map["populated_map"].as_map()["key"].as_string() == "value" &&
        map["populated_sequence"].is_sequence() && map["populated_sequence"].as_sequence().size() == 1 &&
        map["populated_sequence"].as_sequence()[0].as_string() == "item") {
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
  auto authored_result = yaml::YamlDocument::Write(
      "comparison_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();
        author.set_map_value(root, "name", author.create_string("TestApp"));
        author.set_map_value(root, "version", author.create_string("2.0.0"));

        auto config = author.create_map();
        author.set_map_value(config, "debug", author.create_scalar(true));
        author.set_map_value(config, "port", author.create_scalar(9000));
        author.set_map_value(config, "timeout", author.create_scalar(30.5));
        author.set_map_value(root, "config", config);

        auto tags = author.create_sequence();
        author.push_to_sequence(tags, author.create_string("production"));
        author.push_to_sequence(tags, author.create_string("stable"));
        author.set_map_value(root, "tags", tags);

        author.add_root(root);
      },
      false, false); // Don't write to disk

  if (!std::holds_alternative<yaml::YamlDocument>(authored_result)) {
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
  auto parsed_result = yaml::YamlDocument::Parse("comparison_parsed.yaml", yaml_content);
  if (!std::holds_alternative<yaml::YamlDocument>(parsed_result)) {
    std::cerr << "❌ Authoring vs parsing test failed - Parse error occurred" << std::endl;
    throw std::runtime_error("Authoring vs parsing test failed");
  }

  // Compare the two documents
  auto authored_doc = std::get<yaml::YamlDocument>(std::move(authored_result));
  auto parsed_doc = std::get<yaml::YamlDocument>(std::move(parsed_result));

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

  auto result = yaml::YamlDocument::Write(
      "complex_test.yaml",
      [](yaml::YamlAuthor &author) {
        auto root = author.create_map();

        // Simplified complex document for comparison
        auto metadata = author.create_map();
        author.set_map_value(metadata, "version", author.create_string("1.0"));
        author.set_map_value(metadata, "author", author.create_string("Test Author"));
        author.set_map_value(root, "metadata", metadata);

        auto data = author.create_sequence();
        auto item1 = author.create_map();
        author.set_map_value(item1, "name", author.create_string("item1"));
        author.set_map_value(item1, "value", author.create_scalar(100));
        author.push_to_sequence(data, item1);

        auto item2 = author.create_map();
        author.set_map_value(item2, "name", author.create_string("item2"));
        author.set_map_value(item2, "value", author.create_scalar(200));
        author.push_to_sequence(data, item2);

        author.set_map_value(root, "data", data);

        auto config = author.create_map();
        author.set_map_value(config, "debug", author.create_scalar(true));
        author.set_map_value(config, "timeout", author.create_scalar(30));
        author.set_map_value(root, "config", config);

        author.add_root(root);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));
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

  auto result = yaml::YamlDocument::Write(
      "multi_root_test.yaml",
      [](yaml::YamlAuthor &author) {
        // First document
        auto doc1 = author.create_map();
        author.set_map_value(doc1, "count", author.create_scalar(1));
        author.add_root(doc1);

        // Second document
        auto doc2 = author.create_map();
        author.set_map_value(doc2, "count", author.create_scalar(2));
        author.add_root(doc2);

        // Third document
        auto doc3 = author.create_map();
        author.set_map_value(doc3, "count", author.create_scalar(3));
        author.add_root(doc3);
      },
      false, false); // Don't write to disk

  if (std::holds_alternative<yaml::YamlDocument>(result)) {
    auto doc = std::get<yaml::YamlDocument>(std::move(result));

    // Verify document count
    if (doc.document_count() != 3) {
      std::cerr << "❌ Multi-root test failed - expected 3 documents, got " << doc.document_count() << std::endl;
      throw std::runtime_error("Multi-root test failed");
    }

    // Verify each document structure
    for (size_t i = 0; i < 3; ++i) {
      const auto &root = doc.root(i);
      if (!root.IsMap()) {
        std::cerr << "❌ Multi-root test failed - document " << i << " is not a map" << std::endl;
        throw std::runtime_error("Multi-root test failed");
      }

      const auto &map = root.as_map();
      if (map.size() != 1 || map.find("count") == map.end()) {
        std::cerr << "❌ Multi-root test failed - document " << i << " doesn't have count key" << std::endl;
        throw std::runtime_error("Multi-root test failed");
      }

      const auto &count_node = map.find("count")->value;
      if (!count_node.IsScalar() || count_node.as<int>() != static_cast<int>(i + 1)) {
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
  std::cout << "=== YamlDocument Authoring API Test Suite ===" << std::endl;
  std::cout << "Testing YamlDocument authoring API with yaml-cmp comparison..." << std::endl;

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

    std::cout << "\n🎉 All YamlDocument authoring API tests passed!" << std::endl;
    std::cout << "The authoring API with yaml-cmp comparison is working correctly." << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
