#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

#include "../Tree/Tree.hpp"

#include <algorithm>
#include <vector>

namespace
{
   /**
   * @brief Allows for the comparison of vectors of unequal length.
   *
   * @param[in] expected            The expected sequence.
   * @param[in] actual              The actual sequence.
   *
   * @note The `expected` vector has to be smaller than the `actual` vector that it is compared
   * against.
   */
   template<typename DataType>
   void SubsetEquals(
      const std::vector<DataType>& expected,
      const std::vector<DataType>& actual)
   {
      bool isCorrect{ false };

      const auto expectedSize = expected.size();
      const auto actualSize = actual.size();
      if (expectedSize < actualSize)
      {
         const auto delta = static_cast<int>(actualSize - expectedSize);

         auto actualEnd = std::end(actual);
         std::advance(actualEnd, -delta);

         isCorrect = std::equal(
            std::begin(actual), actualEnd,
            std::begin(expected), std::end(expected));
      }

      REQUIRE(isCorrect == true);
   }

   /**
   * @brief Compares two vectors, ensuring that both contain the same data.
   *
   * @param[in] expected                  The expected sequence.
   * @param[in] actual                    The actual sequence.
   * @param[in] performSubsetComparision  Forces the consumer to explicitly acknowledge that he
   *                                      or she wants to allow subset comparison.
   */
   template<typename DataType>
   void VerifyTraversal(
      const std::vector<DataType>& expected,
      const std::vector<DataType>& actual,
      bool performSubsetComparision = false)
   {
      if (performSubsetComparision)
      {
         SubsetEquals(expected, actual);
         return;
      }

      REQUIRE(expected.size() == actual.size());

      const auto isCorrect = std::equal(
         std::begin(actual), std::end(actual),
         std::begin(expected), std::end(expected));

      REQUIRE(isCorrect == true);
   }
}

TEST_CASE("Node Construction and Assignment")
{
   const Tree<std::string>::Node node{ "Bar" };

   SECTION("Default Construction")
   {
      const Tree<std::string>::Node default{};
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
      const Tree<std::string>::Node copy{ node };

      REQUIRE(copy.GetData() == node.GetData());
      REQUIRE(&copy.GetData() != &node.GetData());
   }

   SECTION("Assignment Operator")
   {
      const Tree<std::string>::Node copy = node;

      REQUIRE(copy.GetData() == node.GetData());
   }
}

TEST_CASE("Node Comparison Operations")
{
   const Tree<int>::Node ten{ 10 };
   const Tree<int>::Node twenty{ 20 };

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

TEST_CASE("Node Alterations")
{
   Tree<std::string>::Node node{ "Bar" };
   REQUIRE(node.GetData() == "Bar");

   SECTION("Altering Data")
   {
      auto& data = node.GetData();
      std::transform(std::begin(data), std::end(data), std::begin(data), ::toupper);

      REQUIRE(node.GetData() == "BAR");
   }
}

TEST_CASE("Prepending and Appending Nodes")
{
   Tree<int> tree{ 10 };

   const auto IsEachNodeValueLargerThanTheLast = [&] () noexcept
   {
      int lastValue = -1;

      return std::all_of(std::begin(tree), std::end(tree),
         [&] (Tree<int>::const_reference node)
      {
         return node.GetData() > lastValue;
      });
   };

   SECTION("Prepending Nodes")
   {
      tree.GetRoot()->AppendChild(1);
      tree.GetRoot()->AppendChild(2);
      tree.GetRoot()->AppendChild(3);
      tree.GetRoot()->AppendChild(4);
      tree.GetRoot()->AppendChild(5);
      tree.GetRoot()->AppendChild(6);
      tree.GetRoot()->AppendChild(7);
      tree.GetRoot()->AppendChild(8);
      tree.GetRoot()->AppendChild(9);

      const bool correctlyPrepended = IsEachNodeValueLargerThanTheLast();

      REQUIRE(correctlyPrepended);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 9);
   }

   SECTION("Appending Nodes")
   {
      tree.GetRoot()->PrependChild(9);
      tree.GetRoot()->PrependChild(8);
      tree.GetRoot()->PrependChild(7);
      tree.GetRoot()->PrependChild(6);
      tree.GetRoot()->PrependChild(5);
      tree.GetRoot()->PrependChild(4);
      tree.GetRoot()->PrependChild(3);
      tree.GetRoot()->PrependChild(2);
      tree.GetRoot()->PrependChild(1);

      const bool correctlyAppended = IsEachNodeValueLargerThanTheLast();

      REQUIRE(correctlyAppended);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 9);
   }
}

TEST_CASE("Node Counting")
{
   Tree<std::string> tree{ "F" };
   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Tree Size")
   {
      REQUIRE(tree.Size() == 9);
   }

   SECTION("Node Depth")
   {
      REQUIRE(Tree<std::string>::Depth(*tree.GetRoot()) == 0);
   }

   SECTION("Descendent Count")
   {
      REQUIRE(tree.GetRoot()->GetFirstChild()->CountAllDescendants() == 4);
   }
}

TEST_CASE("Node::Iterators")
{
   const Tree<std::string>::Node node{ "Test" };
   const auto sharedNode = std::make_shared<Tree<std::string>::Node>(node);
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

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-order Traversal")
   {
      const std::vector<std::string> expected = { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      std::vector<std::string> actual;
      std::transform(tree.beginPreOrder(), tree.endPreOrder(), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-order Traversal")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Partial Tree Iteration")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "B", "A", "D", "C", "E" };

      const auto begin = decltype(tree)::PreOrderIterator{ tree.GetRoot()->GetFirstChild() };
      const auto end = decltype(tree)::PreOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B" };

      const auto begin = decltype(tree)::PostOrderIterator{ tree.GetRoot()->GetFirstChild() };
      const auto end = decltype(tree)::PostOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Partial Tree Iteration Corner Cases")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "D", "C", "E" };

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

      const auto begin = decltype(tree)::PreOrderIterator{ startingNode };
      const auto end = decltype(tree)::PreOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expected = { "C", "E", "D" };

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

      const auto begin = decltype(tree)::PostOrderIterator{ startingNode };
      const auto end = decltype(tree)::PostOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Leaf Iteration")
   {
      const std::vector<std::string> expected = { "C", "E" };

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

      const auto begin = decltype(tree)::LeafIterator{ startingNode };
      const auto end = decltype(tree)::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("STL Typedef Compliance")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Standard Algorithms and Parameter Passing by value_type")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::value_type node) noexcept
      {
         return (node.GetData() == "A");
      });

      REQUIRE(count == 1);
   }

   SECTION("Standard Algorithms and Parameter Passing by reference")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::reference node) noexcept
      {
         return (node.GetData() == "C");
      });

      REQUIRE(count == 1);
   }

   SECTION("Standard Algorithms and Parameter Passing by const_reference")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::const_reference node) noexcept
      {
         return (node.GetData() == "D");
      });

      REQUIRE(count == 1);
   }
}

TEST_CASE("Leaf Iterator")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "H", };

      const auto begin = decltype(tree)::LeafIterator{ tree.GetRoot() };
      const auto end = decltype(tree)::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Partial Tree Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E" };

      const auto begin = decltype(tree)::LeafIterator{ tree.GetRoot()->GetFirstChild() };
      const auto end = decltype(tree)::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Tree<T>::beginLeaf and Tree<T>::endLeaf")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "H", };

      std::vector<std::string> actual;
      std::transform(tree.beginLeaf(), tree.endLeaf(), std::back_inserter(actual),
         [] (Tree<std::string>::const_reference node) { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Sibling Iterator")
{
   Tree<std::string> tree{ "IDK" };

   tree.GetRoot()->AppendChild("B");
   tree.GetRoot()->AppendChild("D");
   tree.GetRoot()->AppendChild("A");
   tree.GetRoot()->AppendChild("C");
   tree.GetRoot()->AppendChild("F");
   tree.GetRoot()->AppendChild("G");
   tree.GetRoot()->AppendChild("E");
   tree.GetRoot()->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "B", "D", "A", "C", "F", "G", "E", "H", };

      const auto begin = decltype(tree)::SiblingIterator{ tree.GetRoot()->GetFirstChild() };
      const auto end = decltype(tree)::SiblingIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Sorting")
{
   SECTION("One Generation of Children")
   {
      Tree<std::string> tree{ "X" };

      tree.GetRoot()->AppendChild("B");
      tree.GetRoot()->AppendChild("D");
      tree.GetRoot()->AppendChild("A");
      tree.GetRoot()->AppendChild("C");
      tree.GetRoot()->AppendChild("F");
      tree.GetRoot()->AppendChild("G");
      tree.GetRoot()->AppendChild("E");
      tree.GetRoot()->AppendChild("H");

      tree.GetRoot()->SortChildren(
         [] (const auto& lhs, const auto& rhs) noexcept { return (lhs < rhs); });

      const std::vector<std::string> expected = { "A", "B", "C", "D", "E", "F", "G", "H", "X" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("An Entire Tree")
   {
      Tree<int> tree{ 666 };
      tree.GetRoot()->AppendChild(37);

      tree.GetRoot()->GetFirstChild()->AppendChild(6);
      tree.GetRoot()->GetFirstChild()->AppendChild(8);
      tree.GetRoot()->GetFirstChild()->AppendChild(2);
      tree.GetRoot()->GetFirstChild()->AppendChild(3);
      tree.GetRoot()->GetFirstChild()->AppendChild(1);
      tree.GetRoot()->GetFirstChild()->AppendChild(0);
      tree.GetRoot()->GetFirstChild()->AppendChild(-5);

      tree.GetRoot()->AppendChild(48);
      tree.GetRoot()->AppendChild(17);
      tree.GetRoot()->AppendChild(12);

      const auto sizeBeforeSort = tree.Size();

      std::for_each(std::begin(tree), std::end(tree),
         [] (auto& node) noexcept
      {
         node.SortChildren([] (auto& lhs, auto& rhs) noexcept { return lhs < rhs; });
      });

      const auto sizeAfterSort = tree.Size();

      REQUIRE(sizeBeforeSort == sizeAfterSort);

      const std::vector<int> expected = { 12, 17, -5, 0, 1, 2, 3, 6, 8, 37, 48, 666 };

      std::vector<int> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Node Copying")
{
   Tree<std::string>::Node node{ "Node" };

   const auto copy = node;

   REQUIRE(node.GetData() == copy.GetData());
}

TEST_CASE("Tree Copying")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   const auto copy = tree;

   REQUIRE(tree.Size() == copy.Size());

   SECTION("Pre-order Verification")
   {
      const std::vector<std::string> expected = { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      std::vector<std::string> actual;
      std::transform(tree.beginPreOrder(), tree.endPreOrder(), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-order Verification")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

auto CONSTRUCTION_COUNT{ 0 };
auto DESTRUCTION_COUNT{ 0 };

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

TEST_CASE("Tree and Node Destruction")
{
   SECTION("Node Destruction Count")
   {
      CONSTRUCTION_COUNT = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }
}

TEST_CASE("Selectively Delecting Nodes")
{
   SECTION("Removing a Leaf Node Without Siblings")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetRoot()->GetLastChild()->GetLastChild()->GetFirstChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "H");
         REQUIRE(targetNode->GetPreviousSibling() == nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with A Left Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetRoot()->GetFirstChild()->GetLastChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "E");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "C", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with A Right Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetRoot()->GetFirstChild()->GetLastChild()->GetFirstChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "C");
         REQUIRE(targetNode->GetPreviousSibling() == nullptr);
         REQUIRE(targetNode->GetNextSibling() != nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "E", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with Both Left and Right Siblings")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("X");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode =
            tree.GetRoot()->GetFirstChild()->GetLastChild()->GetFirstChild()->GetNextSibling();

         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "X");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() != nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Node With a Left Sibling and Two Children")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetRoot()->GetFirstChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "D");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() != nullptr);
         REQUIRE(targetNode->GetFirstChild() != targetNode->GetLastChild());

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 3);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - 1);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Deleting a Node by Calling DeleteFromTree()")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         auto* targetNode = tree.GetRoot()->GetFirstChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "D");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() != nullptr);
         REQUIRE(targetNode->GetFirstChild() != targetNode->GetLastChild());

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         targetNode->DeleteFromTree();

         REQUIRE(DESTRUCTION_COUNT == 3);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - 1);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expected = { "A", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Deleting Multiple Nodes from Tree")
   {
      CONSTRUCTION_COUNT = 0;
      std::int64_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         tree.GetRoot()->AppendChild("B")->AppendChild("Delete Me");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("Delete Me");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("Delete Me");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("Delete Me");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         std::vector<Tree<VerboseNode>::Node*> toBeDeleted;

         for (auto&& node : tree)
         {
            if (node.GetData().m_data == "Delete Me")
            {
               toBeDeleted.emplace_back(&node);
            }
         }

         const auto numberOfNodesToDelete = static_cast<int>(toBeDeleted.size());

         for (auto* node : toBeDeleted)
         {
            node->DeleteFromTree();
         }

         const std::vector<std::string> expected = { "D", "B", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);

         REQUIRE(DESTRUCTION_COUNT == numberOfNodesToDelete);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }
}
