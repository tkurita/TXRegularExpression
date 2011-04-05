#include <CoreFoundation/CoreFoundation.h>
#include "TXRegularExpression.h"
#include "icu_regex.h"

#pragma mark internal functions

void SafeRelease(CFTypeRef value)
{
	if (value) CFRelease(value);
}

CFStringRef CFStringRetainAndGetUTF16Ptr(CFStringRef text, UniChar **outptr, CFIndex *length)
{
	*outptr = (UniChar *)CFStringGetCharactersPtr(text);
	*length = CFStringGetLength(text);
	CFStringRef result = NULL;
	if (*outptr) {
		result = CFRetain(text);
	} else {
		size_t required_size = *length * sizeof(UniChar);
		UniChar *buffer = malloc(required_size);
		if (!buffer) return false;
		CFStringGetCharacters(text, CFRangeMake(0L, *length), buffer); // Convert regexString to UTF16.
		*outptr = (UniChar *)buffer;
		result = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, *length, kCFAllocatorDefault);
	}
	return result;
}

CFStringRef CFStringCreateWithRegexGroupWithLength(TXRegularExpression *regexp, CFIndex gnum, CFIndex len, UErrorCode *status)
{
	CFStringRef result = NULL;
	URegularExpression *re = regexp->uregexp;
	int32_t buffer_size = len * sizeof(UniChar);
	UChar *buffer = malloc(buffer_size);
	int32_t returned_size = uregex_group(re, gnum, buffer, buffer_size, status);
	if (returned_size) {
		result = CFStringCreateWithCharacters(kCFAllocatorDefault, buffer, len);
	} 
	return result;
}

CFArrayRef CFArrayCreateWithCapturedGroups(TXRegularExpression *regexp, UErrorCode *status)
{
	CFMutableArrayRef result = NULL;
	URegularExpression *re = regexp->uregexp;
	int32_t gcount = uregex_groupCount(re, status) + 1;
	if (U_ZERO_ERROR != *status) return NULL;
	result = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (int n = 0; n < gcount; n++) {
		int32_t start = uregex_start(re, n, status);
		if (U_ZERO_ERROR != *status) goto bail;
		int32_t end = uregex_end(re, n, status);
		if (U_ZERO_ERROR != *status) goto bail;
		CFStringRef text;
		if (-1 == start) {
			text = CFSTR("");
		} else {
			text = CFStringCreateWithRegexGroupWithLength(regexp, n, end-start+1, status);
		}
		CFArrayAppendValue(result, text);
		CFRelease(text);
	}
bail:
	return result;	
}

CFArrayRef TXRegexCapturedGroups(TXRegularExpression *regexp, UErrorCode *status)
{
	CFMutableArrayRef result = NULL;
	URegularExpression *re = regexp->uregexp;
	int32_t gcount = uregex_groupCount(re, status) + 1;
	if (U_ZERO_ERROR != *status) goto bail;
	result = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (int n = 0; n < gcount; n++) {
		CFStringRef keys[] = {CFSTR("start"), CFSTR("end"), CFSTR("text")};
		CFTypeRef values[3];
		int32_t start = uregex_start(re, n, status);
		if (U_ZERO_ERROR != *status) goto bail;
		int32_t end = uregex_end(re, n, status);
		if (U_ZERO_ERROR != *status) goto bail;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &start);
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &end);
		if (-1 == start) {
			values[2] = CFSTR("");
		} else {
			values[2] = CFStringCreateWithRegexGroupWithLength(regexp, n, end-start+1, status);
		}
		CFDictionaryRef dict = CFDictionaryCreate(kCFAllocatorDefault, (void *)keys, (void *)values, 3,  
												  &kCFTypeDictionaryKeyCallBacks,  &kCFTypeDictionaryValueCallBacks);
		CFArrayAppendValue(result, dict);
		CFRelease(dict);
	}
bail:
	return result;
}

#pragma mark Regex functions

CFIndex TXRegexSetString(TXRegularExpression *regexp, CFStringRef text, UErrorCode *status)
{
	UniChar *uchars = NULL;
	CFIndex length = 0;
	CFStringRef text_retained = CFStringRetainAndGetUTF16Ptr(text, &uchars, &length);
	if (!text_retained) return 0;
	
	if (regexp->targetString) {
		uregex_reset(regexp->uregexp, 0, status);
		if (U_ZERO_ERROR != *status) goto bail;
		CFRelease(regexp->targetString);
	}
	
	uregex_setText(regexp->uregexp, uchars, (int32_t)length, status);
	if (U_ZERO_ERROR != *status) goto bail;
	
	regexp->targetString = text_retained;

	return length;
bail:
	SafeRelease(text_retained);
	return 0;
}

void TXRegexFree(TXRegularExpression *regexp)
{
	if (!regexp) return;
	uregex_close(regexp->uregexp);
	SafeRelease(regexp->targetString);
	free(regexp);
}

void fprintParseError(FILE *stream, UParseError *parse_error)
{

	fprintf(stream ,"line : %d, offset : %d, precontext : %s, postcontext : %s\n",
			parse_error->line, parse_error->offset, parse_error->preContext, parse_error->postContext);
	
}

CFStringRef CFStringCreateWithFormattingParseError(UParseError *parse_error)
{
		return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, 
								CFSTR("line : %d, offset : %d, precontext : %s, postcontext : %s\n"),
								parse_error->line, parse_error->offset, parse_error->preContext, parse_error->postContext);
}

TXRegularExpression* TXRegexCreate(CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status)
{
	TXRegularExpression *regexp = malloc(sizeof(TXRegularExpression));
	if (!regexp) return NULL;

	regexp->targetString = NULL;

	UniChar *uchars = NULL;
	CFIndex length;
	CFStringRef pattern_retained = CFStringRetainAndGetUTF16Ptr(pattern, &uchars, &length);
	if (!pattern_retained) {
		free(regexp);
		return NULL;
	}
		
	regexp->uregexp = uregex_open(uchars, length, options, parse_error, status);
	
	CFRelease(pattern_retained);
	return regexp;
}

CFStringRef TXRegexCreatePatternString(TXRegularExpression *regexp, UErrorCode *status)
{
	int32_t len;
	const UChar *uchars = NULL;
	uchars = uregex_pattern(regexp->uregexp, &len, status);
	if (U_ZERO_ERROR != *status) return NULL;
	return CFStringCreateWithCharacters(kCFAllocatorDefault, uchars, len);
}

CFStringRef TXRegexCreateTargetString(TXRegularExpression *regexp, UErrorCode *status)
{
	int32_t len;
	const UChar *uchars = NULL;
	uchars = uregex_getText(regexp->uregexp, &len, status);
	if (U_ZERO_ERROR != *status) return NULL;
	return CFStringCreateWithCharacters(kCFAllocatorDefault, uchars, len);
}

CFArrayRef CFArrayCreateWithFirstMatch(TXRegularExpression *regexp, CFIndex startIndex, UErrorCode *status)
{
	if (!uregex_find(regexp->uregexp, startIndex, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return CFArrayCreateWithCapturedGroups(regexp, status);
}

CFArrayRef CFArrayCreateWithNextMatch(TXRegularExpression *regexp, UErrorCode *status)
{
	if (!uregex_findNext(regexp->uregexp, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return CFArrayCreateWithCapturedGroups(regexp, status);
}

CFArrayRef TXRegexFirstMatch(TXRegularExpression *regexp, CFIndex startIndex, UErrorCode *status)
{
	if (!uregex_find(regexp->uregexp, startIndex, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexCapturedGroups(regexp, status);
}

CFArrayRef TXRegexNextMatch(TXRegularExpression *regexp, UErrorCode *status)
{
	if (!uregex_findNext(regexp->uregexp, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexCapturedGroups(regexp, status);
}

CFArrayRef TXRegexFirstMatchInString(TXRegularExpression *regexp, CFStringRef text, CFIndex startIndex, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexFirstMatch(regexp, startIndex, status);
}

CFArrayRef TXRegexAllMatchesInString(TXRegularExpression *regexp, CFStringRef text, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;

	CFMutableArrayRef matches = NULL;
	matches = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	CFArrayRef a_match = NULL;
	while (a_match = TXRegexNextMatch(regexp, status)) {
		if(U_ZERO_ERROR != *status) {
			CFRelease(a_match);
			break;
		}
		CFArrayAppendValue(matches, a_match);
		CFRelease(a_match);		
	}
	
	return matches;
}

#pragma mark additions to CFString
Boolean CFStringIsMatchedWithRegex(CFStringRef text, TXRegularExpression *regexp, UErrorCode *status)
{
	Boolean result = false;
	if (TXRegexSetString(regexp, text, status)) {
		result = (Boolean)uregex_matches(regexp->uregexp, 0, status);	
	}
	return result;
}

Boolean CFStringIsMatchedWithPattern(CFStringRef text, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status)
{
	TXRegularExpression *regexp = TXRegexCreate(pattern, options, parse_error, status);
	if (!regexp) return false;
	if (U_ZERO_ERROR != *status) {
		TXRegexFree(regexp);
		return false;
	}
	Boolean result =  CFStringIsMatchedWithRegex(text, regexp, status);
	TXRegexFree(regexp);
	
	return result;
}

CFArrayRef CFStringCreateArrayWithFirstMatch(CFStringRef text, TXRegularExpression *regexp, CFIndex startIndex, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	
	return CFArrayCreateWithFirstMatch(regexp, startIndex, status);
}

CFArrayRef CFStringCreateArrayWithAllMatches(CFStringRef text, TXRegularExpression *regexp, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;

	CFArrayRef groups = NULL;
	CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	while (groups = CFArrayCreateWithNextMatch(regexp, status)) {
		if (U_ZERO_ERROR != *status) {
			CFRelease(groups);
			break;
		}
		CFArrayAppendValue(array, groups);
		CFRelease(groups);
	}
	
	return array;
}

CFArrayRef CFStringCreateArrayByRegexSplitting(CFStringRef text, TXRegularExpression *regexp, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	
	URegularExpression *re = regexp->uregexp;
	CFIndex length = CFStringGetLength(text);
	CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	
	int32_t preend = 0;
	int32_t start = 0;
	int32_t end = 0;
	CFStringRef substring = NULL;
	while(uregex_findNext(re, status)) {		
		start = uregex_start(re, 0, status);
		if (start < 0) goto bail;
		if (U_ZERO_ERROR != *status) goto bail;
		
		end = uregex_end(re, 0, status);
		if (end < 0) goto bail;
		if (U_ZERO_ERROR != *status) goto bail;
		
		if (start > preend) {
			CFRange range = CFRangeMake(preend, start-preend);
			substring = CFStringCreateWithSubstring(kCFAllocatorDefault, text, range);
			CFArrayAppendValue(array, substring);
			CFRelease(substring);
		} else if (start == preend) {
			CFArrayAppendValue(array, CFSTR(""));
		}
		preend = end;
	}
	if (length > preend) {
		CFRange range = CFRangeMake(preend, length-preend);
		substring = CFStringCreateWithSubstring(kCFAllocatorDefault, text, range);
		CFArrayAppendValue(array, substring);
		CFRelease(substring);		
	} 
	
	return array;
bail:
	CFRelease(array);
	return NULL;
}

CFStringRef CFStringCreateByReplacingFirstMatch(CFStringRef text, TXRegularExpression *regexp, 
												CFStringRef replacement, UErrorCode *status)
{
	CFIndex target_len = TXRegexSetString(regexp, text, status);
	if (!target_len) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	
	UniChar *replacement_chars = NULL;
	CFIndex replacement_len = 0;
	CFStringRef replacement_retained = CFStringRetainAndGetUTF16Ptr(replacement, &replacement_chars, &replacement_len);
	if (!replacement_retained) return NULL;
	
	int32_t capacity = target_len + replacement_len;
	UChar *buffer = malloc(capacity * sizeof(UChar));
	int32_t result_len = uregex_replaceFirst(regexp->uregexp, replacement_chars, replacement_len, buffer, capacity, status);
	CFRelease(replacement_retained);
	if (U_ZERO_ERROR != *status) {
		free(buffer);
		return NULL;
	}

	return CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, result_len, kCFAllocatorDefault);
}

CFStringRef CFStringCreateByReplacingAllMatches(CFStringRef text, TXRegularExpression *regexp, 
												CFStringRef replacement, UErrorCode *status)
{
	CFIndex target_len = TXRegexSetString(regexp, text, status);
	if (!target_len) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	
	URegularExpression *re = regexp->uregexp;
	UniChar *replacement_chars = NULL;
	CFIndex replacement_len = 0;
	CFStringRef replacement_retained = CFStringRetainAndGetUTF16Ptr(replacement, &replacement_chars, &replacement_len);
	if (!replacement_retained) return NULL;
	
	int32_t capacity = target_len + replacement_len;
	UChar *buffer = malloc(capacity * sizeof(UChar));
	int32_t result_len = uregex_replaceAll(re, replacement_chars, replacement_len, buffer, capacity, status);
	while ((U_BUFFER_OVERFLOW_ERROR == *status) || (U_STRING_NOT_TERMINATED_WARNING == *status)) {
		*status = U_ZERO_ERROR;
		uregex_reset(re, 0, status);
		capacity = result_len+1; // to avoid U_STRING_NOT_TERMINATED_WARNING
		buffer = reallocf(buffer, capacity*sizeof(UChar));
		if (!buffer) break;
		result_len = uregex_replaceAll(re, replacement_chars, replacement_len, buffer, capacity, status);
	}
	
	CFRelease(replacement_retained);
	if (U_ZERO_ERROR != *status) {
		free(buffer);
		return NULL;
	}
	
	return CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, result_len, kCFAllocatorDefault);
}
