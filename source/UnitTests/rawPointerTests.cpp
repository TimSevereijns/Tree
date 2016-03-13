#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

//#include "../Tree/Tree.hpp"
#include "../Tree/RawPointerTree.hpp"

#include <algorithm>

SCENARIO("Adding nodes to the tree", "[treeConstruction]")
{
   GIVEN("a tree with only a head node")
   {
      Tree<std::string> tree{ "Head" };

      REQUIRE(tree.GetHead() != nullptr);
      REQUIRE(tree.GetHead()->GetData() == "Head");
      REQUIRE(tree.GetHead()->CountAllDescendants() == 0);

      WHEN("a single child node is added to the head node")
      {
         const std::string firstChildLabel{ "First Child" };
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
            const std::string secondChildLabel{ "Second Child" };
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

TEST_CASE("TreeNode Construction and Assignment")
{
   const TreeNode<std::string> node{ "Bar" };

   SECTION("Default Construction")
   {
      const TreeNode<std::string> default{};
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
      const TreeNode<std::string> copy{ node };

      REQUIRE(copy.GetData() == node.GetData());
      REQUIRE(&copy.GetData() != &node.GetData());
   }
   SECTION("Assignment Operator")
   {
      const TreeNode<std::string> copy = node;

      REQUIRE(copy.GetData() == node.GetData());
   }
}

TEST_CASE("TreeNode Comparison Operations")
{
   const TreeNode<int> ten{ 10 };
   const TreeNode<int> twenty{ 20 };

   SECTION("Less Than")
   {
      REQUIRE(ten < twenty);
      REQUIRE(!(twenty < ten));
   }
   SECTION("Less Than or Equal")
   {
      REQUIRE(ten <= ten);
      REQUIRE(ten <= twenty);
      REQUIRE(!(twenty <= ten));
   }
   SECTION("Equality")
   {
      REQUIRE(ten == ten);
      REQUIRE(!(ten == twenty));
   }
   SECTION("Greater Than")
   {
      REQUIRE(twenty > ten);
      REQUIRE(!(ten > twenty));
   }
   SECTION("Greater Than or Equal")
   {
      REQUIRE(twenty >= twenty);
      REQUIRE(twenty >= ten);
      REQUIRE(!(ten >= twenty));
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

TEST_CASE("Node Counting")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Tree Size")
   {
      REQUIRE(tree.Size() == 9);
   }
   SECTION("Node Depth")
   {
      REQUIRE(Tree<std::string>::Depth(*tree.GetHead()) == 0);
   }
   SECTION("Descendent Count")
   {
      REQUIRE(tree.GetHead()->GetFirstChild()->CountAllDescendants() == 4);
   }
}

TEST_CASE("TreeNode::Iterators")
{
   const TreeNode<std::string> node{ "Test" };
   const auto sharedNode = std::make_shared<TreeNode<std::string>>(node);
   const auto constItr = Tree<std::string>::PostOrderIterator(sharedNode.get());

   auto nonConstItr = Tree<std::string>::PostOrderIterator(sharedNode.get());

   SECTION("operator bool")
   {
      auto endItr = Tree<std::string>::PostOrderIterator();

      REQUIRE(constItr);
      REQUIRE(!endItr);
   }
   SECTION("operator*")
   {
      REQUIRE((*nonConstItr).GetData() == "Test");
   }
   SECTION("operator* const")
   {
      REQUIRE((*constItr).GetData() == "Test");
   }
   SECTION("operator&")
   {
      REQUIRE(&nonConstItr == sharedNode.get());
   }
   SECTION("operator& const")
   {
      REQUIRE(&constItr == sharedNode.get());
   }
   SECTION("operator->")
   {
      REQUIRE(nonConstItr->GetData() == "Test");
   }
   SECTION("operator-> const")
   {
      REQUIRE(constItr->GetData() == "Test");
   }
   SECTION("operator==")
   {
      const auto duplicate = Tree<std::string>::PostOrderIterator(sharedNode.get());

      REQUIRE(duplicate == constItr);
   }
}

TEST_CASE("Forward Pre- and Post-Order Traversal of Simple Binary Tree")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-order Traversal")
   {
      const std::vector<std::string> expectedTraversal =
      { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = tree.beginPreOrder(); itr != tree.endPreOrder(); ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
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
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("Partial Tree Iteration")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal =
      { "B", "A", "D", "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto* startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::PreOrderIterator(startingNode);
      const auto end = Tree<std::string>::PreOrderIterator();
      for (; itr != end; itr++) ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal =
      { "A", "C", "E", "D", "B" };

      int index = 0;

      bool traversalError = false;

      auto* startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::PostOrderIterator(startingNode);
      const auto end = Tree<std::string>::PostOrderIterator();
      for (; itr != end; itr++)  ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("STL Typedef Compliance")
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

TEST_CASE("Leaf Iterator")
{
   Tree<std::string> tree{ "F" };
   tree.GetHead()->AppendChild("B")->AppendChild("A");
   tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expectedTraversal =
      { "A", "C", "E", "H", };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(tree.GetHead());
      const auto end = Tree<std::string>::LeafIterator();
      for (; itr != end; ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("Partial Tree Iteration")
   {
      const std::vector<std::string> expectedTraversal =
      { "A", "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(tree.GetHead()->GetFirstChild());
      const auto end = Tree<std::string>::LeafIterator();
      for (; itr != end; ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("Tree<T>::beginLeaf and Tree<T>::endLeaf")
   {
      const std::vector<std::string> expectedTraversal =
      { "A", "C", "E", "H", };

      std::vector<std::string> output;
      std::transform(tree.beginLeaf(), tree.endLeaf(), std::back_inserter(output),
         [](Tree<std::string>::const_reference node)
      {
         return node.GetData();
      });

      REQUIRE(output.size() == expectedTraversal.size());
   }
}

TEST_CASE("Sibling Iterator")
{
   Tree<std::string> tree{ "IDK" };
   tree.GetHead()->AppendChild("B");
   tree.GetHead()->AppendChild("D");
   tree.GetHead()->AppendChild("A");
   tree.GetHead()->AppendChild("C");
   tree.GetHead()->AppendChild("F");
   tree.GetHead()->AppendChild("G");
   tree.GetHead()->AppendChild("E");
   tree.GetHead()->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expectedTraversal =
      { "B", "D", "A", "C", "F", "G", "E", "H", };

      int index = 0;

      bool traversalError = false;

      const auto& startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::SiblingIterator(startingNode);
      const auto end = Tree<std::string>::SiblingIterator();
      for (; itr != end; itr++)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("Sorting")
{
   SECTION("One Generation of Children")
   {
      Tree<std::string> tree{ "IDK" };
      tree.GetHead()->AppendChild("B");
      tree.GetHead()->AppendChild("D");
      tree.GetHead()->AppendChild("A");
      tree.GetHead()->AppendChild("C");
      tree.GetHead()->AppendChild("F");
      tree.GetHead()->AppendChild("G");
      tree.GetHead()->AppendChild("E");
      tree.GetHead()->AppendChild("H");

      const auto comparator =
         [](Tree<std::string>::const_reference lhs, Tree<std::string>::const_reference rhs)
      {
         return (lhs < rhs);
      };

      tree.GetHead()->SortChildren(comparator);

      const std::vector<std::string> expectedTraversal =
      { "A", "B", "C", "D", "E", "F", "G", "H", };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(tree.GetHead());
      const auto end = Tree<std::string>::LeafIterator();
      for (; itr != end; itr++)  ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("An Entire Tree")
   {
      Tree<int> tree{ 999 };
      tree.GetHead()->AppendChild(634);
      tree.GetHead()->GetFirstChild()->AppendChild(34);
      tree.GetHead()->GetFirstChild()->AppendChild(13);
      tree.GetHead()->GetFirstChild()->AppendChild(89);
      tree.GetHead()->GetFirstChild()->AppendChild(3);
      tree.GetHead()->GetFirstChild()->AppendChild(1);
      tree.GetHead()->GetFirstChild()->AppendChild(0);
      tree.GetHead()->GetFirstChild()->AppendChild(-5);

      tree.GetHead()->AppendChild(375);
      tree.GetHead()->AppendChild(173);
      tree.GetHead()->AppendChild(128);

      const auto sizeBeforeSort = tree.Size();

      // Sort:
      std::for_each(std::begin(tree), std::end(tree),
         [](Tree<int>::reference node)
      {
         node.SortChildren(
            [](Tree<int>::value_type lhs, Tree<int>::value_type rhs)
         {
            return lhs < rhs;
         });
      });

      const auto sizeAfterSort = tree.Size();

      bool sortingError = false;
      int lastItem = -999;

      // Verify:
      std::for_each(std::begin(tree), std::end(tree),
         [&](Tree<int>::const_reference node)
      {
         if (!node.HasChildren())
         {
            return;
         }

         auto child = node.GetFirstChild();
         while (child)
         {
            if (child->GetData() < lastItem)
            {
               sortingError = true;
               lastItem = child->GetData();
            }

            child = child->GetNextSibling();
         }
      });

      REQUIRE(sizeBeforeSort == sizeAfterSort);
      REQUIRE(sortingError == false);
   }
}

unsigned int CONSTRUCTION_COUNT = 0;
unsigned int DESTRUCTION_COUNT = 0;

struct VerboseNode
{
   VerboseNode(const char* const data) :
      m_data(data)
   {
      CONSTRUCTION_COUNT++;
   }

   ~VerboseNode()
   {
      DESTRUCTION_COUNT++;
   }

   std::string m_data;
};

TEST_CASE("Tree and TreeNode Destruction")
{
   SECTION("Node Destruction Count")
   {
      CONSTRUCTION_COUNT = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetHead()->AppendChild("B")->AppendChild("A");
         tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }
}