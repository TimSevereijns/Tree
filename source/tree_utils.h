#pragma once

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

#include "tree.h"

namespace
{
   /**
    * @brief Allows std::wstring data to be output to a narrow stream.
    *
    * @note Depending on what code page is active, this may not do what you expect.
    */
   std::ostream& operator<<(std::ostream& stream, std::wstring wideString)
   {
      using WideToNarrowConverterType = std::codecvt_utf8<wchar_t>;
      thread_local static std::wstring_convert<WideToNarrowConverterType, wchar_t> converter;

      stream << converter.to_bytes(wideString);
      return stream;
   }
} // namespace

namespace TreeUtilities
{
   template <typename NodeType>
   void OutputToDotFile(const Tree<NodeType>& tree, const std::string& fileName)
   {
      std::stringstream graphStream;

      graphStream << "digraph {\n"
                  << "   rankdir = TB;\n"
                  << "   edge [arrowsize=0.4, fontsize=10]\n";

      const Node<NodeType>* head = tree.GetRoot();

      graphStream << "\n"
                  << "   // Node Declarations:\n";

      std::for_each(
          Tree<NodeType>::PreOrderIterator{ head },
          Tree<NodeType>::PreOrderIterator{},
          [&](Tree<NodeType>::const_reference node) {
             const auto nodeLabel = std::to_string(reinterpret_cast<size_t>(&node));
             const auto& data = node.GetData();

             graphStream << "   " << nodeLabel << " [label = \"" << data << "\"]\n";
          });

      graphStream << "\n"
                  << "   // Edge Declarations:\n";

      std::for_each(
          Tree<NodeType>::PreOrderIterator{ head },
          Tree<NodeType>::PreOrderIterator{},
          [&](Tree<NodeType>::const_reference node) {
             const auto* parent = node.GetParent();
             if (!parent)
             {
                return;
             }

             const auto parentLabel = std::to_string(reinterpret_cast<std::size_t>(parent));
             const auto childLabel = std::to_string(reinterpret_cast<std::size_t>(&node));

             graphStream << "   " << parentLabel << " -> " << childLabel << "\n";
          });

      graphStream << "}" << std::flush;

      std::ofstream outputFile{ fileName, std::ofstream::out };

      outputFile << graphStream.rdbuf();
   }
} // namespace TreeUtilities
