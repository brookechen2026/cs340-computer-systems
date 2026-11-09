#include <stddef.h>

/**
 * If `*utf8` points to the first byte of a valid UTF-8-encoded code point,
 * updates `*utf8`to point to the byte after that code point and returns the code point.
 *
 * Otherwise, returns a negative number and does nothing else.
 *
 * @param utf8 A pointer to a pointer to an array of immutable bytes.
 *             `**utf8` points to the first character of a UTF-8 encoded string.
 *             `*utf8` is increased by 1, 2, 3, or 4 on a successful invocation.
 *
 * @return A code point (a number between 0 and 0x10FFFF) on success, or a negative
 *         number on a failure.
 */
int decodeCharacter(const char **utf8);


/**
 * Encodes `codepoint` in UTF-8 and writes its bytes to the array pointed to by `*utf8`,
 * incrementing `*utf8` and decrementing `*space` by the number of bytes used and then
 * returning that number of bytes. If `codepoint` is an invalid code point (negative or
 * greater than 0x10FFFF) or the number of bytes exceeds `*space`, instead returns 0 and
 * does nothing else.
 *
 * @param utf8      A pointer to a pointer to an array of bytes.
 *                  The codepoint is encoded in UTF-8 starting at byte `*utf8`.
 *                 `*utf8` is increased by 1, 2, 3, or 4 on a successful invocation.
 *
 * @param space     A pointer to the space left in the array of bytes.
 *                  `*space* is decremented by the number of bytes written.
 *
 * @param codepoint The code point of the character to write.
 *
 * @return The number of bytes written on success, or 0 on failure.
 */
int encodeCharacter(char **utf8, size_t *space, int codepoint);


/**
 * Like `strlen`, but counts unicode characters instead of `char`s.
 * If an invalid UTF-8 byte is encountered, stops as if it were the NUL byte.
 * 
 * @param s A UTF-8 encoded string.
 * @return The number of unicode characters in the string (excluding the terminating NUL).
 */
size_t strlen8c(const char *s);

/**
 * Returns the number of bytes needed to encode the given code points in UTF-8.
 * If an invalid code point is encountered, stops as if it were the NUL character.
 * 
 * @param s A null-terminated list of code points.
 * @return The number of bytes needed to encde this in UTF-8 (excluding the terminating NUL).
 */
size_t strlen8i(const int *s);


typedef struct _replacement_set_t ReplacementSet;

/**
 * Allocates and initializes a ReplacementSet.
 */
ReplacementSet *newReplacementSet();

/**
 * Performs clean-up on and deallocates a ReplacementSet.
 * Do not use `set` after calling this function.
 */
void deleteReplacementSet(ReplacementSet *set);


/**
 * Add a new string replacement to the given replacement set.
 * Assumes that the `from` is a non-empty string not previously added to this set
 * and that the memory for both strings is managed externally.
 *
 * @param set  The replacement set to add this translation to.
 * @param from A substring to be replaced
 * @param to   The string to replace it with
 */
void addReplacement(ReplacementSet *set, const char *from, const char *to);

/**
 * Allocates (with `malloc`) and returns a new string created by
 * applying the replacement set to the input string.
 *
 * @param set    The replacement set to use.
 * @param input  The string to replace parts of.
 * @return       A `malloc`-allocated string with all replacements applied.
 */
char *replaceAll(ReplacementSet *set, const char *input);
