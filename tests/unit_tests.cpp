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
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <new>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

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

//  Matcher-style check of ProgError context (analogue of
// EXPECT_THAT(action, Throws<ProgError>(ErrorWith(...)))).
//  gmock is not linked into this target, so the matcher is hand-rolled.
struct ErrorWith {
  ErrorCode code;
  std::string message;

  template <typename Action>
  void ExpectThrow(Action&& action) const {
    try {
      action();
    } catch (const ProgError& e) {
      EXPECT_EQ(e.GetErrorCode(), code);
      EXPECT_EQ(e.what(), message);
      return;
    } catch (const std::exception&) {
      ADD_FAILURE() << "expected ProgError, got a foreign std::exception";
      return;
    }
    ADD_FAILURE() << "expected ProgError was not thrown";
  }
};

//  Reference count of keys in (left, right].
size_t CountInRange(const std::set<int>& ref, int left, int right) {
  if (left >= right) {
    return 0;
  }
  return static_cast<size_t>(
      std::distance(std::upper_bound(ref.begin(), ref.end(), left),
                    std::upper_bound(ref.begin(), ref.end(), right)));
}

size_t CountLess(const std::set<int>& ref, int key) {
  return static_cast<size_t>(std::distance(
      ref.begin(), std::lower_bound(ref.begin(), ref.end(), key)));
}

// ======================================================================
// StdSetImpl
// ======================================================================

class StdSetImplTest : public ::testing::Test {
 protected:
  StdSetImplTest() : set_{std::set<int>{1, 3, 5, 7, 9}} {}

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
  EXPECT_EQ(s.Query(42, 43), 0u);  //  42 is the left border, not counted
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
  EXPECT_THROW(s.Min(2), ProgError);  //  size stayed 1
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
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 1u);  //  INT32_MAX only
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
  EXPECT_EQ(set_.Query(5, 100), 2u);  //  7, 9 -- key 5 is NOT counted
  EXPECT_EQ(set_.Query(1, 3), 1u);    //  3 only
  EXPECT_EQ(set_.Query(3, 5), 1u);    //  5 only
  EXPECT_EQ(set_.Query(9, 9), 0u);
}

TEST_F(StdSetImplTest, QueryRightBoundaryIsIncluded) {
  EXPECT_EQ(set_.Query(-100, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(set_.Query(-100, 1), 1u);  //  1 is the right border
  EXPECT_EQ(set_.Query(4, 5), 1u);     //  5 is the right border
}

TEST_F(StdSetImplTest, QueryDegenerateAndInvertedRanges) {
  EXPECT_EQ(set_.Query(5, 5), 0u);
  EXPECT_EQ(set_.Query(0, 0), 0u);
  EXPECT_EQ(set_.Query(7, 3), 0u);  //  empty range
  EXPECT_EQ(set_.Query(1, 0), 0u);
  EXPECT_EQ(set_.Query(INT32_MAX, INT32_MIN), 0u);
}

TEST_F(StdSetImplTest, QueryWhenAllKeysBelowLeft) {
  EXPECT_EQ(set_.Query(10, 20), 0u);
  EXPECT_EQ(set_.Query(9, 100), 0u);  // upper_bound(9) == end()
}

TEST_F(StdSetImplTest, QueryWhenAllKeysAboveRight) {
  EXPECT_EQ(set_.Query(-20, 0), 0u);  //  loop does not run: 1 > 0
  EXPECT_EQ(set_.Query(-20, -10), 0u);
}

TEST_F(StdSetImplTest, QueryWorksWithNegativeKeys) {
  StdSetImpl s;
  s.Insert(-10);
  s.Insert(-5);
  s.Insert(0);
  s.Insert(5);
  EXPECT_EQ(s.Query(-10, 5), 3u);  // -5, 0, 5
  EXPECT_EQ(s.Query(-6, -1), 1u);  //  -5 only
  EXPECT_EQ(s.Query(-10, -10), 0u);
  EXPECT_EQ(s.Query(-9, -5), 1u);  //  right border included
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 4u);
}

TEST(StdSetImplQuery, IsStableAcrossRepeatedCalls) {
  StdSetImpl s;
  s.Insert(1);
  s.Insert(2);
  EXPECT_EQ(s.Query(0, 2), 2u);
  EXPECT_EQ(s.Query(0, 2), 2u);  //  repeat query does not change state
}

// ------------ Min ------------

TEST_F(StdSetImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(set_.Min(1), 1);  //  minimum
  EXPECT_EQ(set_.Min(3), 5);
  EXPECT_EQ(set_.Min(5), 9);  //  maximum
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

  ErrorWith{ErrorCode::kOutOfRange, "Incorrect index!"}.ExpectThrow(
      [&s] { s.Min(1); });
}

TEST(StdSetImplMinErrors, ProgIsStdException) {
  StdSetImpl s;
  EXPECT_THROW(s.Min(0),
               std::exception);  //  ProgError is a std::exception descendant
}

// ------------ LessKeysCounter ------------

TEST_F(StdSetImplTest, LessKeysCounterCountsStrictlyLess) {
  EXPECT_EQ(set_.LessKeysCounter(1), 0u);   //  nothing less than 1
  EXPECT_EQ(set_.LessKeysCounter(2), 1u);   // {1}
  EXPECT_EQ(set_.LessKeysCounter(5), 2u);   // {1, 3}
  EXPECT_EQ(set_.LessKeysCounter(10), 5u);  //  all (full pass without break)
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
    EXPECT_GE(set_.LessKeysCounter(k + 1), set_.LessKeysCounter(k))
        << "k=" << k;
  }
}

//  ------------ StdSetImpl internal consistency ------------

TEST_F(StdSetImplTest, MinAndLessKeysCounterAreConsistent) {
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(set_.LessKeysCounter(set_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST_F(StdSetImplTest, QueryAndLessKeysCounterIdentity) {
  //  For integer keys: Query(a, b) == LessKeysCounter(b + 1) - LessKeysCounter(a + 1)
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
  //  {9, 3, 1, 7, 5} unsorted -- constructor must sort them
  SortedVectorImpl set_{std::vector<int>{9, 3, 1, 7, 5}};
};

//  ------------ Constructors ------------

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
  EXPECT_THROW(s.Min(4), ProgError);  //  size = 3, not 6
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
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MIN + 1),
            0u);  //  no keys between the borders
}

TEST(SortedVectorImplCtor, NegativeKeys) {
  SortedVectorImpl s{std::vector<int>{5, -10, 0, -5}};
  EXPECT_EQ(s.Min(1), -10);
  EXPECT_EQ(s.Min(4), 5);
  EXPECT_EQ(s.Query(-10, 5), 3u);  // -5, 0, 5
  EXPECT_EQ(s.Query(-6, -1), 1u);  //  -5 only
  EXPECT_EQ(s.Query(-10, -10), 0u);
  EXPECT_EQ(s.Query(-9, -5), 1u);
}

//  ------------ Insert + lazy merge ------------

TEST_F(SortedVectorImplTest, QuerySeesPendingInserts) {
  set_.Insert(6);
  EXPECT_EQ(set_.Query(5, 7), 2u);  //  6 and 7 (right border included)
  EXPECT_EQ(set_.Query(2, 8), 4u);  // 3, 5, 6, 7
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
  EXPECT_EQ(set_.Query(1, 2), 1u);  //  inserted 2 as the right border
  EXPECT_EQ(set_.Query(2, 3), 1u);  //  2 is the left border, only 3 is counted
  EXPECT_EQ(set_.Query(0, 2), 2u);  //  1 and 2
}

TEST_F(SortedVectorImplTest, MultiplePendingInsertsOfSameKeyMergeOnce) {
  set_.Insert(100);
  set_.Insert(100);
  set_.Insert(100);
  EXPECT_EQ(set_.Query(0, 1000), 6u);  //  5 existing + one new 100
  EXPECT_THROW(set_.Min(7), ProgError);
}

TEST_F(SortedVectorImplTest, DuplicatesAgainstExistingKeysRemovedOnMerge) {
  set_.Insert(5);                    //  5 already present
  set_.Insert(3);                    //  3 already present
  set_.Insert(5);                    //  once more
  EXPECT_EQ(set_.Query(0, 10), 5u);  //  the set did not bloat
  EXPECT_EQ(set_.Query(4, 5), 1u);   //  a single copy of 5
  EXPECT_THROW(set_.Min(6), ProgError);
  EXPECT_EQ(set_.LessKeysCounter(6), 3u);  // {1, 3, 5}
}

TEST_F(SortedVectorImplTest, MergesRepeatAcrossBatches) {
  set_.Insert(4);
  EXPECT_EQ(set_.Query(3, 5), 2u);  //  4 and 5
  set_.Insert(8);
  EXPECT_EQ(set_.Query(7, 9), 2u);  //  8 and 9
  EXPECT_EQ(set_.Query(3, 9), 5u);  // 4, 5, 7, 8, 9
  set_.Insert(2);
  set_.Insert(6);
  EXPECT_EQ(set_.Query(0, 10), 9u);  //  all of {1..9}
}

TEST_F(SortedVectorImplTest, InsertAfterMergeIsVisible) {
  EXPECT_EQ(set_.Query(0, 10), 5u);  // first merge
  set_.Insert(6);
  set_.Insert(4);
  EXPECT_EQ(set_.Query(4, 6), 2u);  //  5 and 6
  EXPECT_EQ(set_.Query(3, 5), 2u);  //  4 and 5
  EXPECT_EQ(set_.Query(0, 10), 7u);
}

TEST(SortedVectorImplInsert, IntoDefaultConstructedEmpty) {
  SortedVectorImpl s;
  s.Insert(5);
  s.Insert(1);
  s.Insert(3);
  EXPECT_EQ(s.Query(0, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(s.Query(5, 10), 0u);
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(3), 5);
  EXPECT_EQ(s.LessKeysCounter(3), 1u);
}

TEST(SortedVectorImplInsert, LargeBatch) {
  SortedVectorImpl s;
  std::set<int> ref;
  for (int i = 0; i < 1000; ++i) {
    int k = (i * 37) % 500 - 250;  //  deterministic pseudo-shuffled stream
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
  EXPECT_EQ(set_.Query(9, 100), 0u);  // lower_bound == end()
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
    EXPECT_EQ(e.GetErrorCode(), ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
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

TEST(SortedVectorImplLessKeysCounter,
     DefaultConstructedWithPendingInsertsOnly) {
  SortedVectorImpl s;
  s.Insert(7);
  s.Insert(3);
  EXPECT_EQ(s.LessKeysCounter(3), 0u);
  EXPECT_EQ(s.LessKeysCounter(4), 1u);
  EXPECT_EQ(s.LessKeysCounter(8), 2u);
}

TEST_F(SortedVectorImplTest, LessKeysCounterIsMonotonic) {
  for (int k = -5; k < 15; ++k) {
    EXPECT_GE(set_.LessKeysCounter(k + 1), set_.LessKeysCounter(k))
        << "k=" << k;
  }
}

//  ------------ SortedVectorImpl internal consistency ------------

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
//  Differential test: both impls vs std::set and vs each other
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

    if (op == 0 || op == 1) {  //  insert (including duplicates)
      const int k = key_dist(rng);
      ref.insert(k);
      set_impl.Insert(k);
      vec_impl.Insert(k);
    } else if (op == 2) {  //  range query
      const int a = key_dist(rng);
      const int b = key_dist(rng);
      const size_t expected = CountInRange(ref, a, b);
      EXPECT_EQ(set_impl.Query(a, b), expected);
      EXPECT_EQ(vec_impl.Query(a, b), expected);
    } else if (op == 3) {  //  order statistic
      if (ref.empty()) {
        EXPECT_THROW(set_impl.Min(1), ProgError);
        EXPECT_THROW(vec_impl.Min(1), ProgError);
      } else {
        const size_t k = 1 + (rng() % ref.size());
        const int expected = *std::next(ref.begin(), static_cast<long>(k - 1));
        EXPECT_EQ(set_impl.Min(k), expected);
        EXPECT_EQ(vec_impl.Min(k), expected);
        //  also out-of-range check
        EXPECT_THROW(set_impl.Min(ref.size() + 1), ProgError);
        EXPECT_THROW(vec_impl.Min(ref.size() + 1), ProgError);
      }
    } else {  //  count of smaller keys
      const int k = key_dist(rng);
      const size_t expected = CountLess(ref, k);
      EXPECT_EQ(set_impl.LessKeysCounter(k), expected);
      EXPECT_EQ(vec_impl.LessKeysCounter(k), expected);
    }
  }

  //  Final end-to-end reconciliation on the accumulated set.
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
  //  Same data, different loading: set -- inserts,
  //  vector -- constructor + interleaved inserts.
  const int vals[] = {4, -8, 15, 15, 16, 23, 42, -8, 99, 4};
  StdSetImpl set_impl;
  for (int v : vals) {
    set_impl.Insert(v);
  }

  SortedVectorImpl vec_impl{
      std::vector<int>{4, -8, 15, 15, 16, 23, 42, -8, 99, 4}};
  for (int v : vals) {
    vec_impl.Insert(v);  //  repeated inserts must not change the set
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
  for (size_t k = 1; k <= 6; ++k) {  //  { -8, 4, 15, 16, 23, 42, 99 } -- 7 keys
    EXPECT_EQ(set_impl.Min(k), vec_impl.Min(k)) << "k = " << k;
  }
}

// ======================================================================
//  AvlTreeImpl -- augmented AVL tree
//
//  Expected semantics per spec (identical to other impls):
//    Insert(key)            -- set semantics, duplicates ignored;
//    Query(left, right)     -- number of keys in (left, right];
//    Min(index)             -- index-th order statistic (1-based),
//                             ProgError(kOutOfRange) when index==0 or > size;
//    LessKeysCounter(key)   -- number of keys strictly less than key.
//
//  Tests deliberately check the REFERENCE semantics: where the impl
//  is wrong, tests fail (or the process dies via assert/segfault),
//  pointing at concrete bugs.
// ======================================================================

using dynamic_order::AvlTreeImpl;

class AvlTreeImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    //  Balanced insertion order -- no rotations are triggered:
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

//  ------------ Empty tree ------------

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
    EXPECT_EQ(e.GetErrorCode(), ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
  } catch (const std::exception&) {
    FAIL() << "Ожидался именно ProgError";
  }
}

TEST(AvlTreeImplErrors, ProgIsStdException) {
  AvlTreeImpl t;
  EXPECT_THROW(t.Min(0), std::exception);
}

//  ------------ Single element ------------

TEST(AvlTreeImplSingle, Semantics) {
  AvlTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.Min(1), 42);
  EXPECT_THROW(t.Min(2), ProgError);
  EXPECT_EQ(t.Query(41, 42), 1u);  // (41,42]={42}
  EXPECT_EQ(t.Query(42, 42), 0u);  //  degenerate empty
  EXPECT_EQ(t.Query(42, 43), 0u);  //  42 is the left border
}

TEST(AvlTreeImplSingle, LessKeysCounterCountsIt) {
  AvlTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.LessKeysCounter(42), 0u);  //  strictly less than 42 -- nothing
  EXPECT_EQ(t.LessKeysCounter(43), 1u);  // 42 < 43
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);
}

TEST(AvlTreeImplSingle, IntMinElement) {
  AvlTreeImpl t;
  t.Insert(INT32_MIN);
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Query(INT32_MIN, 0),
            0u);  //  INT32_MIN is the left border, not counted
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(0), 1u);  // INT32_MIN < 0
}

TEST(AvlTreeImplSingle, IntMaxElement) {
  AvlTreeImpl t;
  t.Insert(INT32_MAX);
  EXPECT_EQ(t.Min(1), INT32_MAX);
  EXPECT_EQ(t.Query(INT32_MAX - 1, INT32_MAX), 1u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 0u);
}

TEST_F(AvlTreeImplTest, DuplicatesIgnored) {
  tree_.Insert(3);  //  duplicate (search goes left of root)
  tree_.Insert(7);  //  duplicate (search goes right of root)
  tree_.Insert(5);  //  root duplicate
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_THROW(tree_.Min(6), ProgError);
  EXPECT_EQ(tree_.LessKeysCounter(1), 0u);
}

//  ------------ Query on the reference tree ------------

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
  EXPECT_EQ(tree_.Query(0, 5), 3u);  // 1, 3, 5
  EXPECT_EQ(tree_.Query(0, 1), 1u);  // 1
  EXPECT_EQ(tree_.Query(8, 9), 1u);  // 9
}

TEST_F(AvlTreeImplTest, QueryEmptyAndInvertedRanges) {
  EXPECT_EQ(tree_.Query(4, 4), 0u);
  EXPECT_EQ(tree_.Query(9, 9), 0u);
  EXPECT_EQ(tree_.Query(0, 0), 0u);
  EXPECT_EQ(tree_.Query(10, 20), 0u);
  EXPECT_EQ(tree_.Query(7, 3), 0u);
}

TEST_F(AvlTreeImplTest, QueryInternalRanges) {
  EXPECT_EQ(tree_.Query(2, 8), 3u);  // 3, 5, 7
  EXPECT_EQ(tree_.Query(4, 5), 1u);  // 5
  EXPECT_EQ(tree_.Query(5, 7), 1u);  //  7 (right border included)
  EXPECT_EQ(tree_.Query(1, 9), 4u);  // 3, 5, 7, 9
}

TEST_F(AvlTreeImplTest, QueryWithNegativeBounds) {
  EXPECT_EQ(tree_.Query(-100, 5), 3u);      // 1, 3, 5
  EXPECT_EQ(tree_.Query(-1000, 1000), 5u);  //  all
  EXPECT_EQ(tree_.Query(-5, -1), 0u);       //  no keys
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
  EXPECT_EQ(tree_.LessKeysCounter(3), 1u);  // {1}
  EXPECT_EQ(tree_.LessKeysCounter(5), 2u);  // {1, 3}
  EXPECT_EQ(tree_.LessKeysCounter(7), 3u);  // {1, 3, 5}
  EXPECT_EQ(tree_.LessKeysCounter(9), 4u);  // {1, 3, 5, 7}
}

TEST_F(AvlTreeImplTest, LessKeysCounterForAbsentKeys) {
  EXPECT_EQ(tree_.LessKeysCounter(0), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(2), 1u);   // {1}
  EXPECT_EQ(tree_.LessKeysCounter(8), 4u);   // {1,3,5,7}
  EXPECT_EQ(tree_.LessKeysCounter(10), 5u);  //  all
  EXPECT_EQ(tree_.LessKeysCounter(INT32_MAX), 5u);
}

//  ------------ Internal consistency ------------

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

//  ------------ Inserts on top of the reference tree ------------

TEST_F(AvlTreeImplTest, InsertPreservesSemantics) {
  tree_.Insert(6);  //  leaf under 7 -- no rebalancing needed
  EXPECT_EQ(tree_.Query(0, 10), 6u);
  EXPECT_EQ(tree_.Query(5, 7), 2u);  // 6, 7
  EXPECT_EQ(tree_.Query(4, 6), 2u);  // 5, 6
  EXPECT_EQ(tree_.Query(6, 8), 1u);  // 7
}

//  ------------ Balancing scenarios ------------

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

//  ------------ Stress vs std::set ------------

TEST(AvlTreeImplStress, MatchesStdSetReference) {
  std::mt19937 rng(20261016);
  std::uniform_int_distribution<int> key_dist(1, 1000);  //  positive keys,
  //    to keep rebalancing bugs apart from sign-boundary bugs in Query
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

//  ------------ Parity with the std::set reference implementation ------------

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
//  AvlTreeImpl -- inserting duplicate keys
//
//  Checks the new feature: re-inserting an existing
//  key must affect neither the structure nor the tree size.
//  Size and structure are checked via the public API: Min throws
//  ProgError when index > size; order statistics, range
//  Query and LessKeysCounter results match before and after a series
//  of duplicate inserts (incl. on a tree where new keys trigger
//  rebalancing).
// ======================================================================

TEST(AvlTreeImplDuplicates, ReinsertedKeyKeepsTreeSizeOne) {
  AvlTreeImpl t;
  t.Insert(42);

  for (int i = 0; i < 10; ++i) {
    t.Insert(42);
    //  Size stayed 1 after every repeated insert.
    EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u);
    EXPECT_EQ(t.Min(1), 42);
    EXPECT_THROW(t.Min(2), ProgError);
  }

  //  Key semantics unchanged as well.
  EXPECT_EQ(t.Query(41, 42), 1u);  //  right border included
  EXPECT_EQ(t.Query(42, 43), 0u);  //  left border excluded
  EXPECT_EQ(t.LessKeysCounter(42), 0u);
  EXPECT_EQ(t.LessKeysCounter(43), 1u);
}

TEST(AvlTreeImplDuplicates, OnlyIdenticalKeysBuildSingleElementTree) {
  //  All-identical keys: the tree must remain single-element.
  const int key = 7;
  AvlTreeImpl t;

  for (int i = 0; i < 100; ++i) {
    t.Insert(key);
    ASSERT_EQ(t.Query(INT32_MIN, INT32_MAX), 1u) << "i = " << i;
    ASSERT_EQ(t.Min(1), key) << "i = " << i;
    ASSERT_THROW(t.Min(2), ProgError)
        << "i = " << i;  //  there is no second element
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

  //  Snapshot of the observable state before duplicate inserts.
  std::vector<int> min_before;
  for (size_t k = 1; k <= 5; ++k) {
    min_before.push_back(t.Min(k));
  }

  //  Five full passes inserting already-present keys:
  //  root, single-child internal nodes and leaves.
  for (int round = 0; round < 5; ++round) {
    for (int key : {5, 3, 7, 1, 9}) {
      t.Insert(key);
    }
  }

  //  Size did not grow: there is no sixth order statistic.
  EXPECT_THROW(t.Min(6), ProgError);

  //  Order statistics are unchanged.
  std::vector<int> min_after;
  for (size_t k = 1; k <= 5; ++k) {
    min_after.push_back(t.Min(k));
  }
  EXPECT_EQ(min_before, min_after);

  //  All range queries and less-counters match std::set.
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
  //  Each key is inserted three times: the first insert may trigger
  //  rebalancing rotations, the rest must be ignored.
  AvlTreeImpl t;
  std::set<int> ref;
  for (int k = 1; k <= 8; ++k) {
    for (int i = 0; i < 3; ++i) {
      t.Insert(k);
    }
    ref.insert(k);
    //  After each triple-insert block the size equals the unique key count.
    EXPECT_EQ(t.Query(0, 9), ref.size()) << "k = " << k;
  }

  EXPECT_THROW(t.Min(9), ProgError);  //  size = 8, not 24
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
    t.Insert(6);  //  previously absent key: added exactly once
  }

  EXPECT_EQ(t.Query(0, 10), 6u);
  EXPECT_THROW(t.Min(7), ProgError);
  EXPECT_EQ(t.Min(4), 6);               // {1, 3, 5, 6, 7, 9}
  EXPECT_EQ(t.LessKeysCounter(7), 4u);  // {1, 3, 5, 6}
}

// ======================================================================
//  BTreeImpl -- B-tree with minimum degree t
//
//  Reference semantics identical to other impls:
//    Insert(key)            -- set semantics, duplicates ignored;
//    Query(left, right)     -- number of keys in (left, right];
//    Min(index)             -- index-th order statistic (1-based),
//                             ProgError(kOutOfRange) when index==0 or > size;
//    LessKeysCounter(key)   -- number of keys strictly less than key.
// ======================================================================

using dynamic_order::BTreeImpl;

class BTreeImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    //  Five keys with t = 2 are guaranteed to cause splits:
    //  a leaf root holds at most 2t - 1 = 3 keys.
    for (int key : {5, 3, 7, 1, 9}) {
      tree_.Insert(key);
    }
  }

  BTreeImpl tree_;  // t = kMinDegree = 2
};

//  ------------ Empty tree ------------

TEST(BTreeImplEmpty, QueryReturnsZero) {
  BTreeImpl t;
  EXPECT_EQ(t.Query(-5, 10), 0u);
  EXPECT_EQ(t.Query(0, 0), 0u);
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX),
            0u);  //  path with right == INT32_MAX
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

//  ------------ Errors ------------

TEST(BTreeImplErrors, ProgErrorCarriesContext) {
  BTreeImpl t;
  try {
    t.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorCode(), ErrorCode::kOutOfRange);
    EXPECT_STREQ(e.what(), "Incorrect index!");
  } catch (const std::exception&) {
    FAIL() << "Ожидался именно ProgError";
  }
}

TEST(BTreeImplErrors, ProgIsStdException) {
  BTreeImpl t;
  EXPECT_THROW(t.Min(0), std::exception);
}

//  ------------ Single element ------------

TEST(BTreeImplSingle, Semantics) {
  BTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.Min(1), 42);
  EXPECT_THROW(t.Min(2), ProgError);
  EXPECT_EQ(t.Query(41, 42), 1u);  // (41,42]={42}
  EXPECT_EQ(t.Query(42, 42), 0u);  //  degenerate range is empty
  EXPECT_EQ(t.Query(42, 43), 0u);  //  42 is the left border
}

TEST(BTreeImplSingle, LessKeysCounterCountsIt) {
  BTreeImpl t;
  t.Insert(42);
  EXPECT_EQ(t.LessKeysCounter(42), 0u);  //  strictly less than 42 -- nothing
  EXPECT_EQ(t.LessKeysCounter(43), 1u);  // 42 < 43
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), 1u);
}

TEST(BTreeImplSingle, IntMinElement) {
  BTreeImpl t;
  t.Insert(INT32_MIN);
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Query(INT32_MIN, 0), 0u);  //  left border excluded
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);
  EXPECT_EQ(t.LessKeysCounter(0), 1u);
}

TEST(BTreeImplSingle, IntMaxElement) {
  BTreeImpl t;
  t.Insert(INT32_MAX);
  EXPECT_EQ(t.Min(1), INT32_MAX);
  EXPECT_EQ(t.Query(INT32_MAX - 1, INT32_MAX), 1u);  //  path via subtree_sz_
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

//  ------------ Duplicates ------------

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

  //  Five full passes of repeated inserts: keys at different tree levels.
  for (int round = 0; round < 5; ++round) {
    for (int key : {5, 3, 7, 1, 9, 6, 4}) {
      t.Insert(key);
    }
  }

  EXPECT_THROW(t.Min(8), ProgError);  //  size did not grow
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
  //  Each key is inserted three times; first inserts trigger splits.
  BTreeImpl t;
  std::set<int> ref;
  for (int k = 1; k <= 8; ++k) {
    for (int i = 0; i < 3; ++i) {
      t.Insert(k);
    }
    ref.insert(k);
    EXPECT_EQ(t.Query(0, 9), ref.size()) << "k = " << k;
  }

  EXPECT_THROW(t.Min(9), ProgError);  //  size = 8, not 24
  for (size_t k = 1; k <= 8; ++k) {
    EXPECT_EQ(t.Min(k), static_cast<int>(k)) << "k = " << k;
  }
  for (int key = 0; key <= 9; ++key) {
    EXPECT_EQ(t.LessKeysCounter(key), CountLess(ref, key)) << "key = " << key;
  }
}

//  ------------ Query on fixture {1,3,5,7,9} ------------

TEST_F(BTreeImplTest, QueryCountsKeysInsideSegment) {
  EXPECT_EQ(tree_.Query(2, 8), 3u);  // 3, 5, 7
}

TEST_F(BTreeImplTest, QueryCountsAllKeys) {
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_EQ(tree_.Query(-1000, 1000), 5u);
  EXPECT_EQ(tree_.Query(INT32_MIN, INT32_MAX), 5u);
}

TEST_F(BTreeImplTest, QueryLeftBoundaryIsExcluded) {
  EXPECT_EQ(tree_.Query(5, 100), 2u);  //  7, 9 -- key 5 is NOT counted
  EXPECT_EQ(tree_.Query(1, 3), 1u);    //  3 only
  EXPECT_EQ(tree_.Query(3, 5), 1u);    //  5 only
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
  EXPECT_EQ(t.Query(-10, 5), 3u);  // -5, 0, 5
  EXPECT_EQ(t.Query(-6, -1), 1u);  //  -5 only
  EXPECT_EQ(t.Query(-10, -10), 0u);
  EXPECT_EQ(t.Query(-9, -5), 1u);
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX), 4u);
}

TEST_F(BTreeImplTest, QueryIsStableAcrossRepeatedCalls) {
  EXPECT_EQ(tree_.Query(0, 10), 5u);
  EXPECT_EQ(tree_.Query(0, 10), 5u);
}

//  ------------ Min on fixture ------------

TEST_F(BTreeImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(tree_.Min(1), 1);  //  minimum
  EXPECT_EQ(tree_.Min(2), 3);
  EXPECT_EQ(tree_.Min(3), 5);
  EXPECT_EQ(tree_.Min(4), 7);
  EXPECT_EQ(tree_.Min(5), 9);  //  maximum
}

TEST_F(BTreeImplTest, MinThrowsOnBadIndex) {
  EXPECT_THROW(tree_.Min(0), ProgError);
  EXPECT_THROW(tree_.Min(6), ProgError);
  EXPECT_THROW(tree_.Min(100), ProgError);
}

//  ------------ LessKeysCounter on fixture ------------

TEST_F(BTreeImplTest, LessKeysCounterCountsStrictlyLess) {
  EXPECT_EQ(tree_.LessKeysCounter(1), 0u);
  EXPECT_EQ(tree_.LessKeysCounter(2), 1u);   // {1}
  EXPECT_EQ(tree_.LessKeysCounter(5), 2u);   // {1, 3}
  EXPECT_EQ(tree_.LessKeysCounter(10), 5u);  //  all
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
    EXPECT_GE(tree_.LessKeysCounter(k + 1), tree_.LessKeysCounter(k))
        << "k=" << k;
  }
}

//  ------------ Full reconciliation with std::set (helper) ------------

//  End-to-end check: Query on a grid, LessKeysCounter on a line,
//  all order statistics, size boundaries.
void VerifyEquivalence(const BTreeImpl& tree, const std::set<int>& ref, int lo,
                       int hi) {
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

//  ------------ Growth scenarios with splits ------------

TEST(BTreeImplGrowth, SortedAscendingMatchesStdSet) {
  BTreeImpl t;  //  t = 2: splits start at the 4th key
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
  for (int k :
       {-1000, -1, 0, 1, 1000, -999, 999, -500, 500, INT32_MIN, INT32_MAX, 2}) {
    t.Insert(k);
    ref.insert(k);
  }
  //  Dense grid around zero + sign boundaries.
  for (int a = -1002; a <= 1002; a += 13) {
    for (int b = -1002; b <= 1002; b += 29) {
      EXPECT_EQ(t.Query(a, b), CountInRange(ref, a, b))
          << "range (" << a << ", " << b << "]";
    }
  }
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MAX),
            ref.size() - 1);  //  INT32_MIN is the left border, excluded
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN + 2),
            0u);                                    //  no keys in (min, min+2]
  EXPECT_EQ(t.Query(INT32_MIN, INT32_MIN), 0u);     //  degenerate range
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN), 0u);      //  no keys below INT32_MIN
  EXPECT_EQ(t.LessKeysCounter(INT32_MIN + 1), 1u);  //  INT32_MIN itself only
  EXPECT_EQ(t.Min(1), INT32_MIN);
  EXPECT_EQ(t.Min(ref.size()), INT32_MAX);
  EXPECT_EQ(t.LessKeysCounter(INT32_MAX), ref.size() - 1);
}

//  ------------ Various minimum degrees ------------

TEST(BTreeImplDegrees, DegreeThree) {
  BTreeImpl t(3);  //  a leaf holds up to 5 keys
  std::set<int> ref;
  const int vals[] = {15, 3, 21, 7, 9, 1, 25, 11, 5, 17, 23, 13, 19, 2, 24};
  for (int k : vals) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 26);
}

TEST(BTreeImplDegrees, DegreeFourDescending) {
  BTreeImpl t(4);  //  a leaf holds up to 7 keys
  std::set<int> ref;
  for (int k = 30; k >= 1; --k) {
    t.Insert(k);
    ref.insert(k);
  }
  VerifyEquivalence(t, ref, 0, 31);
}

//  ------------ Stress vs std::set ------------

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

//  ------------ Parity with all other implementations ------------

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
      EXPECT_EQ(btree.Query(a, b), expected)
          << "range (" << a << ", " << b << "]";
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

//  ------------ Internal consistency ------------

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
// ======================================================================
// Strong exception guarantee: alloc-failpoint harness
// ======================================================================
//
//  The global operator new (defined below, at global scope) is armed
//  by a countdown: the N-th allocation throws std::bad_alloc.
//  A live-allocation counter guards against leaks
//  on failure paths. Arming windows are only ScopedAllocBudget around
//  the checked operation; all gtest macros stay outside armed zones.

std::atomic<std::ptrdiff_t> g_new_budget{-1};  //  -1 = disarmed
std::atomic<std::size_t> g_alloc_count{0};  //  total allocations since startup
std::atomic<std::size_t> g_live_allocs{0};  //_alloc - free

bool TakeAllocStep() {
  std::ptrdiff_t budget = g_new_budget.load(std::memory_order_relaxed);
  while (budget >= 0) {
    std::ptrdiff_t next = (budget == 0) ? -1 : budget - 1;
    if (g_new_budget.compare_exchange_weak(budget, next,
                                           std::memory_order_relaxed)) {
      return budget == 0;  //  this allocation must throw
    }
  }
  return false;
}

class ScopedAllocBudget {
 public:
  explicit ScopedAllocBudget(std::ptrdiff_t fail_at_nth) {
    g_new_budget.store(fail_at_nth, std::memory_order_relaxed);
  }
  ScopedAllocBudget(const ScopedAllocBudget&) = delete;
  ScopedAllocBudget& operator=(const ScopedAllocBudget&) = delete;
  ~ScopedAllocBudget() { g_new_budget.store(-1, std::memory_order_relaxed); }

 private:
};

template <typename Fn>
std::size_t MeasureOpAllocs(Fn&& op) {
  const std::size_t before = g_alloc_count.load(std::memory_order_relaxed);
  op();
  return g_alloc_count.load(std::memory_order_relaxed) - before;
}

// ---------------- helpers for AVL checks ----------------

using dynamic_order::AvlTreeImpl;

AvlTreeImpl BuildAvl(const std::vector<int>& keys) {
  AvlTreeImpl tree;
  for (int key : keys) {
    tree.Insert(key);
  }
  return tree;
}

//  The tree behaves exactly like ref: cardinality, borders, counters.
void CheckAvlEquals(const AvlTreeImpl& tree, const std::set<int>& ref, int lo,
                    int hi) {
  EXPECT_EQ(tree.Query(lo, hi), ref.size());
  EXPECT_EQ(tree.LessKeysCounter(lo), CountLess(ref, lo));
  for (int key : ref) {
    EXPECT_EQ(tree.LessKeysCounter(key), CountLess(ref, key)) << key;
    EXPECT_EQ(tree.LessKeysCounter(key + 1),
              std::distance(ref.begin(), ref.upper_bound(key)));
  }
  if (!ref.empty()) {
    EXPECT_EQ(tree.Min(1), *ref.begin());
    EXPECT_EQ(tree.Min(ref.size()), *ref.rbegin());
    EXPECT_THROW(tree.Min(ref.size() + 1), ProgError);
  }
}

//  Runs the op failing at each of m allocations (0-based):
//  * must throw std::bad_alloc (exactly it, not a slice);
//  * leave no live allocations behind (net leak = 0);
//  * state_check must pass after every attempt.

// ======================================================================
//  Insert: strong guarantee
// ======================================================================

TEST(AvlTreeStrongGuarantee, InsertFailsAtEveryAllocationTreeStaysIntact) {
  const std::vector<int> base = {5, 2, 8, 1, 4, 7, 9, 3};
  const int kNew = 6;
  const std::set<int> ref(base.begin(), base.end());

  //  How many allocations a successful insert costs (Node + nodes_ptr_ growth).
  AvlTreeImpl probe = BuildAvl(base);
  const std::size_t m = MeasureOpAllocs([&] { probe.Insert(kNew); });
  ASSERT_GT(m, std::size_t{0});

  for (std::size_t nth = 0; nth < m; ++nth) {
    SCOPED_TRACE("fail at alloc #" + std::to_string(nth));
    AvlTreeImpl tree = BuildAvl(base);
    const std::size_t live_before = g_live_allocs.load();

    bool caught_bad_alloc = false;
    bool caught_other = false;
    {
      ScopedAllocBudget budget(static_cast<std::ptrdiff_t>(nth));
      try {
        tree.Insert(kNew);
      } catch (const std::bad_alloc&) {
        caught_bad_alloc = true;
      } catch (const std::exception&) {
        caught_other = true;
      }
    }
    EXPECT_TRUE(caught_bad_alloc);
    EXPECT_FALSE(caught_other) << "bad_alloc was sliced to std::exception";
    EXPECT_EQ(g_live_allocs.load(), live_before) << "node leaked at #" << nth;

    //  The tree did not budge at all.
    CheckAvlEquals(tree, ref, -5, 15);

    //  And it is fully usable: the insert succeeds after the failure.
    tree.Insert(kNew);
    std::set<int> grown = ref;
    grown.insert(kNew);
    CheckAvlEquals(tree, grown, -5, 15);
  }
}

// ======================================================================
//  Copy-ctor: exception propagates, source stays intact
// ======================================================================

TEST(AvlTreeStrongGuarantee, CopyCtorFailsAtEveryAllocationSourceIntact) {
  const std::vector<int> base = {5, 2, 8, 1, 4, 7, 9, 3};
  const std::set<int> ref(base.begin(), base.end());
  const AvlTreeImpl src = BuildAvl(base);

  AvlTreeImpl probe;
  const std::size_t m = MeasureOpAllocs([&] { probe = src; });
  ASSERT_GT(m, std::size_t{1});

  for (std::size_t nth = 0; nth < m; ++nth) {
    SCOPED_TRACE("fail at alloc #" + std::to_string(nth));
    const std::size_t live_before = g_live_allocs.load();
    bool caught_bad_alloc = false;
    {
      ScopedAllocBudget budget(static_cast<std::ptrdiff_t>(nth));
      try {
        const AvlTreeImpl copy(src);  //  build, then fail right away
        (void)copy;
        ADD_FAILURE() << "copy unexpectedly succeeded at #" << nth;
      } catch (const std::bad_alloc&) {
        caught_bad_alloc = true;
      } catch (...) {}
    }
    EXPECT_TRUE(caught_bad_alloc) << "exception was swallowed at #" << nth;
    EXPECT_EQ(g_live_allocs.load(), live_before)
        << "partial copy leaked at #" << nth;
    CheckAvlEquals(src, ref, -5, 15);  //  source unharmed
  }

  //  Without the cutoff the copy succeeds.
  const AvlTreeImpl copy(src);
  CheckAvlEquals(copy, ref, -5, 15);
}

// ======================================================================
//  Copy-assignment: on failure the target stays as it was
// ======================================================================

TEST(AvlTreeStrongGuarantee, CopyAssignFailsAtEveryAllocationTargetIntact) {
  const std::vector<int> target_keys = {50, 40, 60};
  const std::vector<int> src_keys = {5, 2, 8, 1, 4, 7, 9, 3};
  const std::set<int> target_ref(target_keys.begin(), target_keys.end());
  const std::set<int> src_ref(src_keys.begin(), src_keys.end());

  const AvlTreeImpl src = BuildAvl(src_keys);
  AvlTreeImpl probe = BuildAvl(target_keys);
  const std::size_t m = MeasureOpAllocs([&] { probe = src; });
  ASSERT_GT(m, std::size_t{1});

  for (std::size_t nth = 0; nth < m; ++nth) {
    SCOPED_TRACE("fail at alloc #" + std::to_string(nth));
    AvlTreeImpl target = BuildAvl(target_keys);
    const std::size_t live_before = g_live_allocs.load();
    bool caught_bad_alloc = false;
    {
      ScopedAllocBudget budget(static_cast<std::ptrdiff_t>(nth));
      try {
        target = src;
        ADD_FAILURE() << "assign unexpectedly succeeded at #" << nth;
      } catch (const std::bad_alloc&) {
        caught_bad_alloc = true;
      } catch (...) {}
    }
    EXPECT_TRUE(caught_bad_alloc) << "exception was swallowed at #" << nth;
    EXPECT_EQ(g_live_allocs.load(), live_before)
        << "partial assign leaked at #" << nth;
    CheckAvlEquals(target, target_ref, -5,
                   70);  //  target intact, exactly as before
  }

  //  Successful assignment after all failures.
  AvlTreeImpl target = BuildAvl(target_keys);
  target = src;
  CheckAvlEquals(target, src_ref, -5, 15);
}

// ======================================================================
//  Separately: bad_alloc arrives as its own type (regression for `throw e;`)
// ======================================================================

TEST(AvlTreeStrongGuarantee, FailureTypeIsExactlyBadAlloc) {
  const std::vector<int> base = {5, 2, 8};
  AvlTreeImpl tree = BuildAvl(base);
  std::type_index thrown_type = typeid(void);
  {
    ScopedAllocBudget budget(0);
    try {
      tree.Insert(1);
    } catch (const std::exception& e) {
      thrown_type = typeid(e);
    }
  }
  EXPECT_TRUE(thrown_type == typeid(std::bad_alloc))
      << "allocation failure escaped as " << thrown_type.name();
}

}  // namespace

// ======================================================================
//  Global operator new/delete replacement for the failpoint harness.
//  Outside ScopedAllocBudget it behaves like the standard one (pass-through malloc).
// ======================================================================

namespace {
void* FailpointMalloc(std::size_t size) {
  void* ptr = std::malloc(size ? size : 1);
  if (ptr) {
    g_alloc_count.fetch_add(1, std::memory_order_relaxed);
    g_live_allocs.fetch_add(1, std::memory_order_relaxed);
  }
  return ptr;
}
void FailpointFree(void* ptr) {
  if (ptr) {
    g_live_allocs.fetch_sub(1, std::memory_order_relaxed);
    std::free(ptr);
  }
}
}  // namespace

void* operator new(std::size_t size) {
  if (TakeAllocStep()) {
    throw std::bad_alloc();
  }
  void* ptr = FailpointMalloc(size);
  if (!ptr) {
    throw std::bad_alloc();
  }
  return ptr;
}

void* operator new[](std::size_t size) {
  return ::operator new(size);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  if (TakeAllocStep()) {
    return nullptr;
  }
  return FailpointMalloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  return ::operator new(size, std::nothrow);
}

void operator delete(void* ptr) noexcept {
  FailpointFree(ptr);
}

void operator delete[](void* ptr) noexcept {
  operator delete(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
  operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
  operator delete(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
  FailpointFree(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
  FailpointFree(ptr);
}
