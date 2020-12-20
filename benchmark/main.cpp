#include <algorithm>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <string>

#include "tree.h"

#include "drive_scanner.h"
#include "scanning_progress.h"
#include "stopwatch.h"

#define BOOST_ASIO_DISABLE_CONCEPTS

namespace
{
#if _DEBUG
   constexpr auto trial_count = 1;
#else
   constexpr auto trial_count = 100;
#endif

   template <typename ChronoType, typename LambdaType>
   auto RunTrials(LambdaType&& trial)
   {
      std::vector<ChronoType> elapsedTimes;
      elapsedTimes.reserve(trial_count);

      for (int i = 0; i < elapsedTimes.capacity(); ++i)
      {
         const auto clock = Stopwatch<ChronoType>([&] { trial(); });
         elapsedTimes.emplace_back(clock.GetElapsedTime());
      }

      const auto sum = std::accumulate(
          std::begin(elapsedTimes),
          std::end(elapsedTimes),
          0ull,
          [](auto total, auto current) { return total + current.count(); });

      return sum / elapsedTimes.size();
   }

   void WaitAndReportProgress(const DriveScanner& scanner)
   {
      const auto& scanningProgress = scanner.GetProgress();

      while (true)
      {
         if (scanningProgress.scanCompleted.load())
         {
            return;
         }

         std::cout << "File scanned: " << scanningProgress.filesScanned.load() << "\t\r"
                   << std::flush;

         std::this_thread::sleep_for(std::chrono::seconds{ 1 });
      }
   }

   void RunPreOrderTrial(Tree<FileInfo>& tree)
   {
      using ChronoType = std::chrono::milliseconds;

      const auto preOrderTraversal = [&]() noexcept {
         std::uintmax_t treeSize{ 0 };
         std::uintmax_t totalBytes{ 0 };

         std::for_each(tree.beginPreOrder(), tree.endPreOrder(), [&](const auto& node) noexcept {
            treeSize += 1;

            if (node.GetData().type == FileType::REGULAR)
            {
               totalBytes += node.GetData().size;
            }
         });
      };

      std::cout << "Average Pre-Order Traversal Time: " << RunTrials<ChronoType>(preOrderTraversal)
                << " " << detail::ChronoTypeName<ChronoType>::value << "." << std::endl;
   }

   void RunPostOrderTrial(Tree<FileInfo>& tree)
   {
      using ChronoType = std::chrono::milliseconds;

      const auto postOrderTraversal = [&]() noexcept {
         std::uintmax_t treeSize{ 0 };
         std::uintmax_t totalBytes{ 0 };

         std::for_each(std::begin(tree), std::end(tree), [&](const auto& node) noexcept {
            treeSize += 1;

            if (node.GetData().type == FileType::REGULAR)
            {
               totalBytes += node.GetData().size;
            }
         });
      };

      std::cout << "Average Post-Order Traversal Time: "
                << RunTrials<ChronoType>(postOrderTraversal) << " "
                << detail::ChronoTypeName<ChronoType>::value << "." << std::endl;
   }
} // namespace

int main()
{
   std::cout.imbue(std::locale{ "" });
   std::cout << "Scanning drive to create a large tree...\n";

   DriveScanner scanner{ std::filesystem::path{ "C:\\" } };
   auto scanningThread = std::thread{ [&] { scanner.Start(); } };
   WaitAndReportProgress(scanner);
   scanningThread.join();

   std::cout << "\n\n";

   const auto tree = scanner.GetTree();
   RunPreOrderTrial(*tree);
   RunPostOrderTrial(*tree);

   return 0;
}