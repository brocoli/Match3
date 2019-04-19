#include "catch.hpp"

#include "Util/Array2D.h"


SCENARIO("patterns can be detected inside an Array2D", "[Array2D]") {
    GIVEN("an 8x6 array striped vertically instances") {
        Match3::Util::Array2D<size_t> arr(8, 6, -1);
        for (size_t j = 0; j < arr.GetCols(); j += 2) {
            for (size_t i = 0; i < arr.GetRows(); ++i) {
                arr[i][j] = 0;
            }
        }

        AND_GIVEN("a simple vertical pattern mask") {
            Match3::Util::Array2D<size_t> pat(1, 3, 0);

            WHEN("we collect patternInstances") {
                std::vector<size_t> patternInstances = arr.FindPatternInstances(pat);

                THEN("we find 16 pattern matches") {
                    REQUIRE(patternInstances.size() == 32);
                }
            }
        }

        AND_GIVEN("a simple horizontal pattern mask") {
            Match3::Util::Array2D<size_t> pat(3, 1, 0);

            WHEN("we collect patternInstances") {
                std::vector<size_t> patternInstances = arr.FindPatternInstances(pat);

                THEN("we find 0 pattern matches") {
                    REQUIRE(patternInstances.size() == 0);
                }
            }
        }
    }

    GIVEN("an 8x6 array striped horizontally instances") {
        GIVEN("an 8x6 array striped vertically instances") {
            Match3::Util::Array2D<size_t> arr(8, 6, -1);
            for (size_t j = 0; j < arr.GetCols(); ++j) {
                for (size_t i = 0; i < arr.GetRows(); i += 2) {
                    arr[i][j] = 0;
                }
            }

            AND_GIVEN("a simple vertical pattern mask") {
                Match3::Util::Array2D<size_t> pat(1, 3, 0);

                WHEN("we collect patternInstances") {
                    std::vector<size_t> patternInstances = arr.FindPatternInstances(pat);

                    THEN("we find 0 pattern matches") {
                        REQUIRE(patternInstances.size() == 0);
                    }
                }
            }

            AND_GIVEN("a simple horizontal pattern mask") {
                Match3::Util::Array2D<size_t> pat(3, 1, 0);

                WHEN("we collect patternInstances") {
                    std::vector<size_t> patternInstances = arr.FindPatternInstances(pat);

                    THEN("we find 18 pattern matches") {
                        REQUIRE(patternInstances.size() == 36);
                    }
                }
            }
        }
    }
}