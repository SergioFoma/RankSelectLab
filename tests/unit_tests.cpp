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
#include <utility>
#include <vector>


import prog_error;
import std_set_impl;
import sorted_vector_impl;

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

}  // namespace
