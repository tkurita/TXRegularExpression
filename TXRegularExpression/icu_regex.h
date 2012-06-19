URegularExpression *uregex_open(const  UChar   *pattern,
								int32_t         patternLength,
								uint32_t        flags,
								UParseError    *pe,
								UErrorCode     *status);

void uregex_close(URegularExpression *regexp);

URegularExpression* uregex_clone(const URegularExpression * regexp,
								 UErrorCode *status);

const UChar *uregex_pattern(const  URegularExpression   *regexp,
							int32_t           *patLength,
							UErrorCode        *status);

void uregex_setText(URegularExpression *regexp,
					const UChar        *text,
					int32_t             textLength,
					UErrorCode         *status);

const UChar *uregex_getText(URegularExpression *regexp,
							int32_t            *textLength,
							UErrorCode         *status);

UBool uregex_matches(URegularExpression *regexp,
					 int32_t            startIndex,
					 UErrorCode        *status);

UBool uregex_find(URegularExpression *regexp,
				  int32_t             startIndex, 
				  UErrorCode         *status);

int32_t uregex_groupCount(URegularExpression *regexp,
						  UErrorCode         *status);

int32_t uregex_group(URegularExpression *regexp,
					 int32_t             groupNum,
					 UChar              *dest,
					 int32_t             destCapacity,
					 UErrorCode          *status);


UBool uregex_findNext(URegularExpression *regexp,
					  UErrorCode         *status);


int32_t uregex_start(URegularExpression *regexp,
					 int32_t             groupNum,
					 UErrorCode          *status);


int32_t uregex_end(URegularExpression   *regexp,
				   int32_t               groupNum,
				   UErrorCode           *status);

void uregex_reset(URegularExpression    *regexp,
				  int32_t               index,
				  UErrorCode            *status);


int32_t uregex_replaceAll(URegularExpression    *regexp,
						  const UChar           *replacementText,
						  int32_t                replacementLength,
						  UChar                 *destBuf,
						  int32_t                destCapacity,
						  UErrorCode            *status);

int32_t uregex_replaceFirst(URegularExpression  *regexp,
							const UChar         *replacementText,
							int32_t              replacementLength,
							UChar               *destBuf,
							int32_t              destCapacity,
							UErrorCode          *status);

char* u_austrcpy(char *dst,
				 const UChar *src );

int32_t u_strlen(const UChar *s);