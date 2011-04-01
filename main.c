#include <CoreFoundation/CoreFoundation.h>
#include "TXRegularExpression.h"

void test_CFStringCreateArrayByRegexSplitting()
{
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("a"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}

	CFArrayRef array = CFStringCreateArrayByRegexSplitting(CFSTR("a dd a"), regexp, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexFirstMatchInString with UErrorCode : %d\n", status);
		return;
	}
	CFShow(array);
}

void test_RegexFirstMatchInString()
{
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("aaa"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}

	CFArrayRef array = TXRegexFirstMatchInString(regexp, CFSTR("aaaadbbbccc"), 0, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexFirstMatchInString with UErrorCode : %d\n", status);
		return;
	}
	
	CFShow(array);	
}

void test_TXRegexAllMatchesInString()
{
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("aaa"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}
	
	CFArrayRef array = TXRegexAllMatchesInString(regexp, CFSTR("aaadbbbaaa"), &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on TXRegexAllMatchesInString with UErrorCode : %d\n", status);
		return;
	}
	
	CFShow(array);	
}

void test_CFStringCreateArrayWithFirstMatch()
{
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("a+"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}
	
	CFArrayRef array = CFStringCreateArrayWithFirstMatch(CFSTR("aaaadbbaabccc"), regexp, 0, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexFirstMatchInString with UErrorCode : %d\n", status);
		return;
	}
	
	CFShow(array);	
}

void test_CFStringCreateArrayWithAllMatches()
{
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("a+"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}
	
	CFArrayRef array = CFStringCreateArrayWithAllMatches(CFSTR("aaaadbbaabccc"), regexp, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on CFStringCreateArrayWithAllMatches with UErrorCode : %d\n", status);
		return;
	}
	
	CFShow(array);	
}

void test_CFStringCreateByReplacingFirstMatch()
{
    // insert code here...
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("a+"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}
	CFStringRef pat = TXRegexCreatePatternString(regexp, &status);
	CFShow(pat);
	
	CFStringRef string = CFStringCreateByReplacingFirstMatch(CFSTR("dd aaa bb aaa"), regexp, CFSTR("ffffff"), &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on CFStringCreateByReplacingFirstMatch with UErrorCode : %d\n", status);
		return;
	}
	CFStringRef target_string = TXRegexCreateTargetString(regexp, &status);
	CFShow(target_string);
	CFShow(string);
}

void test_CFStringCreateByReplacingAllMatches()
{
    // insert code here...
	UParseError parse_error;
	UErrorCode status = U_ZERO_ERROR;
	
	TXRegularExpression *regexp = TXRegexCreate(CFSTR("a+"), 0, &parse_error, &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on RegexCreate with UErrorCode : %d\n", status);
		return;
	}
	CFStringRef pat = TXRegexCreatePatternString(regexp, &status);
	CFShow(pat);
	
	CFStringRef string = CFStringCreateByReplacingAllMatches(CFSTR("aa aa"), regexp, CFSTR("fff"), &status);
	if (status != U_ZERO_ERROR) {
		fprintf(stderr, "Error on CFStringCreateByReplacingAllMatches with UErrorCode : %d\n", status);
		return;
	}
	CFStringRef target_string = TXRegexCreateTargetString(regexp, &status);
	CFShow(target_string);
	CFShow(string);
}

int main (int argc, const char * argv[]) {
	//test_RegexFirstMatchInString();
	test_TXRegexAllMatchesInString();
	//test_CFStringCreateArrayWithFirstMatch();
	//test_CFStringCreateArrayWithAllMatches();
	//test_CFStringCreateArrayByRegexSplitting();
	//test_CFStringCreateByReplacingFirstMatch();
	//test_CFStringCreateByReplacingAllMatches();
	return 0;
}
