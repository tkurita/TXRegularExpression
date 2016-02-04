#include <CoreFoundation/CoreFoundation.h>
#include "TXRegularExpression.h"
#include "icu_regex.h"

#define useLog 0

#pragma mark internal functions

#define SafeRelease(x) if(x) CFRelease(x)

#define TXRegexGetStruct(x) (TXRegexStruct *)CFDataGetBytePtr(x);

CFStringRef CFStringRetainAndGetUTF16Ptr(CFStringRef text, UniChar **outptr, CFIndex *length)
{
	CFStringRef result = NULL;
	*outptr = (UniChar *)CFStringGetCharactersPtr(text);
	*length = CFStringGetLength(text);
	if (*outptr) {
		result = CFRetain(text);
	} else {
#if useLog
		fputs("will malloc in CFStringRetainAndGetUTF16Ptr\n", stderr);
#endif		
		size_t required_size = *length * sizeof(UniChar);
		UniChar *buffer = malloc(required_size);
		if (!buffer) goto bail;
		CFStringGetCharacters(text, CFRangeMake(0L, *length), buffer); // Convert regexString to UTF16.
		*outptr = (UniChar *)buffer;
		result = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, *length, kCFAllocatorMalloc);
	}
bail:	
	return result;
}

CFStringRef CFStringCreateWithRegexGroupWithLength(TXRegexRef regexp, CFIndex gnum, CFIndex len, UErrorCode *status)
{
	CFStringRef result = NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	URegularExpression *re = regexp_struct->uregexp;
	int32_t buffer_size = len * sizeof(UniChar);
	UChar *buffer = malloc(buffer_size);
	int32_t returned_size = uregex_group(re, gnum, buffer, buffer_size, status);
	if (returned_size) {
		result = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, len, kCFAllocatorMalloc);
	} else {
		free(buffer);
	}
	return result;
}

CFArrayRef CFArrayCreateWithCapturedGroups(TXRegexRef regexp, UErrorCode *status)
{
	CFMutableArrayRef result = NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	URegularExpression *re = regexp_struct->uregexp;
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
			text = CFStringCreateWithRegexGroupWithLength(regexp, n, end-start, status);
		}
		CFArrayAppendValue(result, text);
		CFRelease(text);
	}
bail:
	return result;	
}

CFArrayRef TXRegexCapturedGroups(TXRegexRef regexp, UErrorCode *status)
{
	CFMutableArrayRef result = NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	URegularExpression *re = regexp_struct->uregexp;
	int32_t gcount = uregex_groupCount(re, status) + 1;
	if (U_ZERO_ERROR != *status) goto bail;
	result = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringRef keys[] = {CFSTR("start"), CFSTR("end"), CFSTR("text")};
	for (int n = 0; n < gcount; n++) {
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
			values[2] = CFStringCreateWithRegexGroupWithLength(regexp, n, end-start, status);
		}
		CFDictionaryRef dict = CFDictionaryCreate(kCFAllocatorDefault, (void *)keys, (void *)values, 3,  
												  &kCFTypeDictionaryKeyCallBacks,  &kCFTypeDictionaryValueCallBacks);
		CFArrayAppendValue(result, dict);
		CFRelease(dict);
		CFRelease(values[0]);
		CFRelease(values[2]);
		CFRelease(values[1]);
	}
bail:
	return result;
}

#pragma mark Regex functions

CFIndex TXRegexSetString(TXRegexRef regexp, CFStringRef text, UErrorCode *status)
{
	UniChar *uchars = NULL;
	CFIndex length = 0;
	CFStringRef text_retained = CFStringRetainAndGetUTF16Ptr(text, &uchars, &length);
	if (!text_retained) return 0;
	TXRegexStruct* regex_struct = TXRegexGetStruct(regexp);
	if (regex_struct->targetString) {
#if useLog
		fputs("before uregex_reset\n", stderr);
#endif		
		uregex_reset(regex_struct->uregexp, 0, status);
		if (U_ZERO_ERROR != *status) goto bail;
		CFRelease(regex_struct->targetString);
	}
	
	uregex_setText(regex_struct->uregexp, uchars, (int32_t)length, status);
	if (U_ZERO_ERROR != *status) goto bail;
	
	regex_struct->targetString = text_retained;

	return length;
bail:
	SafeRelease(text_retained);
	return 0;
}

static void TXRegexFree(TXRegexStruct *regexp)
{
	if (!regexp) return;
	uregex_close(regexp->uregexp);
	SafeRelease(regexp->targetString);
	free(regexp);
}

static void TXRegexDeallocate(void *ptr, void *info)
{
#if useLog
	fputs("TXRegexDeallocate\n", stderr);
#endif		
	TXRegexStruct *regexp = (TXRegexStruct *)ptr;
	uregex_close(regexp->uregexp);
	SafeRelease(regexp->targetString);
	free(regexp);
}

char *austrdup(const UChar* unichars)

{
    int   length;
    char *newString;
	
    length    = u_strlen ( unichars );
    newString = (char*)malloc ( sizeof( char ) * 4 * ( length + 1 ) );
    if ( newString == NULL )
        return NULL;
	
    u_austrcpy ( newString, unichars );
	
    return newString;
}

void fprintParseError(FILE *stream, UParseError *parse_error)
{
	char *post_context = austrdup(parse_error->postContext);
	char *pre_context = austrdup(parse_error->preContext);
	fprintf(stream ,"line : %d, offset : %d, precontext : %s, postcontext : %s\n",
			parse_error->line, parse_error->offset, pre_context, post_context);
	free(post_context);
	free(pre_context);
}

CFStringRef CFStringCreateWithFormattingParseError(UParseError *parse_error)
{
    char *post_context = austrdup(parse_error->postContext);
	char *pre_context = austrdup(parse_error->preContext);
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
								CFSTR("line : %d, offset : %d, precontext : %s, postcontext : %s\n"),
								parse_error->line, parse_error->offset, pre_context, post_context);
    free(post_context);
	free(pre_context);
}

static CFAllocatorRef CreateTXRegexDeallocator(void) {
    static CFAllocatorRef allocator = NULL;
    if (!allocator) {
        CFAllocatorContext context =
		{0, // version
			NULL, //info
			NULL, // retain callback
			(void *)free,  //  CFAllocatorReleaseCallBack
			NULL, // CFAllocatorCopyDescriptionCallBack
		NULL, //CFAllocatorAllocateCallBack
		NULL, // CFAllocatorReallocateCallBack 
		TXRegexDeallocate, //CFAllocatorDeallocateCallBack 
		NULL //CFAllocatorPreferredSizeCallBack 
		};
        allocator = CFAllocatorCreate(NULL, &context);
    }
    return allocator;
}

TXRegexRef TXRegexCreate(CFAllocatorRef allocator, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status)
{
	TXRegexStruct *regexp_struct = malloc(sizeof(TXRegexStruct));
	if (!regexp_struct) return NULL;

	regexp_struct->targetString = NULL;

	UniChar *uchars = NULL;
	CFIndex length;
	CFStringRef pattern_retained = CFStringRetainAndGetUTF16Ptr(pattern, &uchars, &length);
	if (!pattern_retained) {
		free(regexp_struct);
		return NULL;
	}
		
	regexp_struct->uregexp = uregex_open(uchars, length, options, parse_error, status);
	
	CFRelease(pattern_retained);
	CFAllocatorRef deallocator = CreateTXRegexDeallocator();
	TXRegexRef regexp = CFDataCreateWithBytesNoCopy(allocator, (const UInt8 *)regexp_struct, 
															  sizeof(TXRegexStruct), deallocator);
	return regexp;
}

TXRegexRef TXRegexCreateCopy(CFAllocatorRef allocator, TXRegexRef regexp, UErrorCode *status)
{
#if useLog
	fputs("TXRegexCreateCopy\n", stderr);
#endif	
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	URegularExpression *new_uregexp = uregex_clone(regexp_struct->uregexp, status);
	if (U_ZERO_ERROR != *status) return NULL;
	
	
	TXRegexStruct *new_regexp_struct = malloc(sizeof(TXRegexStruct));
	if (!new_regexp_struct) return NULL;
	new_regexp_struct->targetString = NULL;
	CFAllocatorRef deallocator = CreateTXRegexDeallocator();
	TXRegexRef new_regexp = CFDataCreateWithBytesNoCopy(allocator, (const UInt8 *)new_regexp_struct, 
													sizeof(TXRegexStruct), deallocator);
	new_regexp_struct->uregexp = new_uregexp;
	
	return new_regexp; 
}


CFStringRef TXRegexCopyPatternString(TXRegexRef regexp, UErrorCode *status)
{
	int32_t len;
	const UChar *uchars = NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	uchars = uregex_pattern(regexp_struct->uregexp, &len, status);
	if (U_ZERO_ERROR != *status) return NULL;
	return CFStringCreateWithCharacters(kCFAllocatorDefault, uchars, len);
}

CFStringRef TXRegexCopyTargetString(TXRegexRef regexp, UErrorCode *status)
{
	int32_t len;
	const UChar *uchars = NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	uchars = uregex_getText(regexp_struct->uregexp, &len, status);
	if (U_ZERO_ERROR != *status) return NULL;
	return CFStringCreateWithCharacters(kCFAllocatorDefault, uchars, len);
}

CFArrayRef CFArrayCreateWithFirstMatch(TXRegexRef regexp, CFIndex startIndex, UErrorCode *status)
{
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	if (!uregex_find(regexp_struct->uregexp, startIndex, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return CFArrayCreateWithCapturedGroups(regexp, status);
}

CFArrayRef CFArrayCreateWithNextMatch(TXRegexRef regexp, UErrorCode *status)
{
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	if (!uregex_findNext(regexp_struct->uregexp, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return CFArrayCreateWithCapturedGroups(regexp, status);
}

CFArrayRef TXRegexFirstMatch(TXRegexRef regexp, CFIndex startIndex, UErrorCode *status)
{
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	if (!uregex_find(regexp_struct->uregexp, startIndex, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexCapturedGroups(regexp, status);
}

CFArrayRef TXRegexNextMatch(TXRegexRef regexp, UErrorCode *status)
{
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	if (!uregex_findNext(regexp_struct->uregexp, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexCapturedGroups(regexp, status);
}

CFArrayRef TXRegexFirstMatchInString(TXRegexRef regexp, CFStringRef text, CFIndex startIndex, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	return TXRegexFirstMatch(regexp, startIndex, status);
}

CFArrayRef TXRegexAllMatchesInString(TXRegexRef regexp, CFStringRef text, UErrorCode *status)
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
Boolean CFStringIsMatchedWithRegex(CFStringRef text, TXRegexRef regexp, UErrorCode *status)
{
	Boolean result = false;
	if (TXRegexSetString(regexp, text, status)) {
		TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
		result = (Boolean)uregex_matches(regexp_struct->uregexp, 0, status);	
	}
	return result;
}

Boolean CFStringIsMatchedWithPattern(CFStringRef text, CFStringRef pattern, uint32_t options, UParseError *parse_error, UErrorCode *status)
{
	TXRegexRef regexp = TXRegexCreate(kCFAllocatorDefault, pattern, options, parse_error, status);
	if (!regexp) return false;
	if (U_ZERO_ERROR != *status) {
		CFRelease(regexp);
		return false;
	}
	Boolean result =  CFStringIsMatchedWithRegex(text, regexp, status);
	CFRelease(regexp);
	
	return result;
}

CFArrayRef CFStringCreateArrayWithFirstMatch(CFStringRef text, TXRegexRef regexp, CFIndex startIndex, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	
	return CFArrayCreateWithFirstMatch(regexp, startIndex, status);
}

CFArrayRef CFStringCreateArrayWithAllMatches(CFStringRef text, TXRegexRef regexp, UErrorCode *status)
{
#if useLog
	fputs("CFStringCreateArrayWithAllMatches\n", stderr);
#endif	
	TXRegexSetString(regexp, text, status);
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

CFArrayRef CFStringCreateArrayByRegexSplitting(CFStringRef text, TXRegexRef regexp, UErrorCode *status)
{
	if (!TXRegexSetString(regexp, text, status)) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);	
	URegularExpression *re = regexp_struct->uregexp;
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

CFStringRef CFStringCreateByReplacingFirstMatch(CFStringRef text, TXRegexRef regexp, 
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
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	int32_t result_len = uregex_replaceFirst(regexp_struct->uregexp, replacement_chars, replacement_len, buffer, capacity, status);
	CFRelease(replacement_retained);
	if (U_ZERO_ERROR != *status) {
		free(buffer);
		return NULL;
	}

	return CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, result_len, kCFAllocatorMalloc);
}

CFStringRef CFStringCreateByReplacingAllMatches(CFStringRef text, TXRegexRef regexp, 
												CFStringRef replacement, UErrorCode *status)
{
	CFIndex target_len = TXRegexSetString(regexp, text, status);
	if (!target_len) return NULL;
	if (U_ZERO_ERROR != *status) return NULL;
	TXRegexStruct *regexp_struct = TXRegexGetStruct(regexp);
	URegularExpression *re = regexp_struct->uregexp;
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
	
	return CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, result_len, kCFAllocatorMalloc);
}
