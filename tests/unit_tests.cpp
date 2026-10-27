// ==================================================================
//                  __        .-.
//             .-"` .`'.    /\\|.       -----------------------------
//    _(\-/)_" ,  .   ,\  /\\\/.        |                           |
//    {(#b^d#)} .   ./,  |/\\\/         |       ATTENTION!!!        |
//    `-.(Y).-`  ,  |  , |\.-`          | THIS FILE IS FULLY GENE - |
//         /~/,_/~~~\,__.-`             | RATED BY AI               |
//        ////~    // ~\\               |___________________________|
//       ==`==`   ==`   ==`
// ==================================================================
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <exception>
#include <utility>
#include <vector>
#include <cstddef>
#include <string>


import prog_error;
import std_set_impl;
import sorted_vector_impl;
import avl_tree_impl;
import b_tree_impl;

namespace {

using dynamic_order::SortedVectorImpl;
using dynamic_order::StdSetImpl;
using prog_error::ErrorCode;
using prog_error::ProgError;

// Эталонное количество ключей из (left, right].
size_t CountInRange(const std::set<int>& ref, int left, int right) {
  if (left >= right) {
    return 0;
  }
  return static_cast<size_t>(
      std::distance(std::upper_bound(ref.begin(), ref.end(), left),
                    std::upper_bound(ref.begin(), ref.end(), right)));
}

size_t CountLess(const std::set<int>& ref, int key) {
  return static_cast<size_t>(
      std::distance(ref.begin(), std::lower_bound(ref.begin(), ref.end(), key)));
}

// ======================================================================
// StdSetImpl
// ======================================================================

class StdSetImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // {1, 3, 5, 7, 9}
    for (int key : {1, 3, 5, 7, 9}) {
      set_.Insert(key);
    }
  }

  StdSetImpl set_;
};

// ------------ Insert ------------

TEST(StdSetImplInsert, EmptySetIsEmpty) {
  StdSetImpl s;
  EXPECT_EQ(s.Query(-1000, 1000), 0u);
  EXPECT_THROW(s.Min(1), ProgError);
  EXPECT_EQ(s.LessKeysCounter(0), 0u);
}

TEST(StdSetImplInsert, SingleKey) {
  StdSetImpl s;
  s.Insert(42);
  EXPECT_EQ(s.Query(41, 42), 1u);
  EXPECT_EQ(s.Query(42, 43), 0u);  // 42 — левый конец, не считается
  EXPECT_EQ(s.Min(1), 42);
  EXPECT_THROW(s.Min(2), ProgError);
}

TEST(StdSetImplInsert, DuplicatesAreIgnored) {
  StdSetImpl s;
  s.Insert(42);
  s.Insert(42);
  s.Insert(42);
  EXPECT_EQ(s.Query(41, 42), 1u);
  EXPECT_EQ(s.LessKeysCounter(42), 0u);
  EXPECT_EQ(s.Min(1), 42);
  EXPECT_THROW(s.Min(2), ProgError);  // размер остался 1
}

TEST(StdSetImplInsert, UnorderedInsertionsKeepOrder) {
  StdSetImpl s;
  for (int key : {10, -5, 7, 0, 7, 100}) {
    s.Insert(key);
  }
  EXPECT_EQ(s.Min(1), -5);
  EXPECT_EQ(s.Min(2), 0);
  EXPECT_EQ(s.Min(3), 7);
  EXPECT_EQ(s.Min(4), 10);
  EXPECT_EQ(s.Min(5), 100);
  EXPECT_EQ(s.Query(-6, 100), 5u);
}

TEST(StdSetImplInsert, IntMinMaxBounds) {
  StdSetImpl s;
  s.Insert(INT32_MIN);
  s.Insert(INT32_MAX);
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 1u);  // только INT32_MAX
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(s.Query(INT32_MAX - 1, INT32_MAX), 1u);
  EXPECT_EQ(s.Min(1), INT32_MIN);
  EXPECT_EQ(s.Min(2), INT32_MAX);
}

// ------------ Query ------------

TEST_F(StdSetImplTest, QueryCountsKeysInsideSegment) {
  EXPECT_EQ(set_.Query(2, 8), 3u);  // 3, 5, 7
}

TEST_F(StdSetImplTest, QueryCountsAllKeys) {
  EXPECT_EQ(set_.Query(0, 10), 5u);
  EXPECT_EQ(set_.Query(-1000, 1000), 5u);
  EXPECT_EQ(set_.Query(INT32_MIN, INT32_MAX), 5u);
}

TEST_F(StdSetImplTest, QueryLeftBoundaryIsExcluded) {
  EXPECT_EQ(set_.Query(5, 100), 2u);  // 7, 9 — ключ 5 НЕ считается
  EXPECT_EQ(set_.Query(1, 3), 1u);    // только 3
  EXPECT_EQ(set_.Query(3, 5), 1u);    // только 5
  EXPECT_EQ(set_.Query(9, 9), 0u);
}

TEST_F(StdSetImplTest, QueryRightBoundaryIsIncluded) {
  EXPECT_EQ(set_.Query(-100, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(set_.Query(-100, 1), 1u);  // 1 — правый конец
  EXPECT_EQ(set_.Query(4, 5), 1u);     // 5 — правый конец
}

TEST_F(StdSetImplTest, QueryDegenerateAndInvertedRanges) {
  EXPECT_EQ(set_.Query(5, 5), 0u);
  EXPECT_EQ(set_.Query(0, 0), 0u);
  EXPECT_EQ(set_.Query(7, 3), 0u);   // пустой диапазон
  EXPECT_EQ(set_.Query(1, 0), 0u);
  EXPECT_EQ(set_.Query(INT32_MAX, INT32_MIN), 0u);
}

TEST_F(StdSetImplTest, QueryWhenAllKeysBelowLeft) {
  EXPECT_EQ(set_.Query(10, 20), 0u);
  EXPECT_EQ(set_.Query(9, 100), 0u);   // upper_bound(9) == end()
}

TEST_F(StdSetImplTest, QueryWhenAllKeysAboveRight) {
  EXPECT_EQ(set_.Query(-20, 0), 0u);   // цикл не выполняется: 1 > 0
  EXPECT_EQ(set_.Query(-20, -10), 0u);
}

TEST_F(StdSetImplTest, QueryWorksWithNegativeKeys) {
  StdSetImpl s;
  s.Insert(-10);
  s.Insert(-5);
  s.Insert(0);
  s.Insert(5);
  EXPECT_EQ(s.Query(-10, 5), 3u);     // -5, 0, 5
  EXPECT_EQ(s.Query(-6, -1), 1u);     // только -5
  EXPECT_EQ(s.Query(-10, -10), 0u);
  EXPECT_EQ(s.Query(-9, -5), 1u);     // правый конец включён
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 4u);
}

TEST(StdSetImplQuery, IsStableAcrossRepeatedCalls) {
  StdSetImpl s;
  s.Insert(1);
  s.Insert(2);
  EXPECT_EQ(s.Query(0, 2), 2u);
  EXPECT_EQ(s.Query(0, 2), 2u);  // повторный запрос не меняет состояние
}

// ------------ Min ------------

TEST_F(StdSetImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(set_.Min(1), 1);   // минимум
  EXPECT_EQ(set_.Min(3), 5);
  EXPECT_EQ(set_.Min(5), 9);   // максимум
}

TEST(StdSetImplMinErrors, ThrowsOnZeroIndex) {
  StdSetImpl s;
  s.Insert(1);
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(StdSetImplMinErrors, ThrowsOnIndexGreaterThanSize) {
  StdSetImpl s;
  s.Insert(1);
  s.Insert(2);
  EXPECT_THROW(s.Min(3), ProgError);
  EXPECT_THROW(s.Min(100), ProgError);
}

TEST(StdSetImplMinErrors, ThrowsOnAnyIndexForEmptySet) {
  StdSetImpl s;
  EXPECT_THROW(s.Min(1), ProgError);
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(StdSetImplMinErrors, ProgErrorCarriesContext) {
  StdSetImpl s;
  try {
    s.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
    EXPECT_EQ(e.GetErrorInfo().func_name, "dynamic_order::StdSetImpl::Min");
  }
}

TEST(StdSetImplMinErrors, ProgIsStdException) {
  StdSetImpl s;
  EXPECT_THROW(s.Min(0), std::exception);  // ProgError — потомок std::exception
}

// ------------ LessKeysCounter ------------

TEST_F(StdSetImplTest, LessKeysCounterCountsStrictlyLess) {
  EXPECT_EQ(set_.LessKeysCounter(1), 0u);    // ничего меньше 1 нет
  EXPECT_EQ(set_.LessKeysCounter(2), 1u);    // {1}
  EXPECT_EQ(set_.LessKeysCounter(5), 2u);    // {1, 3}
  EXPECT_EQ(set_.LessKeysCounter(10), 5u);   // все (полный проход без break)
  EXPECT_EQ(set_.LessKeysCounter(INT32_MAX), 5u);
}

TEST_F(StdSetImplTest, LessKeysCounterDoesNotCountEqualKey) {
  EXPECT_EQ(set_.LessKeysCounter(3), 1u);
  EXPECT_EQ(set_.LessKeysCounter(7), 3u);
}

TEST_F(StdSetImplTest, LessKeysCounterForVerySmallKey) {
  EXPECT_EQ(set_.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(set_.LessKeysCounter(0), 0u);
}

TEST(StdSetImplLessKeysCounter, EmptySetReturnsZero) {
  StdSetImpl s;
  EXPECT_EQ(s.LessKeysCounter(0), 0u);
  EXPECT_EQ(s.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(s.LessKeysCounter(INT32_MAX), 0u);
}

TEST_F(StdSetImplTest, LessKeysCounterIsMonotonic) {
  for (int k = -5; k < 15; ++k) {
    EXPECT_GE(set_.LessKeysCounter(k + 1), set_.LessKeysCounter(k)) << "k=" << k;
  }
}

// ------------ Внутренняя согласованность StdSetImpl ------------

TEST_F(StdSetImplTest, MinAndLessKeysCounterAreConsistent) {
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(set_.LessKeysCounter(set_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST_F(StdSetImplTest, QueryAndLessKeysCounterIdentity) {
  // Для целых ключей: Query(a, b) == LessKeysCounter(b + 1) - LessKeysCounter(a + 1)
  for (int a = -3; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(set_.Query(a, b),
                set_.LessKeysCounter(b + 1) - set_.LessKeysCounter(a + 1))
          << "range (" << a << ", " << b << "]";
    }
  }
}

TEST(StdSetImplReference, MatchesStdSetOnGrid) {
  StdSetImpl s;
  std::set<int> ref;
  const int vals[] = {17, -3, 0, 42, 42, -100, 7, 999, 1000000, -1000000};
  for (int v : vals) {
    s.Insert(v);
    ref.insert(v);
  }

  for (int a = -10; a <= 100; a += 7) {
    for (int b = -10; b <= 100; b += 11) {
      EXPECT_EQ(s.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = -100; k <= 100; ++k) {
    EXPECT_EQ(s.LessKeysCounter(k), CountLess(ref, k)) << "k = " << k;
  }
  for (size_t k = 1; k <= ref.size(); ++k) {
    EXPECT_EQ(s.Min(k), *std::next(ref.begin(), static_cast<long>(k - 1)))
        << "k = " << k;
  }
}

// ======================================================================
// SortedVectorImpl
// ======================================================================

class SortedVectorImplTest : public ::testing::Test {
 protected:
  // {9, 3, 1, 7, 5} неразсортированными — конструктор обязан отсортировать
  SortedVectorImpl set_{std::vector<int>{9, 3, 1, 7, 5}};
};

// ------------ Конструкторы ------------

TEST(SortedVectorImplCtor, DefaultConstructedIsEmpty) {
  SortedVectorImpl s;
  EXPECT_EQ(s.Query(-1000, 1000), 0u);
  EXPECT_THROW(s.Min(1), ProgError);
  EXPECT_EQ(s.LessKeysCounter(0), 0u);
}

TEST(SortedVectorImplCtor, EmptyVector) {
  SortedVectorImpl s{std::vector<int>{}};
  EXPECT_EQ(s.Query(-1000, 1000), 0u);
  EXPECT_THROW(s.Min(1), ProgError);
}

TEST(SortedVectorImplCtor, SortsUnsortedInput) {
  SortedVectorImpl s{std::vector<int>{9, 3, 1, 7, 5}};
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(3), 5);
  EXPECT_EQ(s.Min(5), 9);
  EXPECT_EQ(s.Query(0, 10), 5u);
}

TEST(SortedVectorImplCtor, AlreadySortedInputStaysValid) {
  SortedVectorImpl s{std::vector<int>{1, 2, 3, 4, 5}};
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(5), 5);
  EXPECT_EQ(s.Query(1, 4), 3u);  // 2, 3, 4
}

TEST(SortedVectorImplCtor, RemovesDuplicatesFromInput) {
  SortedVectorImpl s{std::vector<int>{2, 1, 2, 3, 1, 2}};
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(2), 2);
  EXPECT_EQ(s.Min(3), 3);
  EXPECT_THROW(s.Min(4), ProgError);  // размер = 3, не 6
  EXPECT_EQ(s.Query(0, 4), 3u);
  EXPECT_EQ(s.LessKeysCounter(2), 1u);
}

TEST(SortedVectorImplCtor, SingleElement) {
  SortedVectorImpl s{std::vector<int>{42}};
  EXPECT_EQ(s.Min(1), 42);
  EXPECT_EQ(s.Query(41, 42), 1u);
  EXPECT_EQ(s.Query(42, 43), 0u);
}

TEST(SortedVectorImplCtor, IntMinMaxBounds) {
  SortedVectorImpl s{std::vector<int>{INT32_MAX, INT32_MIN}};
  EXPECT_EQ(s.Min(1), INT32_MIN);
  EXPECT_EQ(s.Min(2), INT32_MAX);
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 1u);
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(s.Query(INT32_MAX, INT32_MAX), 0u);
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MIN + 1), 0u);  // между границами ключей нет
}

TEST(SortedVectorImplCtor, NegativeKeys) {
  SortedVectorImpl s{std::vector<int>{5, -10, 0, -5}};
  EXPECT_EQ(s.Min(1), -10);
  EXPECT_EQ(s.Min(4), 5);
  EXPECT_EQ(s.Query(-10, 5), 3u);   // -5, 0, 5
  EXPECT_EQ(s.Query(-6, -1), 1u);   // только -5
  EXPECT_EQ(s.Query(-10, -10), 0u);
  EXPECT_EQ(s.Query(-9, -5), 1u);
}

// ------------ Insert + ленивое слияние ------------

TEST_F(SortedVectorImplTest, QuerySeesPendingInserts) {
  set_.Insert(6);
  EXPECT_EQ(set_.Query(5, 7), 2u);       // 6 и 7 (правый конец включается)
  EXPECT_EQ(set_.Query(2, 8), 4u);       // 3, 5, 6, 7
  EXPECT_EQ(set_.Query(-1000, 1000), 6u);
}

TEST_F(SortedVectorImplTest, MinSeesPendingInserts) {
  set_.Insert(0);
  set_.Insert(10);
  EXPECT_EQ(set_.Min(1), 0);
  EXPECT_EQ(set_.Min(7), 10);
}

TEST_F(SortedVectorImplTest, LessKeysCounterSeesPendingInserts) {
  set_.Insert(4);
  EXPECT_EQ(set_.LessKeysCounter(5), 3u);  // {1, 3, 4}
  EXPECT_EQ(set_.LessKeysCounter(4), 2u);  // {1, 3}
}

TEST_F(SortedVectorImplTest, PendingKeyAffectsBoundaries) {
  set_.Insert(2);
  EXPECT_EQ(set_.Query(1, 2), 1u);   // вставленная 2 как правый конец
  EXPECT_EQ(set_.Query(2, 3), 1u);   // 2 — левый конец, считается только 3
  EXPECT_EQ(set_.Query(0, 2), 2u);   // 1 и 2
}

TEST_F(SortedVectorImplTest, MultiplePendingInsertsOfSameKeyMergeOnce) {
  set_.Insert(100);
  set_.Insert(100);
  set_.Insert(100);
  EXPECT_EQ(set_.Query(0, 1000), 6u);  // 5 существующих + одна новая 100
  EXPECT_THROW(set_.Min(7), ProgError);
}

TEST_F(SortedVectorImplTest, DuplicatesAgainstExistingKeysRemovedOnMerge) {
  set_.Insert(5);  // 5 уже есть
  set_.Insert(3);  // 3 уже есть
  set_.Insert(5);  // повторно
  EXPECT_EQ(set_.Query(0, 10), 5u);    // множество не распухло
  EXPECT_EQ(set_.Query(4, 5), 1u);     // одна копия 5
  EXPECT_THROW(set_.Min(6), ProgError);
  EXPECT_EQ(set_.LessKeysCounter(6), 3u);  // {1, 3, 5}
}

TEST_F(SortedVectorImplTest, MergesRepeatAcrossBatches) {
  set_.Insert(4);
  EXPECT_EQ(set_.Query(3, 5), 2u);   // 4 и 5
  set_.Insert(8);
  EXPECT_EQ(set_.Query(7, 9), 2u);   // 8 и 9
  EXPECT_EQ(set_.Query(3, 9), 5u);   // 4, 5, 7, 8, 9
  set_.Insert(2);
  set_.Insert(6);
  EXPECT_EQ(set_.Query(0, 10), 9u);  // {1..9} целиком
}

TEST_F(SortedVectorImplTest, InsertAfterMergeIsVisible) {
  EXPECT_EQ(set_.Query(0, 10), 5u);  // first merge
  set_.Insert(6);
  set_.Insert(4);
  EXPECT_EQ(set_.Query(4, 6), 2u);   // 5 и 6
  EXPECT_EQ(set_.Query(3, 5), 2u);   // 4 и 5
  EXPECT_EQ(set_.Query(0, 10), 7u);
}

TEST(SortedVectorImplInsert, IntoDefaultConstructedEmpty) {
  SortedVectorImpl s;
  s.Insert(5);
  s.Insert(1);
  s.Insert(3);
  EXPECT_EQ(s.Query(0, 5), 3u);      // 1, 3, 5
  EXPECT_EQ(s.Query(5, 10), 0u);
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(3), 5);
  EXPECT_EQ(s.LessKeysCounter(3), 1u);
}

TEST(SortedVectorImplInsert, LargeBatch) {
  SortedVectorImpl s;
  std::set<int> ref;
  for (int i = 0; i < 1000; ++i) {
    int k = (i * 37) % 500 - 250;  // детерминированный «перемешанный» поток
    s.Insert(k);
    ref.insert(k);
  }
  EXPECT_EQ(s.Query(-251, 250), ref.size());
  EXPECT_EQ(s.Min(1), *ref.begin());
  EXPECT_EQ(s.Min(ref.size()), *ref.rbegin());
  EXPECT_EQ(s.LessKeysCounter(0), CountLess(ref, 0));
}

// ------------ Query ------------

TEST_F(SortedVectorImplTest, QueryCountsAllKeys) {
  EXPECT_EQ(set_.Query(0, 10), 5u);
  EXPECT_EQ(set_.Query(INT32_MIN, INT32_MAX), 5u);
}

TEST_F(SortedVectorImplTest, QueryLeftBoundaryIsExcluded) {
  EXPECT_EQ(set_.Query(5, 100), 2u);  // 7, 9
  EXPECT_EQ(set_.Query(1, 3), 1u);
  EXPECT_EQ(set_.Query(3, 5), 1u);
}

TEST_F(SortedVectorImplTest, QueryRightBoundaryIsIncluded) {
  EXPECT_EQ(set_.Query(-100, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(set_.Query(-100, 1), 1u);
  EXPECT_EQ(set_.Query(8, 9), 1u);
}

TEST_F(SortedVectorImplTest, QueryDegenerateAndInvertedRanges) {
  EXPECT_EQ(set_.Query(5, 5), 0u);
  EXPECT_EQ(set_.Query(0, 0), 0u);
  EXPECT_EQ(set_.Query(7, 3), 0u);
  EXPECT_EQ(set_.Query(INT32_MAX, INT32_MIN), 0u);
}

TEST_F(SortedVectorImplTest, QueryWhenAllKeysBelowLeft) {
  EXPECT_EQ(set_.Query(10, 20), 0u);
  EXPECT_EQ(set_.Query(9, 100), 0u);   // lower_bound == end()
}

TEST_F(SortedVectorImplTest, QueryWhenAllKeysAboveRight) {
  EXPECT_EQ(set_.Query(-20, 0), 0u);
  EXPECT_EQ(set_.Query(-20, -10), 0u);
}

TEST_F(SortedVectorImplTest, QueryIsStableAcrossRepeatedCalls) {
  EXPECT_EQ(set_.Query(0, 10), 5u);
  EXPECT_EQ(set_.Query(0, 10), 5u);
}

// ------------ Min ------------

TEST_F(SortedVectorImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(set_.Min(1), 1);
  EXPECT_EQ(set_.Min(3), 5);
  EXPECT_EQ(set_.Min(5), 9);
}

TEST(SortedVectorImplMinErrors, ThrowsOnZeroIndex) {
  SortedVectorImpl s{std::vector<int>{1}};
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(SortedVectorImplMinErrors, ThrowsOnIndexGreaterThanSize) {
  SortedVectorImpl s{std::vector<int>{1, 2}};
  EXPECT_THROW(s.Min(3), ProgError);
  EXPECT_THROW(s.Min(100), ProgError);
}

TEST(SortedVectorImplMinErrors, ThrowsOnAnyIndexForEmptySet) {
  SortedVectorImpl s;
  EXPECT_THROW(s.Min(1), ProgError);
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(SortedVectorImplMinErrors, ThrowsEvenWithPendingInsertsOutOfRange) {
  SortedVectorImpl s{std::vector<int>{1, 2}};
  s.Insert(3);
  EXPECT_EQ(s.Min(3), 3);
  EXPECT_THROW(s.Min(4), ProgError);
}

TEST(SortedVectorImplMinErrors, ProgErrorCarriesContext) {
  SortedVectorImpl s;
  try {
    s.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
    EXPECT_EQ(e.GetErrorInfo().func_name, "dynamic_order::SortedVectorImpl::Min");
  }
}

TEST(SortedVectorImplMinErrors, ProgIsStdException) {
  SortedVectorImpl s;
  EXPECT_THROW(s.Min(0), std::exception);
}

// ------------ LessKeysCounter ------------

TEST_F(SortedVectorImplTest, LessKeysCounterCountsStrictlyLess) {
  EXPECT_EQ(set_.LessKeysCounter(1), 0u);
  EXPECT_EQ(set_.LessKeysCounter(2), 1u);
  EXPECT_EQ(set_.LessKeysCounter(5), 2u);
  EXPECT_EQ(set_.LessKeysCounter(10), 5u);
  EXPECT_EQ(set_.LessKeysCounter(INT32_MAX), 5u);
}

TEST_F(SortedVectorImplTest, LessKeysCounterDoesNotCountEqualKey) {
  EXPECT_EQ(set_.LessKeysCounter(3), 1u);
  EXPECT_EQ(set_.LessKeysCounter(7), 3u);
}

TEST_F(SortedVectorImplTest, LessKeysCounterForVerySmallKey) {
  EXPECT_EQ(set_.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(set_.LessKeysCounter(0), 0u);
  EXPECT_EQ(set_.LessKeysCounter(1), 0u);  // key == front
}

TEST(SortedVectorImplLessKeysCounter, EmptySetReturnsZero) {
  SortedVectorImpl s;
  EXPECT_EQ(s.LessKeysCounter(0), 0u);
  EXPECT_EQ(s.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(s.LessKeysCounter(INT32_MAX), 0u);
}

TEST(SortedVectorImplLessKeysCounter, DefaultConstructedWithPendingInsertsOnly) {
  SortedVectorImpl s;
  s.Insert(7);
  s.Insert(3);
  EXPECT_EQ(s.LessKeysCounter(3), 0u);
  EXPECT_EQ(s.LessKeysCounter(4), 1u);
  EXPECT_EQ(s.LessKeysCounter(8), 2u);
}

TEST_F(SortedVectorImplTest, LessKeysCounterIsMonotonic) {
  for (int k = -5; k < 15; ++k) {
    EXPECT_GE(set_.LessKeysCounter(k + 1), set_.LessKeysCounter(k)) << "k=" << k;
  }
}

// ------------ Внутренняя согласованность SortedVectorImpl ------------

TEST_F(SortedVectorImplTest, MinAndLessKeysCounterAreConsistent) {
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(set_.LessKeysCounter(set_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST_F(SortedVectorImplTest, QueryAndLessKeysCounterIdentity) {
  for (int a = -3; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(set_.Query(a, b),
                set_.LessKeysCounter(b + 1) - set_.LessKeysCounter(a + 1))
          << "range (" << a << ", " << b << "]";
    }
  }
}

TEST(SortedVectorImplReference, MatchesStdSetOnGrid) {
  SortedVectorImpl s{
      std::vector<int>{17, 17, -3, 0, 42, 42, -100, 7, 999, 1000000, -1000000}};
  const int vals[] = {17, 17, -3, 0, 42, 42, -100, 7, 999, 1000000, -1000000};
  std::set<int> ref(vals, vals + sizeof(vals) / sizeof(vals[0]));

  for (int a = -10; a <= 100; a += 7) {
    for (int b = -10; b <= 100; b += 11) {
      EXPECT_EQ(s.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = -100; k <= 100; ++k) {
    EXPECT_EQ(s.LessKeysCounter(k), CountLess(ref, k)) << "k = " << k;
  }
  for (size_t k = 1; k <= ref.size(); ++k) {
    EXPECT_EQ(s.Min(k), *std::next(ref.begin(), static_cast<long>(k - 1)))
        << "k = " << k;
  }
}

// ======================================================================
// Дифференциальный тест: обе реализации против std::set и друг против друга
// ======================================================================

TEST(DynamicOrderDifferential, BothImplsMatchStdSetUnderRandomOperations) {
  std::mt19937 rng(20261015);
  std::uniform_int_distribution<int> key_dist(-500, 500);

  std::set<int> ref;
  StdSetImpl set_impl;
  SortedVectorImpl vec_impl;

  for (int step = 0; step < 3000; ++step) {
    const int op = step % 5;
    SCOPED_TRACE("step " + std::to_string(step) + ", op " + std::to_string(op));

    if (op == 0 || op == 1) {  // вставка (в т.ч. дубликатов)
      const int k = key_dist(rng);
      ref.insert(k);
      set_impl.Insert(k);
      vec_impl.Insert(k);
    } else if (op == 2) {  // диапазонный запрос
      const int a = key_dist(rng);
      const int b = key_dist(rng);
      const size_t expected = CountInRange(ref, a, b);
      EXPECT_EQ(set_impl.Query(a, b), expected);
      EXPECT_EQ(vec_impl.Query(a, b), expected);
    } else if (op == 3) {  // порядковая статистика
      if (ref.empty()) {
        EXPECT_THROW(set_impl.Min(1), ProgError);
        EXPECT_THROW(vec_impl.Min(1), ProgError);
      } else {
        const size_t k = 1 + (rng() % ref.size());
        const int expected = *std::next(ref.begin(), static_cast<long>(k - 1));
        EXPECT_EQ(set_impl.Min(k), expected);
        EXPECT_EQ(vec_impl.Min(k), expected);
        // заодно выход за границы
        EXPECT_THROW(set_impl.Min(ref.size() + 1), ProgError);
        EXPECT_THROW(vec_impl.Min(ref.size() + 1), ProgError);
      }
    } else {  // счётчик меньших ключей
      const int k = key_dist(rng);
      const size_t expected = CountLess(ref, k);
      EXPECT_EQ(set_impl.LessKeysCounter(k), expected);
      EXPECT_EQ(vec_impl.LessKeysCounter(k), expected);
    }
  }

  // Финальная сквозная сверка на накопленном множестве.
  ASSERT_FALSE(ref.empty());
  EXPECT_EQ(set_impl.Query(INT32_MIN, INT32_MAX), ref.size());
  EXPECT_EQ(vec_impl.Query(INT32_MIN, INT32_MAX), ref.size());
  for (size_t k = 1; k <= ref.size(); ++k) {
    const int expected = *std::next(ref.begin(), static_cast<long>(k - 1));
    EXPECT_EQ(set_impl.Min(k), expected) << "k = " << k;
    EXPECT_EQ(vec_impl.Min(k), expected) << "k = " << k;
    EXPECT_EQ(set_impl.LessKeysCounter(expected), k - 1) << "k = " << k;
    EXPECT_EQ(vec_impl.LessKeysCounter(expected), k - 1) << "k = " << k;
  }
}

TEST(DynamicOrderParity, BothImplsAgreeOnSameData) {
  // Однаковые данные, разный способ загрузки: set — вставками,
  // vector — конструктором + вставками вперемешку.
  const int vals[] = {4, -8, 15, 15, 16, 23, 42, -8, 99, 4};
  StdSetImpl set_impl;
  for (int v : vals) {
    set_impl.Insert(v);
  }

  SortedVectorImpl vec_impl{
      std::vector<int>{4, -8, 15, 15, 16, 23, 42, -8, 99, 4}};
  for (int v : vals) {
    vec_impl.Insert(v);  // повторные вставки не должны менять множество
  }

  for (int a = -100; a <= 100; a += 13) {
    for (int b = -100; b <= 100; b += 17) {
      EXPECT_EQ(set_impl.Query(a, b), vec_impl.Query(a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = -10; k <= 100; ++k) {
    EXPECT_EQ(set_impl.LessKeysCounter(k), vec_impl.LessKeysCounter(k))
        << "k = " << k;
  }
  for (size_t k = 1; k <= 6; ++k) {  // { -8, 4, 15, 16, 23, 42, 99 } — 7 ключей
    EXPECT_EQ(set_impl.Min(k), vec_impl.Min(k)) << "k = " << k;
  }
}

// ======================================================================
// AvlTreeImpl — дополненное AVL-дерево
//
// Ожидания по ТЗ (идентичны прочим реализациям):
//   Insert(key)            — множество, дубликаты игнорируются;
//   Query(left, right)     — количество ключей в (left, right];
//   Min(index)             — index-я порядковая статистика (с 1),
//                            ProgError(kOutOfRange) при index==0 или > размера;
//   LessKeysCounter(key)   — количество ключей строго меньше key.
//
// Тесты намеренно проверяют ЭТАЛОННУЮ семантику: там, где реализация
// ошибочна, тесты падают (или процесс завершается из-за assert/segfault),
// указывая конкретные баги.
// ======================================================================

using dynamic_order::AvlTreeImpl;

class AvlTreeImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Сбалансированный порядок вставки — rotations не вызываются:
    //        5
    //       / \
    //      3   7
    //     /     \
    //    1       9
    for (int key : {5, 3, 7, 1, 9}) {
      tree_.Insert(key);
    }
  }

  AvlTreeImpl tree_;
};

// ------------ Пустое дерево ------------

TEST(AvlTreeImplEmpty, QueryReturnsZero) {
  AvlTreeImpl t;
  EXPECT_EQ(t.Query(-5, 10), 0u);
  EXPECT_EQ(t.Query(0, 0), 0u);
}

TEST(AvlTreeImplEmpty, MinThrows) {
  AvlTreeImpl t;
  EXPECT_THROW(t.Min(0), ProgError);
  EXPECT_THROW(t.Min(1), ProgError);
}

TEST(AvlTreeImplEmpty, LessKeysCounterReturnsZero) {
  AvlTreeImpl t;
  EXPECT_EQ(t.LessKeysCounter(0), 0u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 0u);
}

TEST(AvlTreeImplErrors, ProgErrorCarriesContext) {
  AvlTreeImpl t;
  try {
    t.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
    EXPECT_EQ(e.GetErrorInfo().func_name, "dynamic_order::AvlTreeImpl::Min");
  } catch (const std::exception&) {
    FAIL() << "Ожидался именно ProgError";
  }
}

TEST(AvlTreeImplErrors, ProgIsStdException) {
  AvlTreeImpl t;
  EXPECT_THROW(t.Min(0), std::exception);
}

// ------------ Один элемент ------------

TEST(AvlTreeImplSingle, Semantics) {
  AvlTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.Min(1), 42);
  EXPECT_THROW(t.Min(2), ProgError);
  EXPECT_EQ(t.Query(41, 42), 1u);   // (41,42]={42}
  EXPECT_EQ(t.Query(42, 42), 0u);   // вырожденный пуст
  EXPECT_EQ(t.Query(42, 43), 0u);   // 42 — левый конец
}

TEST(AvlTreeImplSingle, LessKeysCounterCountsIt) {
  AvlTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.LessKeysCounter(42), 0u);  // строго меньше 42 — ничего
  EXPECT_EQ(t.LessKeysCounter(43), 1u);  // 42 < 43
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);
}

TEST(AvlTreeImplSingle, IntMinElement) {
  AvlTreeImpl t;
  t.Insert(INT32_MIN);
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Query(INT32_MIN, 0), 0u);  // INT32_MIN — левый конец, не считается
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(0), 1u);   // INT32_MIN < 0
}

TEST(AvlTreeImplSingle, IntMaxElement) {
  AvlTreeImpl t;
  t.Insert(INT32_MAX);
  EXPECT_EQ(t.Min(1), INT32_MAX);
  EXPECT_EQ(t.Query(INT32_MAX - 1, INT32_MAX), 1u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 0u);
}

TEST_F(AvlTreeImplTest, DuplicatesIgnored) {
  tree_.Insert(3);   // дубликат (поиск уйдёт влево от корня)
  tree_.Insert(7);   // дубликат (поиск уйдёт вправо от корня)
  tree_.Insert(5);   // дубликат корня
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_THROW(tree_.Min(6), ProgError);
  EXPECT_EQ(tree_.LessKeysCounter(1), 0u);
}

// ------------ Query на эталонном дереве ------------

TEST_F(AvlTreeImplTest, QueryCountsAllKeys) {
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_EQ(tree_.Query(0, 9), 5u);
  EXPECT_EQ(tree_.Query(INT32_MIN, INT32_MAX), 5u);
}

TEST_F(AvlTreeImplTest, QueryLeftBoundaryIsExcluded) {
  EXPECT_EQ(tree_.Query(5, 100), 2u);  // 7, 9
  EXPECT_EQ(tree_.Query(1, 3), 1u);    // 3
  EXPECT_EQ(tree_.Query(3, 5), 1u);    // 5
}

TEST_F(AvlTreeImplTest, QueryRightBoundaryIsIncluded) {
  EXPECT_EQ(tree_.Query(0, 5), 3u);    // 1, 3, 5
  EXPECT_EQ(tree_.Query(0, 1), 1u);    // 1
  EXPECT_EQ(tree_.Query(8, 9), 1u);    // 9
}

TEST_F(AvlTreeImplTest, QueryEmptyAndInvertedRanges) {
  EXPECT_EQ(tree_.Query(4, 4), 0u);
  EXPECT_EQ(tree_.Query(9, 9), 0u);
  EXPECT_EQ(tree_.Query(0, 0), 0u);
  EXPECT_EQ(tree_.Query(10, 20), 0u);
  EXPECT_EQ(tree_.Query(7, 3), 0u);
}

TEST_F(AvlTreeImplTest, QueryInternalRanges) {
  EXPECT_EQ(tree_.Query(2, 8), 3u);    // 3, 5, 7
  EXPECT_EQ(tree_.Query(4, 5), 1u);    // 5
  EXPECT_EQ(tree_.Query(5, 7), 1u);    // 7 (правый конец включается)
  EXPECT_EQ(tree_.Query(1, 9), 4u);    // 3, 5, 7, 9
}

TEST_F(AvlTreeImplTest, QueryWithNegativeBounds) {
  EXPECT_EQ(tree_.Query(-100, 5), 3u);     // 1, 3, 5
  EXPECT_EQ(tree_.Query(-1000, 1000), 5u); // все
  EXPECT_EQ(tree_.Query(-5, -1), 0u);      // ключей нет
}

// ------------ Min ------------

TEST_F(AvlTreeImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(tree_.Min(1), 1);
  EXPECT_EQ(tree_.Min(2), 3);
  EXPECT_EQ(tree_.Min(3), 5);
  EXPECT_EQ(tree_.Min(4), 7);
  EXPECT_EQ(tree_.Min(5), 9);
}

TEST_F(AvlTreeImplTest, MinThrowsOnBadIndex) {
  EXPECT_THROW(tree_.Min(0), ProgError);
  EXPECT_THROW(tree_.Min(6), ProgError);
  EXPECT_THROW(tree_.Min(100), ProgError);
}

// ------------ LessKeysCounter ------------

TEST_F(AvlTreeImplTest, LessKeysCounterForExistingKeys) {
  EXPECT_EQ(tree_.LessKeysCounter(1), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(3), 1u);   // {1}
  EXPECT_EQ(tree_.LessKeysCounter(5), 2u);   // {1, 3}
  EXPECT_EQ(tree_.LessKeysCounter(7), 3u);   // {1, 3, 5}
  EXPECT_EQ(tree_.LessKeysCounter(9), 4u);   // {1, 3, 5, 7}
}

TEST_F(AvlTreeImplTest, LessKeysCounterForAbsentKeys) {
  EXPECT_EQ(tree_.LessKeysCounter(0), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(2), 1u);    // {1}
  EXPECT_EQ(tree_.LessKeysCounter(8), 4u);    // {1,3,5,7}
  EXPECT_EQ(tree_.LessKeysCounter(10), 5u);   // все
  EXPECT_EQ(tree_.LessKeysCounter(INT32_MAX), 5u);
}

// ------------ Внутренняя согласованность ------------

TEST_F(AvlTreeImplTest, MinAndLessKeysCounterAreConsistent) {
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(tree_.LessKeysCounter(tree_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST_F(AvlTreeImplTest, QueryAndLessKeysCounterIdentity) {
  for (int a = 0; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(tree_.Query(a, b),
                tree_.LessKeysCounter(b + 1) - tree_.LessKeysCounter(a + 1))
          << "range (" << a << ", " << b << "]";
    }
  }
}

// ------------ Вставки поверх эталонного дерева ------------

TEST_F(AvlTreeImplTest, InsertPreservesSemantics) {
  tree_.Insert(6);  // лист под 7 — баланс-поворотов не требуется
  EXPECT_EQ(tree_.Query(0, 10), 6u);
  EXPECT_EQ(tree_.Query(5, 7), 2u);   // 6, 7
  EXPECT_EQ(tree_.Query(4, 6), 2u);   // 5, 6
  EXPECT_EQ(tree_.Query(6, 8), 1u);   // 7
}

// ------------ Сценарии, требующие балансировки ------------

TEST(AvlTreeImplRebalance, SortedAscending) {
  AvlTreeImpl t;
  t.Insert(1);
  t.Insert(2);
  t.Insert(3);
  t.Insert(4);
  t.Insert(5);
  EXPECT_EQ(t.Query(0, 6), 5u);
  EXPECT_EQ(t.Min(1), 1);
  EXPECT_EQ(t.Min(5), 5);
  EXPECT_EQ(t.LessKeysCounter(3), 2u);
}

TEST(AvlTreeImplRebalance, SortedDescending) {
  AvlTreeImpl t;
  for (int k : {5, 4, 3, 2, 1}) {
    t.Insert(k);
  }
  EXPECT_EQ(t.Query(0, 6), 5u);
  EXPECT_EQ(t.Min(1), 1);
  EXPECT_EQ(t.Min(5), 5);
  EXPECT_EQ(t.LessKeysCounter(4), 3u);
}

TEST(AvlTreeImplRebalance, ZigzagPattern) {
  AvlTreeImpl t;
  t.Insert(2);
  t.Insert(3);
  t.Insert(1);
  EXPECT_EQ(t.Query(0, 4), 3u);
  EXPECT_EQ(t.Min(1), 1);
  EXPECT_EQ(t.Min(2), 2);
  EXPECT_EQ(t.Min(3), 3);
}

// ------------ Стресс против std::set ------------

TEST(AvlTreeImplStress, MatchesStdSetReference) {
  std::mt19937 rng(20261016);
  std::uniform_int_distribution<int> key_dist(1, 1000);  // положительные,
  //   чтобы не смешивать баги балансировки с багом знаковых границ в Query
  AvlTreeImpl t;
  std::set<int> ref;

  for (int i = 0; i < 300; ++i) {
    const int k = key_dist(rng);
    ref.insert(k);
    t.Insert(k);
  }

  ASSERT_FALSE(ref.empty());
  EXPECT_EQ(t.Query(0, 1001), ref.size());
  EXPECT_EQ(t.Min(1), *ref.begin());
  EXPECT_EQ(t.Min(ref.size()), *ref.rbegin());
  for (int probe = 0; probe < 1000; probe += 37) {
    EXPECT_EQ(t.LessKeysCounter(probe), CountLess(ref, probe))
        << "probe = " << probe;
  }
}

// ------------ Паритет с эталонной реализацией на std::set ------------

TEST(AvlTreeImplParity, AgreesWithStdSetImpl) {
  AvlTreeImpl avl;
  StdSetImpl set_impl;
  for (int key : {5, 3, 7, 1, 9, 6, 4}) {
    avl.Insert(key);
    set_impl.Insert(key);
  }
  for (int a = 0; a <= 12; a += 3) {
    for (int b = 0; b <= 12; b += 5) {
      EXPECT_EQ(avl.Query(a, b), set_impl.Query(a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = 0; k <= 12; ++k) {
    EXPECT_EQ(avl.LessKeysCounter(k), set_impl.LessKeysCounter(k)) << "k=" << k;
  }
  const size_t size = 7;
  for (size_t k = 1; k <= size; ++k) {
    EXPECT_EQ(avl.Min(k), set_impl.Min(k)) << "k = " << k;
  }
}

// ======================================================================
// AvlTreeImpl — вставка одинаковых ключей
//
// Проверка новой функциональности: повторная вставка уже существующего
// ключа не должна влиять ни на структуру, ни на размер дерева.
// Размер и структура проверяются по публичному API: Min бросает
// ProgError при index > размера, порядковые статистики, диапазонные
// запросы Query и LessKeysCounter сверяются до и после серии
// дублирующих вставок (в т.ч. на дереве, где новые ключи вызывают
// балансировку).
// ======================================================================

TEST(AvlTreeImplDuplicates, ReinsertedKeyKeepsTreeSizeOne) {
  AvlTreeImpl t;
  t.Insert(42);

  for (int i = 0; i < 10; ++i) {
    t.Insert(42);
    // Размер остался равен 1 после каждой повторной вставки.
    EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u);
    EXPECT_EQ(t.Min(1), 42);
    EXPECT_THROW(t.Min(2), ProgError);
  }

  // Семантика ключей тоже не изменилась.
  EXPECT_EQ(t.Query(41, 42), 1u);   // правый конец включается
  EXPECT_EQ(t.Query(42, 43), 0u);   // левый конец исключается
  EXPECT_EQ(t.LessKeysCounter(42), 0u);
  EXPECT_EQ(t.LessKeysCounter(43), 1u);
}

TEST(AvlTreeImplDuplicates, OnlyIdenticalKeysBuildSingleElementTree) {
  // Набор из только одинаковых ключей: дерево обязано остаться одноэлементным.
  const int key = 7;
  AvlTreeImpl t;

  for (int i = 0; i < 100; ++i) {
    t.Insert(key);
    ASSERT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u) << "i = " << i;
    ASSERT_EQ(t.Min(1), key) << "i = " << i;
    ASSERT_THROW(t.Min(2), ProgError) << "i = " << i;  // второго элемента нет
  }

  EXPECT_EQ(t.Query(6, 7), 1u);
  EXPECT_EQ(t.Query(7, 7), 0u);
  EXPECT_EQ(t.Query(7, 8), 0u);
  EXPECT_EQ(t.LessKeysCounter(7), 0u);
  EXPECT_EQ(t.LessKeysCounter(8), 1u);
}

TEST(AvlTreeImplDuplicates, DuplicateInsertionsDoNotChangeTreeState) {
  AvlTreeImpl t;
  std::set<int> ref;
  for (int key : {5, 3, 7, 1, 9}) {
    t.Insert(key);
    ref.insert(key);
  }

  // Снимок observable-состояния до дублирующих вставок.
  std::vector<int> min_before;
  for (size_t k = 1; k <= 5; ++k) {
    min_before.push_back(t.Min(k));
  }

  // Пять полных проходов вставки уже существующих ключей:
  // корень, внутренние узлы с одним ребёнком и листья.
  for (int round = 0; round < 5; ++round) {
    for (int key : {5, 3, 7, 1, 9}) {
      t.Insert(key);
    }
  }

  // Размер не вырос: шестой порядковой статистики нет.
  EXPECT_THROW(t.Min(6), ProgError);

  // Порядковые статистики не изменились.
  std::vector<int> min_after;
  for (size_t k = 1; k <= 5; ++k) {
    min_after.push_back(t.Min(k));
  }
  EXPECT_EQ(min_before, min_after);

  // Все диапазонные запросы и счётчики меньших ключей — как у std::set.
  for (int a = 0; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(t.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = 0; k <= 12; ++k) {
    EXPECT_EQ(t.LessKeysCounter(k), CountLess(ref, k)) << "k = " << k;
  }
}

TEST(AvlTreeImplDuplicates, DuplicatesAlongsideRebalancingInsertions) {
  // Каждый ключ вставляется трижды: первая вставка может вызывать
  // повороты балансировки, последующие — должны игнорироваться.
  AvlTreeImpl t;
  std::set<int> ref;
  for (int k = 1; k <= 8; ++k) {
    for (int i = 0; i < 3; ++i) {
      t.Insert(k);
    }
    ref.insert(k);
    // После каждого блока из трёх вставок размер равен числу уникальных ключей.
    EXPECT_EQ(t.Query(0, 9), ref.size()) << "k = " << k;
  }

  EXPECT_THROW(t.Min(9), ProgError);  // размер = 8, не 24
  for (size_t k = 1; k <= 8; ++k) {
    EXPECT_EQ(t.Min(k), static_cast<int>(k)) << "k = " << k;
  }
  for (int key = 0; key <= 9; ++key) {
    EXPECT_EQ(t.LessKeysCounter(key), CountLess(ref, key)) << "key = " << key;
  }
}

TEST(AvlTreeImplDuplicates, NewKeyAfterDuplicatesCountedOnce) {
  AvlTreeImpl t;
  for (int key : {5, 3, 7, 1, 9}) {
    t.Insert(key);
  }
  for (int i = 0; i < 4; ++i) {
    t.Insert(6);  // ранее отсутствующий ключ: добавляется ровно один раз
  }

  EXPECT_EQ(t.Query(0, 10), 6u);
  EXPECT_THROW(t.Min(7), ProgError);
  EXPECT_EQ(t.Min(4), 6);  // {1, 3, 5, 6, 7, 9}
  EXPECT_EQ(t.LessKeysCounter(7), 4u);  // {1, 3, 5, 6}
}

// ======================================================================
// BTreeImpl — B-дерево с минимальной степенью t
//
// Эталонная семантика идентична прочим реализациям:
//   Insert(key)            — множество, дубликаты игнорируются;
//   Query(left, right)     — количество ключей в (left, right];
//   Min(index)             — index-я порядковая статистика (с 1),
//                            ProgError(kOutOfRange) при index==0 или > размера;
//   LessKeysCounter(key)   — количество ключей строго меньше key.
//
// Дополнительно покрываются низкоуровневые операции BTreeNode:
// InsertNonFull (вставка в неполный узел) и SplitChild (деление узла),
// а также поддержка размера поддерева subtree_sz_.
// ======================================================================

using dynamic_order::BTreeImpl;
using dynamic_order::BTreeNode;

class BTreeImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Пять ключей при t = 2 гарантированно вызывают сплиты:
    // корень-лист вмещает не более 2t - 1 = 3 ключа.
    for (int key : {5, 3, 7, 1, 9}) {
      tree_.Insert(key);
    }
  }

  BTreeImpl tree_;  // t = kMinDegree = 2
};

// ------------ Пустое дерево ------------

TEST(BTreeImplEmpty, QueryReturnsZero) {
  BTreeImpl t;
  EXPECT_EQ(t.Query(-5, 10), 0u);
  EXPECT_EQ(t.Query(0, 0), 0u);
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 0u);  // путь с right == INT32_MAX
}

TEST(BTreeImplEmpty, MinThrows) {
  BTreeImpl t;
  EXPECT_THROW(t.Min(0), ProgError);
  EXPECT_THROW(t.Min(1), ProgError);
}

TEST(BTreeImplEmpty, LessKeysCounterReturnsZero) {
  BTreeImpl t;
  EXPECT_EQ(t.LessKeysCounter(0), 0u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 0u);
}

TEST(BTreeImplEmpty, FirstInsertSucceeds) {
  BTreeImpl t;
  t.Insert(1);
  EXPECT_EQ(t.Query(0, 1), 1u);
  EXPECT_EQ(t.Min(1), 1);
}

// ------------ Ошибки ------------

TEST(BTreeImplErrors, ProgErrorCarriesContext) {
  BTreeImpl t;
  try {
    t.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
    EXPECT_EQ(e.GetErrorInfo().func_name, "dynamic_order::BTreeImpl::Min");
  } catch (const std::exception&) {
    FAIL() << "Ожидался именно ProgError";
  }
}

TEST(BTreeImplErrors, ProgIsStdException) {
  BTreeImpl t;
  EXPECT_THROW(t.Min(0), std::exception);
}

// ------------ Один элемент ------------

TEST(BTreeImplSingle, Semantics) {
  BTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.Min(1), 42);
  EXPECT_THROW(t.Min(2), ProgError);
  EXPECT_EQ(t.Query(41, 42), 1u);   // (41,42]={42}
  EXPECT_EQ(t.Query(42, 42), 0u);   // вырожденный диапазон пуст
  EXPECT_EQ(t.Query(42, 43), 0u);   // 42 — левый конец
}

TEST(BTreeImplSingle, LessKeysCounterCountsIt) {
  BTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.LessKeysCounter(42), 0u);  // строго меньше 42 — ничего
  EXPECT_EQ(t.LessKeysCounter(43), 1u);  // 42 < 43
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 1u);
}

TEST(BTreeImplSingle, IntMinElement) {
  BTreeImpl t;
  t.Insert(INT32_MIN);
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Query(INT32_MIN, 0), 0u);  // левый конец исключается
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(0), 1u);
}

TEST(BTreeImplSingle, IntMaxElement) {
  BTreeImpl t;
  t.Insert(INT32_MAX);
  EXPECT_EQ(t.Min(1), INT32_MAX);
  EXPECT_EQ(t.Query(INT32_MAX - 1, INT32_MAX), 1u);  // путь через subtree_sz_
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 0u);
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u);
}

TEST(BTreeImplSingle, ReinsertedKeyKeepsTreeSizeOne) {
  BTreeImpl t;
  t.Insert(42);

  for (int i = 0; i < 10; ++i) {
    t.Insert(42);
    EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u);
    EXPECT_EQ(t.Min(1), 42);
    EXPECT_THROW(t.Min(2), ProgError);
  }

  EXPECT_EQ(t.LessKeysCounter(42), 0u);
  EXPECT_EQ(t.LessKeysCounter(43), 1u);
}

// ------------ Дубликаты ------------

TEST(BTreeImplDuplicates, OnlyIdenticalKeysBuildSingleElementTree) {
  const int key = 7;
  BTreeImpl t;
  for (int i = 0; i < 100; ++i) {
    t.Insert(key);
    ASSERT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u) << "i = " << i;
    ASSERT_EQ(t.Min(1), key) << "i = " << i;
    ASSERT_THROW(t.Min(2), ProgError) << "i = " << i;
  }
}

TEST(BTreeImplDuplicates, DuplicateInsertionsDoNotChangeTreeState) {
  BTreeImpl t;
  std::set<int> ref;
  for (int key : {5, 3, 7, 1, 9, 6, 4}) {
    t.Insert(key);
    ref.insert(key);
  }

  std::vector<int> min_before;
  for (size_t k = 1; k <= ref.size(); ++k) {
    min_before.push_back(t.Min(k));
  }

  // Пять полных проходов повторных вставок: ключи на разных уровнях дерева.
  for (int round = 0; round < 5; ++round) {
    for (int key : {5, 3, 7, 1, 9, 6, 4}) {
      t.Insert(key);
    }
  }

  EXPECT_THROW(t.Min(8), ProgError);  // размер не вырос
  std::vector<int> min_after;
  for (size_t k = 1; k <= ref.size(); ++k) {
    min_after.push_back(t.Min(k));
  }
  EXPECT_EQ(min_before, min_after);

  for (int a = 0; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(t.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = 0; k <= 12; ++k) {
    EXPECT_EQ(t.LessKeysCounter(k), CountLess(ref, k)) << "k = " << k;
  }
}

TEST(BTreeImplDuplicates, DuplicatesAlongsideSplittingInsertions) {
  // Каждый ключ вставляется трижды; первые вставки вызывают сплиты.
  BTreeImpl t;
  std::set<int> ref;
  for (int k = 1; k <= 8; ++k) {
    for (int i = 0; i < 3; ++i) {
      t.Insert(k);
    }
    ref.insert(k);
    EXPECT_EQ(t.Query(0, 9), ref.size()) << "k = " << k;
  }

  EXPECT_THROW(t.Min(9), ProgError);  // размер = 8, не 24
  for (size_t k = 1; k <= 8; ++k) {
    EXPECT_EQ(t.Min(k), static_cast<int>(k)) << "k = " << k;
  }
  for (int key = 0; key <= 9; ++key) {
    EXPECT_EQ(t.LessKeysCounter(key), CountLess(ref, key)) << "key = " << key;
  }
}

// ------------ Query на фикстуре {1,3,5,7,9} ------------

TEST_F(BTreeImplTest, QueryCountsKeysInsideSegment) {
  EXPECT_EQ(tree_.Query(2, 8), 3u);  // 3, 5, 7
}

TEST_F(BTreeImplTest, QueryCountsAllKeys) {
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_EQ(tree_.Query(-1000, 1000), 5u);
  EXPECT_EQ(tree_.Query(INT32_MIN, INT32_MAX), 5u);
}

TEST_F(BTreeImplTest, QueryLeftBoundaryIsExcluded) {
  EXPECT_EQ(tree_.Query(5, 100), 2u);  // 7, 9 — ключ 5 НЕ считается
  EXPECT_EQ(tree_.Query(1, 3), 1u);    // только 3
  EXPECT_EQ(tree_.Query(3, 5), 1u);    // только 5
}

TEST_F(BTreeImplTest, QueryRightBoundaryIsIncluded) {
  EXPECT_EQ(tree_.Query(-100, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(tree_.Query(-100, 1), 1u);
  EXPECT_EQ(tree_.Query(8, 9), 1u);
}

TEST_F(BTreeImplTest, QueryDegenerateAndInvertedRanges) {
  EXPECT_EQ(tree_.Query(5, 5), 0u);
  EXPECT_EQ(tree_.Query(0, 0), 0u);
  EXPECT_EQ(tree_.Query(7, 3), 0u);
  EXPECT_EQ(tree_.Query(1, 0), 0u);
  EXPECT_EQ(tree_.Query(INT32_MAX, INT32_MIN), 0u);
}

TEST_F(BTreeImplTest, QueryWhenAllKeysOutsideRange) {
  EXPECT_EQ(tree_.Query(10, 20), 0u);
  EXPECT_EQ(tree_.Query(9, 100), 0u);
  EXPECT_EQ(tree_.Query(-20, 0), 0u);
  EXPECT_EQ(tree_.Query(-20, -10), 0u);
}

TEST_F(BTreeImplTest, QueryWorksWithNegativeKeys) {
  BTreeImpl t;
  for (int key : {-10, -5, 0, 5}) {
    t.Insert(key);
  }
  EXPECT_EQ(t.Query(-10, 5), 3u);   // -5, 0, 5
  EXPECT_EQ(t.Query(-6, -1), 1u);   // только -5
  EXPECT_EQ(t.Query(-10, -10), 0u);
  EXPECT_EQ(t.Query(-9, -5), 1u);
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 4u);
}

TEST_F(BTreeImplTest, QueryIsStableAcrossRepeatedCalls) {
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_EQ(tree_.Query(0, 10), 5u);
}

// ------------ Min на фикстуре ------------

TEST_F(BTreeImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(tree_.Min(1), 1);  // минимум
  EXPECT_EQ(tree_.Min(2), 3);
  EXPECT_EQ(tree_.Min(3), 5);
  EXPECT_EQ(tree_.Min(4), 7);
  EXPECT_EQ(tree_.Min(5), 9);  // максимум
}

TEST_F(BTreeImplTest, MinThrowsOnBadIndex) {
  EXPECT_THROW(tree_.Min(0), ProgError);
  EXPECT_THROW(tree_.Min(6), ProgError);
  EXPECT_THROW(tree_.Min(100), ProgError);
}

// ------------ LessKeysCounter на фикстуре ------------

TEST_F(BTreeImplTest, LessKeysCounterCountsStrictlyLess) {
  EXPECT_EQ(tree_.LessKeysCounter(1), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(2), 1u);   // {1}
  EXPECT_EQ(tree_.LessKeysCounter(5), 2u);   // {1, 3}
  EXPECT_EQ(tree_.LessKeysCounter(10), 5u);  // все
  EXPECT_EQ(tree_.LessKeysCounter(INT32_MAX), 5u);
}

TEST_F(BTreeImplTest, LessKeysCounterDoesNotCountEqualKey) {
  EXPECT_EQ(tree_.LessKeysCounter(3), 1u);
  EXPECT_EQ(tree_.LessKeysCounter(7), 3u);
}

TEST_F(BTreeImplTest, LessKeysCounterForVerySmallKey) {
  EXPECT_EQ(tree_.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(0), 0u);
}

TEST_F(BTreeImplTest, LessKeysCounterIsMonotonic) {
  for (int k = -5; k < 15; ++k) {
    EXPECT_GE(tree_.LessKeysCounter(k + 1), tree_.LessKeysCounter(k)) << "k=" << k;
  }
}

// ------------ Полная сверка с std::set (хелпер) ------------

// Сквозная проверка: Query на сетке, LessKeysCounter на прямой,
// все порядковые статистики, границы размера.
void VerifyEquivalence(const BTreeImpl& tree, const std::set<int>& ref,
                       int lo, int hi) {
  EXPECT_EQ(tree.Query(INT32_MIN, INT32_MAX), ref.size());
  for (int a = lo; a <= hi; ++a) {
    for (int b = lo; b <= hi; ++b) {
      EXPECT_EQ(tree.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  for (int k = lo; k <= hi; ++k) {
    EXPECT_EQ(tree.LessKeysCounter(k), CountLess(ref, k)) << "k = " << k;
  }
  for (size_t k = 1; k <= ref.size(); ++k) {
    EXPECT_EQ(tree.Min(k), *std::next(ref.begin(), static_cast<long>(k - 1)))
        << "k = " << k;
  }
  EXPECT_THROW(tree.Min(ref.size() + 1), ProgError);
}

// ------------ Сценарии роста со сплитами ------------

TEST(BTreeImplGrowth, SortedAscendingMatchesStdSet) {
  BTreeImpl t;  // t = 2: сплиты начинаются с 4-го ключа
  std::set<int> ref;
  for (int k = 1; k <= 20; ++k) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 21);
}

TEST(BTreeImplGrowth, SortedDescendingMatchesStdSet) {
  BTreeImpl t;
  std::set<int> ref;
  for (int k = 20; k >= 1; --k) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 21);
}

TEST(BTreeImplGrowth, ZigzagMatchesStdSet) {
  BTreeImpl t;
  std::set<int> ref;
  for (int k : {50, 30, 70, 20, 40, 60, 80, 10, 35, 45, 55, 65, 75, 5}) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 90);
}

TEST(BTreeImplGrowth, MixedSignKeysMatchesStdSet) {
  BTreeImpl t;
  std::set<int> ref;
  for (int k : {-1000, -1, 0, 1, 1000, -999, 999, -500, 500, INT32_MIN, INT32_MAX, 2}) {
    t.Insert(k);
    ref.insert(k);
  }
  // Глубокая сетка вокруг нуля + знаковые границы.
  for (int a = -1002; a <= 1002; a += 13) {
    for (int b = -1002; b <= 1002; b += 29) {
      EXPECT_EQ(t.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), ref.size() - 1);  // INT32_MIN — левый конец, исключается
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN + 2), 0u);  // в (min, min+2] ключей нет
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);      // вырожденный диапазон
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);       // меньше INT32_MIN ключей нет
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN + 1), 1u);   // только сам INT32_MIN
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Min(ref.size()), INT32_MAX);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), ref.size() - 1);
}

// ------------ Разные минимальные степени ------------

TEST(BTreeImplDegrees, DegreeThree) {
  BTreeImpl t(3);  // лист вмещает до 5 ключей
  std::set<int> ref;
  const int vals[] = {15, 3, 21, 7, 9, 1, 25, 11, 5, 17, 23, 13, 19, 2, 24};
  for (int k : vals) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 26);
}

TEST(BTreeImplDegrees, DegreeFourDescending) {
  BTreeImpl t(4);  // лист вмещает до 7 ключей
  std::set<int> ref;
  for (int k = 30; k >= 1; --k) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 31);
}

// ------------ Стресс против std::set ------------

TEST(BTreeImplStress, MatchesStdSetReference) {
  std::mt19937 rng(20261018);
  std::uniform_int_distribution<int> key_dist(1, 1000);

  BTreeImpl t;
  std::set<int> ref;
  for (int i = 0; i < 500; ++i) {
    const int k = key_dist(rng);
    ref.insert(k);
    t.Insert(k);
  }

  ASSERT_FALSE(ref.empty());
  EXPECT_EQ(t.Query(0, 1001), ref.size());
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), ref.size());

  const std::vector<int> sorted(ref.begin(), ref.end());
  for (size_t k = 1; k <= ref.size(); ++k) {
    EXPECT_EQ(t.Min(k), sorted[k - 1]) << "k = " << k;
  }
  EXPECT_THROW(t.Min(ref.size() + 1), ProgError);

  for (int probe = 0; probe <= 1000; probe += 13) {
    EXPECT_EQ(t.LessKeysCounter(probe), CountLess(ref, probe))
        << "probe = " << probe;
  }
}

// ------------ Паритет со всеми остальными реализациями ------------

TEST(BTreeImplParity, AgreesWithStdSetAndAvl) {
  BTreeImpl btree;
  StdSetImpl set_impl;
  AvlTreeImpl avl;
  std::set<int> ref;

  const int vals[] = {5, 3, 7, 1, 9, 6, 4, 3, 5, 42, -8, 42, 0, 100, -100};
  for (int k : vals) {
    btree.Insert(k);
    set_impl.Insert(k);
    avl.Insert(k);
    ref.insert(k);
  }

  for (int a = -110; a <= 110; a += 11) {
    for (int b = -110; b <= 110; b += 17) {
      const size_t expected = CountInRange(ref, a, b);
      EXPECT_EQ(btree.Query(a, b), expected) << "range (" << a << ", " << b << "]";
      EXPECT_EQ(set_impl.Query(a, b), expected);
      EXPECT_EQ(avl.Query(a, b), expected);
    }
  }
  for (int k = -110; k <= 110; ++k) {
    EXPECT_EQ(btree.LessKeysCounter(k), avl.LessKeysCounter(k)) << "k = " << k;
    EXPECT_EQ(btree.LessKeysCounter(k), set_impl.LessKeysCounter(k));
  }
  for (size_t k = 1; k <= ref.size(); ++k) {
    EXPECT_EQ(btree.Min(k), set_impl.Min(k)) << "k = " << k;
    EXPECT_EQ(btree.Min(k), avl.Min(k)) << "k = " << k;
  }
}

// ======================================================================
// BTreeNode — низкоуровневые операции напрямую
// ======================================================================

TEST(BTreeNodeInsertNonFull, LeafKeepsKeysSorted) {
  BTreeNode leaf(2);
  leaf.InsertNonFull(5);
  leaf.InsertNonFull(9);
  leaf.InsertNonFull(1);
  leaf.InsertNonFull(3);

  EXPECT_EQ(leaf.keys_, std::vector<int>({1, 3, 5, 9}));
  EXPECT_EQ(leaf.subtree_sz_, 4u);
  EXPECT_TRUE(leaf.leaf_);
}

TEST(BTreeNodeInsertNonFull, DescendsToCorrectChild) {
  BTreeNode root(2, /*leaf=*/false);
  auto* left = new BTreeNode(2);
  auto* mid = new BTreeNode(2);
  auto* right = new BTreeNode(2);
  left->keys_ = {1};
  mid->keys_ = {16};
  right->keys_ = {99};
  for (BTreeNode* child : {left, mid, right}) {
    child->subtree_sz_ = child->keys_.size();
  }
  root.keys_ = {10, 20};
  root.children_ = {left, mid, right};
  root.subtree_sz_ = root.keys_.size() + 3;

  root.InsertNonFull(5);   // в left: 5 < 10
  root.InsertNonFull(15);  // в mid: 10 <= 15 < 20
  root.InsertNonFull(25);  // в right: 25 >= 20

  EXPECT_EQ(left->keys_, std::vector<int>({1, 5}));
  EXPECT_EQ(mid->keys_, std::vector<int>({15, 16}));
  EXPECT_EQ(right->keys_, std::vector<int>({25, 99}));
  EXPECT_EQ(root.subtree_sz_, 8u);  // 2 ключа узла + 2+2+2 в детях
}

TEST(BTreeNodeInsertNonFull, SplitsFullChildOnDescent) {
  BTreeNode root(2, /*leaf=*/false);
  auto* full = new BTreeNode(2);
  auto* spare = new BTreeNode(2);
  full->keys_ = {1, 2, 3};
  spare->keys_ = {11};
  full->subtree_sz_ = 3;
  spare->subtree_sz_ = 1;
  root.keys_ = {10};
  root.children_ = {full, spare};
  root.subtree_sz_ = 5;

  root.InsertNonFull(4);
  // полный ребёнок {1,2,3} разделился: медиана 2 поднялась в корень,
  // а 4 ушла в правую половину {3}.

  EXPECT_EQ(root.keys_, std::vector<int>({2, 10}));
  ASSERT_EQ(root.children_.size(), 3u);
  EXPECT_EQ(root.children_[0]->keys_, std::vector<int>({1}));
  EXPECT_EQ(root.children_[1]->keys_, std::vector<int>({3, 4}));
  EXPECT_EQ(root.children_[2]->keys_, std::vector<int>({11}));
  EXPECT_EQ(root.subtree_sz_, 6u);
}

TEST(BTreeNodeSplitChild, SplitsFullLeaf) {
  BTreeNode parent(2, /*leaf=*/false);
  auto* child = new BTreeNode(2);
  child->keys_ = {1, 2, 3};
  child->subtree_sz_ = 3;
  parent.children_ = {child};

  parent.SplitChild(0, child);

  EXPECT_EQ(parent.keys_, std::vector<int>({2}));
  ASSERT_EQ(parent.children_.size(), 2u);
  EXPECT_EQ(child->keys_, std::vector<int>({1}));
  EXPECT_EQ(parent.children_[1]->keys_, std::vector<int>({3}));

  EXPECT_TRUE(parent.children_[1]->leaf_);  // признак листа наследуется
  EXPECT_EQ(child->subtree_sz_, 1u);
  EXPECT_EQ(parent.children_[1]->subtree_sz_, 1u);
  EXPECT_EQ(parent.subtree_sz_, 3u);
  // parent удалит обоих детей в деструкторе.
}

TEST(BTreeNodeSplitChild, SplitsInternalNodeMovingChildren) {
  auto* a = new BTreeNode(2);
  auto* b = new BTreeNode(2);
  auto* c = new BTreeNode(2);
  auto* d = new BTreeNode(2);
  a->keys_ = {1};
  b->keys_ = {15};
  c->keys_ = {25};
  d->keys_ = {35};
  for (BTreeNode* leaf : {a, b, c, d}) {
    leaf->subtree_sz_ = 1;
  }

  auto* child = new BTreeNode(2, /*leaf=*/false);
  child->keys_ = {10, 20, 30};
  child->children_ = {a, b, c, d};
  child->subtree_sz_ = 7;

  BTreeNode parent(2, /*leaf=*/false);
  parent.children_ = {child};

  parent.SplitChild(0, child);

  EXPECT_EQ(parent.keys_, std::vector<int>({20}));  // медиана поднята
  ASSERT_EQ(parent.children_.size(), 2u);
  EXPECT_EQ(child->keys_, std::vector<int>({10}));
  EXPECT_EQ(parent.children_[1]->keys_, std::vector<int>({30}));
  EXPECT_FALSE(parent.children_[1]->leaf_);  // leaf_ унаследован от child

  // дети переезжают целиком: [a, b] остались, [c, d] — в новой вершине
  ASSERT_EQ(child->children_.size(), 2u);
  EXPECT_EQ(child->children_[0], a);
  EXPECT_EQ(child->children_[1], b);
  const auto& right_children = parent.children_[1]->children_;
  ASSERT_EQ(right_children.size(), 2u);
  EXPECT_EQ(right_children[0], c);
  EXPECT_EQ(right_children[1], d);

  EXPECT_EQ(child->subtree_sz_, 3u);   // {10, a, b}
  EXPECT_EQ(parent.children_[1]->subtree_sz_, 3u);  // {30, c, d}
  EXPECT_EQ(parent.subtree_sz_, 7u);   // {20} + 3 + 3
  // parent владеет child и новой вершиной; те — своими внуками.
}

TEST(BTreeNodeMove, MoveConstructorTransfersOwnership) {
  auto* src = new BTreeNode(2, /*leaf=*/false);
  src->keys_ = {5};
  src->children_.push_back(new BTreeNode(2));
  src->children_[0]->keys_ = {1};
  src->children_[0]->subtree_sz_ = 1;
  src->subtree_sz_ = 2;

  BTreeNode moved(std::move(*src));

  EXPECT_EQ(moved.keys_, std::vector<int>({5}));
  EXPECT_EQ(moved.subtree_sz_, 2u);
  EXPECT_FALSE(moved.leaf_);
  EXPECT_TRUE(src->keys_.empty());
  EXPECT_TRUE(src->children_.empty());  // владение перешло, src не удалит детей
  delete src;
}

TEST(BTreeNodeMove, MoveAssignmentTransfersOwnership) {
  auto* src = new BTreeNode(2);
  src->keys_ = {7, 8};
  src->subtree_sz_ = 2;

  BTreeNode dst(2, /*leaf=*/false);
  dst.keys_ = {10};
  dst.children_.push_back(new BTreeNode(2));
  dst.children_[0]->keys_ = {1};
  dst.children_[0]->subtree_sz_ = 1;

  dst = std::move(*src);

  EXPECT_EQ(dst.keys_, std::vector<int>({7, 8}));
  EXPECT_EQ(dst.subtree_sz_, 2u);
  EXPECT_TRUE(src->keys_.empty());
  EXPECT_TRUE(src->children_.empty());
  delete src;  // dst уже не владеет старым ребёнком {1} — тот удалён присваиванием
}

// ------------ Внутренняя согласованность ------------

TEST_F(BTreeImplTest, MinAndLessKeysCounterAreConsistent) {
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(tree_.LessKeysCounter(tree_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST_F(BTreeImplTest, QueryAndLessKeysCounterIdentity) {
  // Query(a, b) == LessKeysCounter(b + 1) - LessKeysCounter(a + 1)
  for (int a = 0; a <= 12; ++a) {
    for (int b = a; b <= 12; ++b) {
      EXPECT_EQ(tree_.Query(a, b),
                tree_.LessKeysCounter(b + 1) - tree_.LessKeysCounter(a + 1))
          << "range (" << a << ", " << b << "]";
    }
  }
}
}  // namespace
