#ifndef MathUtils_DEFINED
#define MathUtils_DEFINED

#include <math.h>


/**
 * Clamp a number between two values.
 *
 * Args:
 *     val:
 *         The number to clamp.
 *     min:
 *         The minimum value the number can hold.
 *     max:
 *         The maximum value the number can hold.
 *
 * Returns:
 *     The clamped number such that min <= val <= max.
 */
static inline float clamp(float val, float min, float max) {
    return std::max(min, std::min(max, val));
}


/**
 * Return the maximum of a list of numbers.
 *
 * Args:
 *     nums:
 *         An array of numbers.
 *     count:
 *         The number of elements in 'nums'.
 *
 * Returns:
 *     The highest value from 'nums'.
 */
static inline float manyMax(float* nums, int count) {
    GASSERT(count >= 1);

    float max = nums[0];
    for (int i = 1; i < count; ++i) {
        max = std::max(max, nums[i]);
    }

    return max;
}


/**
 * Return the minimum of a list of numbers.
 *
 * Args:
 *     nums:
 *         An array of numbers.
 *     count:
 *         The number of elements in 'nums'.
 *
 * Returns:
 *     The lowest value from 'nums'.
 */
static inline float manyMin(float* nums, int count) {
    GASSERT(count >= 1);

    float min = nums[0];
    for (int i = 1; i < count; ++i) {
        min = std::min(min, nums[i]);
    }

    return min;
}


#endif
