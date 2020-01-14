/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 * 
 * If using the Work as, or as part of, a network application, by 
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading, 
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "ResultsPattern.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees;

#define RESULT(r,i) ((ResultPattern*)r->b.b.items + i)
#define SIGNATURE(r,i) ((fiftyoneDegreesSignature*) \
	RESULT(r, i)->signature.data.ptr)

DeviceDetection::Pattern::ResultsPattern::ResultsPattern(
	fiftyoneDegreesResultsPattern *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: ResultsDeviceDetection(&results->b, manager) {
	this->results = results;
	_jsHardwareProfileRequiredIndex =
		PropertiesGetRequiredPropertyIndexFromName(
			this->available,
			"javascripthardwareprofile");
}


DeviceDetection::Pattern::ResultsPattern::~ResultsPattern() {
	ResultsPatternFree(results);
}

void DeviceDetection::Pattern::ResultsPattern::getValuesInternal(
	int requiredPropertyIndex,
	vector<string> &values) {
	EXCEPTION_CREATE;
	uint32_t i;
	Item *valuesItems;
	String *valueName;
	
	// Get a pointer to the first value item for the property.
	valuesItems = ResultsPatternGetValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;

	if (valuesItems == NULL) {
		// No pointer to values was returned. 
		throw NoValuesAvailableException();
	}

	// Set enough space in the vector for all the strings that will be 
	// inserted.
	values.reserve(results->values.count);

	if (_jsHardwareProfileRequiredIndex >= 0 &&
		(int)requiredPropertyIndex == _jsHardwareProfileRequiredIndex) {

		// Add the values as JavaScript snippets to the result.
		for (i = 0; i < results->values.count; i++) {
			valueName = (String*)valuesItems[i].data.ptr;
			if (valueName != nullptr) {
				stringstream stream;
				stream <<
					"var profileIds = []\n" <<
					STRING(valueName) <<
					"\ndocument.cookie = \"51D_ProfileIds=\" + " <<
					"profileIds.join(\"|\")";
				values.push_back(stream.str());
			}
		}
	}
	else {

		// Add the values in their original form to the result.
		for (i = 0; i < results->values.count; i++) {
			valueName = (String*)valuesItems[i].data.ptr;
			if (valueName != nullptr)
			{
				values.push_back(string(STRING(valueName)));
			}
		}
	}
}

bool DeviceDetection::Pattern::ResultsPattern::hasValuesInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	bool hasValues = fiftyoneDegreesResultsPatternGetHasValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;
	return hasValues;
}

const char* DeviceDetection::Pattern::ResultsPattern::getNoValueMessageInternal(
	fiftyoneDegreesResultsNoValueReason reason) {
	return fiftyoneDegreesResultsPatternGetNoValueReasonMessage(reason);
}

fiftyoneDegreesResultsNoValueReason
DeviceDetection::Pattern::ResultsPattern::getNoValueReasonInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsNoValueReason reason =
		fiftyoneDegreesResultsPatternGetNoValueReason(
			results,
			requiredPropertyIndex,
			exception);
	EXCEPTION_THROW;
	return reason;
}

string DeviceDetection::Pattern::ResultsPattern::getDeviceId(
	uint32_t resultIndex) {
	EXCEPTION_CREATE;
	char deviceId[50] = "";
	if (resultIndex < results->count) {
		PatternGetDeviceIdFromResult(
			(DataSetPattern*)results->b.b.dataSet,
			&results->items[resultIndex],
			deviceId,
			sizeof(deviceId),
			exception);
		EXCEPTION_THROW;
	}
	return string(deviceId);
}

string DeviceDetection::Pattern::ResultsPattern::getDeviceId() {
	EXCEPTION_CREATE;
	char deviceId[50] = "";
	PatternGetDeviceIdFromResults(
		results,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;
	return string(deviceId);
}

int DeviceDetection::Pattern::ResultsPattern::getRank(uint32_t resultIndex) {
	if (resultIndex < results->count) {
		return results->items[resultIndex].rank;
	}
	return 0;
}

int DeviceDetection::Pattern::ResultsPattern::getRank() {
	uint32_t i;
	int rank = getRank(0),
		nextRank;
	for (i = 1; i < results->count; i++) {
		nextRank = getRank(i);
		if (nextRank > rank) {
			rank = nextRank;
		}
	}
	return rank;
}

int DeviceDetection::Pattern::ResultsPattern::getDifference(
	uint32_t resultIndex) {
	return results->items[resultIndex].difference;
}

int DeviceDetection::Pattern::ResultsPattern::getDifference() {
	uint32_t i;
	int difference = 0;
	for (i = 0; i < results->count; i++) {
		difference += getDifference(i);
	}
	return difference;
}

int DeviceDetection::Pattern::ResultsPattern::getSignaturesCompared(
	uint32_t resultIndex) {
	return results->items[resultIndex].signaturesCompared;
}

int DeviceDetection::Pattern::ResultsPattern::getSignaturesCompared() {
	uint32_t i;
	int signaturesCompared = 0;
	for (i = 0; i < results->count; i++) {
		signaturesCompared += getSignaturesCompared(i);
	}
	return signaturesCompared;
}

int DeviceDetection::Pattern::ResultsPattern::getMethod(uint32_t resultIndex) {
	if (resultIndex < results->count) {
		return results->items[resultIndex].method;
	}
	return 0;
}

int DeviceDetection::Pattern::ResultsPattern::getMethod() {
	uint32_t i;
	int method = getMethod(0),
		nextMethod;

	for (i = 1; i < results->count; i++) {
		nextMethod = getMethod(i);
		if (nextMethod > method) {
			method = nextMethod;
		}
	}
	return method;
}

int DeviceDetection::Pattern::ResultsPattern::getUserAgents() {
	return results->count;
}

string DeviceDetection::Pattern::ResultsPattern::getUserAgent(
	int resultIndex) {
	string userAgent;
	if (resultIndex >= 0 && (uint32_t)resultIndex < results->count) {
		if (results->items[resultIndex].b.matchedUserAgent != NULL) {
			userAgent.assign(results->items[resultIndex].b.matchedUserAgent);
		}
	}
	return userAgent;
}