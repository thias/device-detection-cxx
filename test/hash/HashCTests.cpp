#include "../../src/common-cxx/tests/pch.h"
#include <string>
#include "../Constants.hpp"
#include "../../src/common-cxx/tests/Base.hpp"
#include "../../src/hash/fiftyone.h"


// User-Agent string of an iPhone mobile device.
const char* mobileUserAgent = (
	"Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
	"AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 Mobile/11D167 "
	"Safari/9537.53");

const char* commonProperties =
	"ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";

using namespace std;

class HashCTests : public Base {
public:
	HashCTests() {
		dataFilePath = "";
		for (int i = 0;
			i < _HashFileNamesLength && strcmp("", dataFilePath.c_str()) == 0;
			i++) {
			dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
		}
	}
protected:
	string dataFilePath;
};

static char* getPropertyValueAsString(
	ResultsHash* results,
	const char* propertyName,
	char *buffer,
	size_t bufferSize) {
	EXCEPTION_CREATE;
	buffer[0] = '\0';
	ResultsHashGetValuesString(
		results,
		propertyName,
		buffer,
		bufferSize,
		",",
		exception);
	EXCEPTION_THROW;
	return buffer;
}

/*
 * Check that the API ResultsHashFromDeviceId returned
 * correct result
 */
TEST_F (HashCTests, ResultsHashFromDeviceIdTest) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	StatusCode status = SUCCESS;
	char deviceId[40] = "";
	char isMobile[40] = "";

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

	// Obtain results from user agent
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;
	EXPECT_EQ(1, resultsUserAgents->count) << "Only one results should be "
		<< "returned.\n";
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsUserAgents,
			"isMobile",
			isMobile,
			sizeof(isMobile)))) << "Property isMobile should be true.\n";

	// Obtain device ID from result
	HashGetDeviceIdFromResults(
		resultsUserAgents,
		(char*)deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;

	// Obtain result again from device ID
	ResultsHashFromDeviceId(
		resultsDeviceId,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;

	memset(isMobile, 0, sizeof(isMobile));
	EXPECT_EQ(1, resultsDeviceId->count) << "Only one results should be "
		<< "returned from detection using device ID.\n";
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsDeviceId,
			"isMobile",
			isMobile,
			sizeof(isMobile)))) << "Property isMobile should be true.\n";

	// Free the results and resource
	ResultsHashFree(resultsUserAgents);
	ResultsHashFree(resultsDeviceId);
	ResourceManagerFree(&manager);
}

/*
 * Check that the API ResultsHashGetValuesString correctly
 * deal with invalid uniqueHttpHeaderIndex for a single result.
 */
TEST_F(HashCTests, ResultsHashGetValuesStringTest) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	StatusCode status = SUCCESS;
	char deviceId[40] = "";
	char isMobile[40] = "";

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

	// Obtain result again from device ID
	// with invalid uniqueHttpHeaderIndex
	resultsDeviceId->items[0].b.uniqueHttpHeaderIndex = -2;
	memset(isMobile, 0, sizeof(isMobile));
	size_t charsAdded = ResultsHashGetValuesString(
		resultsDeviceId,
		"isMobile",
		isMobile,
		sizeof(isMobile),
		",",
		exception);
	EXCEPTION_THROW;
	EXPECT_EQ(0, charsAdded) << "No result should have been found where "
		<< "uniqueHttpHeaderIndex is "
		<< resultsDeviceId->items[0].b.uniqueHttpHeaderIndex
		<< "\n";

	// Obtain result again from device ID
	// with invalid uniqueHttpHeaderIndex
	DataSetHash* dataSet = (DataSetHash*)resultsDeviceId->b.b.dataSet;
	resultsDeviceId->items[0].b.uniqueHttpHeaderIndex =
		dataSet->b.b.uniqueHeaders->count + 1;
	memset(isMobile, 0, sizeof(isMobile));
	charsAdded = ResultsHashGetValuesString(
		resultsDeviceId,
		"isMobile",
		isMobile,
		sizeof(isMobile),
		",",
		exception);
	EXCEPTION_THROW;
	EXPECT_EQ(0, charsAdded) << "No result should have been found where "
		<< "uniqueHttpHeaderIndex is "
		<< resultsDeviceId->items[0].b.uniqueHttpHeaderIndex
		<< "\n";

	// Free the results and resource
	ResultsHashFree(resultsUserAgents);
	ResultsHashFree(resultsDeviceId);
	ResourceManagerFree(&manager);
}

/*
 * Test if the graph trace get API deal with buffer correctly. Check
 * potentially written number of characters are returned even if the buffer
 * does not have enough space.
 */
TEST_F(HashCTests, GraphTraceGetTests) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	configHash.traceRoute = true;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;

	StatusCode status = SUCCESS;
	char isMobile[40] = "";

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);

	// Obtain results from user agent
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;

	// Test if GraphTraceGet returns potentially written number
	char buffer[1] = "";
	// Test with 0
	int potentiallyWritten = GraphTraceGet(
		buffer,
		0,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_TRUE(potentiallyWritten > 0) <<
		"Potentially written number should have been returned.\n";

	// Test with 1
	potentiallyWritten = GraphTraceGet(
		buffer,
		1,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_TRUE(potentiallyWritten > 0) <<
		"Potentially written number should have been returned.\n";

	// Test if GraphTraceGet returns correct written number
	// Add 1 for null character
	int length = potentiallyWritten + 1;
	char* fullBuffer = (char*)Malloc(length);
	EXPECT_TRUE(fullBuffer != NULL) <<
		"Failed to allocate memory for graph trace.\n";

	memset(fullBuffer, 0, length);
	int written = GraphTraceGet(
		fullBuffer,
		length,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_EQ(strlen(fullBuffer), written) <<
		"Failed to write the full graph trace.\n";
	EXPECT_EQ(potentiallyWritten, written) <<
		"Failed to return corrent number of written characters.\n";

	// Free resource
	Free(fullBuffer);
	ResultsHashFree(resultsUserAgents);
	ResourceManagerFree(&manager);
}