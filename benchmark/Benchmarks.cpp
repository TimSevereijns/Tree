#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <filesystem>

#include "tree.h"

#include "DriveScanner.h"
#include "Stopwatch.hpp"

#define BOOST_ASIO_DISABLE_CONCEPTS

namespace
{
#if _DEBUG
   constexpr auto TRIAL_COUNT{ 1 };
#else
   constexpr auto TRIAL_COUNT{ 100 };
#endif

   template<
      typename ChronoType,
      typename LambdaType
   >
   auto RunTrials(LambdaType&& lambda)
   {
      std::vector<ChronoType> elapsedTimes;
      elapsedTimes.reserve(TRIAL_COUNT);

      for (auto index{ 0u }; index < elapsedTimes.capacity(); ++index)
      {
         const auto clock = Stopwatch<ChronoType>([&] () noexcept { lambda(); });
         elapsedTimes.emplace_back(clock.GetElapsedTime());
      }

      const auto sum = std::accumulate(std::begin(elapsedTimes), std::end(elapsedTimes), 0ull,
         [] (auto total, auto current) noexcept { return total + current.count(); });

      return sum / elapsedTimes.size();
   }
}

int main()
{
   using ChronoType = std::chrono::milliseconds;

   std::cout.imbue(std::locale{ "" });
   std::cout << "Scanning Drive to Create a Large Tree...\n" << std::endl;

   DriveScanner scanner{ std::filesystem::path{ "C:\\" } };
   scanner.Start();

   std::cout << "\n";

   const auto tree = scanner.GetTree();

   const auto leafCount = std::count_if(tree->beginLeaf(), tree->endLeaf(),
      [] (const auto&) noexcept { return true; });

   std::cout << "Tree Leaf Count: " << leafCount << "\n";

   const auto preOrderTraversal = [&] () noexcept
   {
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      std::for_each(
         tree->beginPreOrder(),
         tree->endPreOrder(),
         [&] (const auto& node) noexcept
      {
         treeSize += 1;

         if (node.GetData().type == FileType::REGULAR)
         {
            totalBytes += node.GetData().size;
         }
      });
   };

   const auto postOrderTraversal = [&] () noexcept
   {
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      std::for_each(
         std::begin(*tree),
         std::end(*tree),
         [&] (const auto& node) noexcept
      {
         treeSize += 1;

         if (node.GetData().type == FileType::REGULAR)
         {
            totalBytes += node.GetData().size;
         }
      });
   };

   std::cout
      << "Average Pre-Order Traversal Time: " << RunTrials<ChronoType>(preOrderTraversal)
      << " " << StopwatchInternals::TypeName<ChronoType>::value << ".\n";

   std::cout
      << "Average Post-Order Traversal Time: " << RunTrials<ChronoType>(postOrderTraversal)
      << " " << StopwatchInternals::TypeName<ChronoType>::value << ".\n";

   std::cout << std::endl;

   return 0;
}