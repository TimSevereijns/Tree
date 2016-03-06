#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

#include "../Tree/Tree.hpp"

#include <algorithm>

SCENARIO("Adding nodes to the tree", "[treeConstruction]")
{
   GIVEN("a tree with only a head node")
   {
      Tree<std::string> tree{ "Head" };

      REQUIRE(tree.GetHead() != nullptr);
      REQUIRE(tree.GetHead()->GetData() == "Head");

      WHEN("a single child node is added to the head node")
      {
         std::string firstChildLabel{ "First Child" };
         tree.GetHead()->AppendChild(firstChildLabel);

         THEN("that child is reachable from the head node")
         {
            REQUIRE(tree.GetHead()->GetChildCount() == 1);
            REQUIRE(tree.GetHead()->GetFirstChild() != nullptr);
            REQUIRE(tree.GetHead()->GetLastChild() != nullptr);
            REQUIRE(tree.GetHead()->GetFirstChild() == tree.GetHead()->GetLastChild());
            REQUIRE(tree.GetHead()->GetFirstChild()->GetData() == firstChildLabel);
         }

         WHEN("a sibling is added to the head node's only child")
         {
            std::string secondChildLabel{ "Second Child" };
            tree.GetHead()->AppendChild(secondChildLabel);

            THEN("both the first child and second child are properly reachable")
            {
               REQUIRE(tree.GetHead()->GetChildCount() == 2);
               REQUIRE(tree.GetHead()->GetFirstChild() != nullptr);
               REQUIRE(tree.GetHead()->GetLastChild() != nullptr);
               REQUIRE(tree.GetHead()->GetFirstChild() != tree.GetHead()->GetLastChild());
               REQUIRE(tree.GetHead()->GetFirstChild()->GetData() == firstChildLabel);
               REQUIRE(tree.GetHead()->GetLastChild()->GetData() == secondChildLabel);
               REQUIRE(tree.GetHead()->GetFirstChild()->GetNextSibling() == tree.GetHead()->GetLastChild());
            }
         }
      }
   }
}

TEST_CASE("TreeNode Construction")
{
   TreeNode<std::string> node{ "Bar" };

   SECTION("Default Construction")
   {
      TreeNode<std::string> default{};
      const std::string emptyString;

      REQUIRE(default.GetChildCount() == 0);
      REQUIRE(default.GetFirstChild() == nullptr);
      REQUIRE(default.GetLastChild() == nullptr);
      REQUIRE(default.GetParent() == nullptr);
      REQUIRE(default.GetNextSibling() == nullptr);
      REQUIRE(default.GetPreviousSibling() == nullptr);
      REQUIRE(default.GetData() == emptyString);
   }
   SECTION("Copy Construction")
   {
      TreeNode<std::string> copy{ node };

      REQUIRE(copy.GetData() == node.GetData());
      REQUIRE(&copy.GetData() != &node.GetData());
   }
}

TEST_CASE("TreeNode Alterations")
{
   TreeNode<std::string> node{ "Bar" };
   REQUIRE(node.GetData() == "Bar");

   SECTION("Altering Data")
   {
      auto& data = node.GetData();
      std::transform(std::begin(data), std::end(data), std::begin(data), ::toupper);

      REQUIRE(node.GetData() == "BAR");
   }
}

TEST_CASE("Forward Pre- and Post-Order Traversal of Simple Binary Tree")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Node Count")
   {
      REQUIRE(tree.Size() == 9);
      REQUIRE(Tree<std::string>::Depth(*tree.GetHead()) == 0);
   }
   SECTION("Pre-order Traversal")
   {
      const std::vector<std::string> expectedTraversal =
         { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = tree.beginPreOrder(); itr != tree.endPreOrder(); ++itr)
      {
         if (itr->GetData() != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("Post-order Traversal")
   {
      const std::vector<std::string> expectedTraversal =
         { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = tree.begin(); itr != tree.end(); ++itr)
      {
         if (itr->GetData() != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("STL Compliance")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Standard Algorithms and Parameter Passing by value_type")
   {
      const size_t count = std::count_if(std::begin(tree), std::end(tree), 
         [](Tree<std::string>::value_type node)
      {
         return (node.GetData() == "A");
      });

      REQUIRE(count == 1);
   }
   //SECTION("Standard Algorithms and Parameter Passing by pointer")
   //{
   //   const size_t count = std::count_if(std::begin(tree), std::end(tree),
   //      [](Tree<std::string>::pointer node)
   //   {
   //      return (node->GetData() == "B");
   //   });

   //   REQUIRE(count == 1);
   //}
   SECTION("Standard Algorithms and Parameter Passing by reference")
   {
      const size_t count = std::count_if(std::begin(tree), std::end(tree),
         [](Tree<std::string>::reference node)
      {
         return (node.GetData() == "C");
      });

      REQUIRE(count == 1);
   }
   SECTION("Standard Algorithms and Parameter Passing by const_reference")
   {
      const size_t count = std::count_if(std::begin(tree), std::end(tree),
         [](Tree<std::string>::const_reference node)
      {
         return (node.GetData() == "D");
      });

      REQUIRE(count == 1);
   }
}