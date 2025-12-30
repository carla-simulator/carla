// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include <pugixml/pugixml.hpp>

using namespace pugi;

TEST(pugixml, null_pointer_dereference_insert_move_before) {
  // Create an empty node (without root, _root = nullptr)
  xml_node empty_node;
  ASSERT_FALSE(empty_node);

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child1 = root.append_child("child1");
  xml_node child2 = root.append_child("child2");

  // Attempt insert_move_before from an empty node
  // Before the patch this would crash, now it should return an empty xml_node
  xml_node result = empty_node.insert_move_before(child2, child1);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}

TEST(pugixml, null_pointer_dereference_insert_move_after) {
  // Create an empty node
  xml_node empty_node;

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child1 = root.append_child("child1");
  xml_node child2 = root.append_child("child2");

  // Attempt insert_move_after from an empty node
  xml_node result = empty_node.insert_move_after(child2, child1);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}

TEST(pugixml, null_pointer_dereference_insert_child_before) {
  // Create an empty node
  xml_node empty_node;

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child = root.append_child("child");

  // Attempt insert_child_before from an empty node
  xml_node result = empty_node.insert_child_before(node_element, child);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}

TEST(pugixml, null_pointer_dereference_insert_child_after) {
  // Create an empty node
  xml_node empty_node;

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child = root.append_child("child");

  // Attempt insert_child_after from an empty node
  xml_node result = empty_node.insert_child_after(node_element, child);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}

TEST(pugixml, null_pointer_dereference_insert_copy_before) {
  // Create an empty node
  xml_node empty_node;

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child = root.append_child("child");
  xml_node proto = root.append_child("proto");

  // Attempt insert_copy_before from an empty node
  xml_node result = empty_node.insert_copy_before(proto, child);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}

TEST(pugixml, null_pointer_dereference_insert_copy_after) {
  // Create an empty node
  xml_node empty_node;

  // Create a valid document
  xml_document doc;
  xml_node root = doc.append_child("root");
  xml_node child = root.append_child("child");
  xml_node proto = root.append_child("proto");

  // Attempt insert_copy_after from an empty node
  xml_node result = empty_node.insert_copy_after(proto, child);

  // Verify it returns an empty node without crashing
  EXPECT_FALSE(result);
}
