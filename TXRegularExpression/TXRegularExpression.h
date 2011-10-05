#include "UErrorCode.h"
//#include "unicode/uregex.h"

/* TXRegularExpression - regular expression for CoreFoundation
 Copyright (C) 2011 Tetsuro KURITA.
 
 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU Library General Public License as published
 by the Free Software Foundation; either version 2, or (at your option)
 any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 
 You should have received a copy of the GNU Library General Public
 License along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 USA. */


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

#pragma mark TXRegex functions

typedef struct  {
	URegularExpression *uregexp;
	CFStringRef targetString;
} TXRegexStruct;

/*!
 @typedef TXRegexRef
 @abstract A reference to an TXRegularExpresion object.
*/

typedef CFDataRef TXRegexRef;

/*!
 @function TXRegexCreate
 @abstract Create a TXRegularExpression object. 
 @param allocator The allocator to use to allocate memory for the new string. Pass NULL or kCFAllocatorDefault to use the current default allocator.
 @param pattern A string of a regular expression
 @param options options of regular expression.
 @param parse_error A pointer to UParseError to receive information about errors occurred during parsing.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result A reference to TXRegularExpression object.
 */
TXRegexRef TXRegexCreate(CFAllocatorRef allocator, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status);


/*!
 @function TXRegexCreateCopy
 @abstract Copy a TXRegularExpression object. 
 @param regexp A TXRegularExpression object to copy.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result A reference to TXRegularExpression object.
 */
TXRegexRef TXRegexCreateCopy(CFAllocatorRef allocator, TXRegexRef regexp, UErrorCode *status);

/*!
 @function TXRegexSetString
 @abstract Set a taget string to TXRegularExpression object. 
 @param regexp A TXRegularExpression object.
 @param text A string to match with the regular expression.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result length of the string to process.
 */
CFIndex TXRegexSetString(TXRegexRef regexp, CFStringRef text, UErrorCode *status);

CFArrayRef TXRegexFirstMatchInString(TXRegexRef regexp, CFStringRef text, CFIndex startIndex, UErrorCode *status);
CFArrayRef TXRegexNextMatch(TXRegexRef regexp, UErrorCode *status);
CFArrayRef TXRegexAllMatchesInString(TXRegexRef regexp, CFStringRef text, UErrorCode *status);
CFStringRef TXRegexCopyPatternString(TXRegexRef regexp, UErrorCode *status);
CFStringRef TXRegexCopyTargetString(TXRegexRef regexp, UErrorCode *status);

void fprintParseError(FILE *stream, UParseError *parse_error);

#pragma mark additions to CFString
/*!
 @function CFStringCreateWithFormattingParseError
 @abstract Make an error message from UParseError.
 @param parse_error A pointer to UParseError.
 @result A formatted error message.
 */
CFStringRef CFStringCreateWithFormattingParseError(UParseError *parse_error);

Boolean CFStringIsMatchedWithRegex(CFStringRef text, TXRegexRef regexp, UErrorCode *status);
Boolean CFStringIsMatchedWithPattern(CFStringRef text, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status);

/*!
 @function CFStringCreateArrayWithFirstMatch
 @abstract Obtain an arrey of captured groups in the first match.
 @param text A string to process.
 @param regexp A reference to TXRegularExpression object.
 @param startIndex the index of the beginning character of the range to process.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result An array of captured groups.
 */
CFArrayRef CFStringCreateArrayWithFirstMatch(CFStringRef text, TXRegexRef regexp, CFIndex startIndex, UErrorCode *status);

/*!
 @function CFStringCreateArrayWithAllMatches
 @abstract Obtain an arrey of captured groups in all matches.
 @param text A string to process.
 @param regexp A reference to TXRegularExpression object.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result An array of arrays of captured groups.
 */
CFArrayRef CFStringCreateArrayWithAllMatches(CFStringRef text, TXRegexRef regexp, UErrorCode *status);

CFArrayRef CFStringCreateArrayByRegexSplitting(CFStringRef text, TXRegexRef regexp, UErrorCode *status);
CFStringRef CFStringCreateByReplacingFirstMatch(CFStringRef text, TXRegexRef regexp, CFStringRef replacement, UErrorCode *status);

/*!
 @function CFStringCreateByReplacingAllMatches
 @abstract Create a new string by replacing matched strings with a replacement.
 @param text A string to process.
 @param regexp A reference to TXRegularExpression object.
 @param replacement A replacement string for matched strings with regexp.
 @param status A pointer to UErrorCode to recive any errors. U_ZERO_ERROR will be returned when no errors.
 @result A formatted error message.
 */
CFStringRef CFStringCreateByReplacingAllMatches(CFStringRef text, TXRegexRef regexp, CFStringRef replacement, UErrorCode *status);
