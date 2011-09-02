#include "UErrorCode.h"
//#include "unicode/uregex.h"

#pragma mark icu definitions
struct URegularExpression;
typedef struct URegularExpression URegularExpression;

typedef int8_t 	UBool;
typedef uint16_t UChar;

enum { U_PARSE_CONTEXT_LEN = 16 };

typedef struct UParseError {
	/**
	 * The line on which the error occured.  If the parser uses this
	 * field, it sets it to the line number of the source text line on
	 * which the error appears, which will be be a value >= 1.  If the
	 * parse does not support line numbers, the value will be <= 0.
	 * @stable ICU 2.0
	 */
	int32_t        line;
	
	/**
	 * The character offset to the error.  If the line field is >= 1,
	 * then this is the offset from the start of the line.  Otherwise,
	 * this is the offset from the start of the text.  If the parser
	 * does not support this field, it will have a value < 0.
	 * @stable ICU 2.0
	 */
	int32_t        offset;
	
	/**
	 * Textual context before the error.  Null-terminated.  The empty
	 * string if not supported by parser.
	 * @stable ICU 2.0   
	 */
	UChar          preContext[U_PARSE_CONTEXT_LEN];
	
	/**
	 * The error itself and/or textual context after the error.
	 * Null-terminated.  The empty string if not supported by parser.
	 * @stable ICU 2.0   
	 */
	UChar          postContext[U_PARSE_CONTEXT_LEN];
	
} UParseError;


typedef enum URegexpFlag{
#ifndef U_HIDE_DRAFT_API 
	/** Forces normalization of pattern and strings. 
	 Not implemented yet, just a placeholder, hence draft. 
	 @draft ICU 2.4 */
	UREGEX_CANON_EQ         = 128,
#endif
	/**  Enable case insensitive matching.  @stable ICU 2.4 */
	UREGEX_CASE_INSENSITIVE = 2,
	
	/**  Allow white space and comments within patterns  @stable ICU 2.4 */
	UREGEX_COMMENTS         = 4,
	
	/**  If set, '.' matches line terminators,  otherwise '.' matching stops at line end.
	 *  @stable ICU 2.4 */
	UREGEX_DOTALL           = 32,
	
	/**   Control behavior of "$" and "^"
	 *    If set, recognize line terminators within string,
	 *    otherwise, match only at start and end of input string.
	 *   @stable ICU 2.4 */
	UREGEX_MULTILINE        = 8,
	
	/**  Unicode word boundaries.
	 *     If set, \b uses the Unicode TR 29 definition of word boundaries.
	 *     Warning: Unicode word boundaries are quite different from
	 *     traditional regular expression word boundaries.  See
	 *     http://unicode.org/reports/tr29/#Word_Boundaries
	 *     @stable ICU 2.8
	 */
	UREGEX_UWORD            = 256
}  URegexpFlag;

#pragma mark Regex functions

typedef struct  {
	URegularExpression *uregexp;
	CFStringRef targetString;
} TXRegexStruct;

typedef CFDataRef TXRegexRef;

TXRegexRef TXRegexCreate(CFAllocatorRef allocator, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status);
TXRegexRef TXRegexCreateCopy(CFAllocatorRef allocator, TXRegexRef regexp, UErrorCode *status);

void fprintParseError(FILE *stream, UParseError *parse_error);
//void TXRegexFree(TXRegexStruct *regexp);
CFIndex TXRegexSetString(TXRegexRef regexp, CFStringRef text, UErrorCode *status);

CFArrayRef TXRegexFirstMatchInString(TXRegexRef regexp, CFStringRef text, CFIndex startIndex, UErrorCode *status);
CFArrayRef TXRegexNextMatch(TXRegexRef regexp, UErrorCode *status);
CFArrayRef TXRegexAllMatchesInString(TXRegexRef regexp, CFStringRef text, UErrorCode *status);
CFStringRef TXRegexCopyPatternString(TXRegexRef regexp, UErrorCode *status);
CFStringRef TXRegexCopyTargetString(TXRegexRef regexp, UErrorCode *status);

#pragma mark additions to CFString
CFStringRef CFStringCreateWithFormattingParseError(UParseError *parse_error);
Boolean CFStringIsMatchedWithRegex(CFStringRef text, TXRegexRef regexp, UErrorCode *status);
Boolean CFStringIsMatchedWithPattern(CFStringRef text, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status);
CFArrayRef CFStringCreateArrayWithFirstMatch(CFStringRef text, TXRegexRef regexp, CFIndex startIndex, UErrorCode *status);
CFArrayRef CFStringCreateArrayWithAllMatches(CFStringRef text, TXRegexRef regexp, UErrorCode *status);
CFArrayRef CFStringCreateArrayByRegexSplitting(CFStringRef text, TXRegexRef regexp, UErrorCode *status);
CFStringRef CFStringCreateByReplacingFirstMatch(CFStringRef text, TXRegexRef regexp, CFStringRef replacement, UErrorCode *status);
CFStringRef CFStringCreateByReplacingAllMatches(CFStringRef text, TXRegexRef regexp, CFStringRef replacement, UErrorCode *status);

