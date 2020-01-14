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

#include "EngineHash.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

#define SUFFIX_LENGTH 20

EngineHash::EngineHash(
	const char *fileName,
	DeviceDetection::Hash::ConfigHash *config,
	RequiredPropertiesConfig *properties) 
	: EngineDeviceDetection(config, properties) {
	EXCEPTION_CREATE;
	StatusCode status = HashInitManagerFromFile(
		manager.get(),
		config->getConfig(),
		properties->getConfig(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status, fileName);
		return;
	}
	EXCEPTION_THROW;
	init();
}

EngineHash::EngineHash(
	const string &fileName,
	DeviceDetection::Hash::ConfigHash *config,
	RequiredPropertiesConfig *properties) 
	: EngineHash(fileName.c_str(), config, properties) {
}

EngineHash::EngineHash(
	void *data,
	long length,
	DeviceDetection::Hash::ConfigHash *config,
	RequiredPropertiesConfig *properties) 
	: EngineDeviceDetection(config, properties) {
	EXCEPTION_CREATE;
	StatusCode status = 
		HashInitManagerFromMemory(
			manager.get(),
			config->getConfig(),
			properties->getConfig(),
			data,
			(size_t)length,
			exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
		return;
	}
	EXCEPTION_THROW;
	init();
}

EngineHash::EngineHash(
	unsigned char data[],
	long length,
	DeviceDetection::Hash::ConfigHash *config,
	RequiredPropertiesConfig *properties)
	: EngineHash((void*)data, length, config, properties) {
}

void EngineHash::init() {
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	init(dataSet);
	DataSetHashRelease(dataSet);
}

void EngineHash::init(fiftyoneDegreesDataSetHash *dataSet) {
	EngineDeviceDetection::init(&dataSet->b);
	initMetaData();
}

void EngineHash::initMetaData() {
	metaData = new MetaDataHash(manager);
}

string EngineHash::getProduct() {
	stringstream stream;
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	appendString(stream, dataSet->strings, dataSet->header.nameOffset);
	DataSetHashRelease(dataSet);
	return stream.str();
}

string EngineHash::getType() {
	return string("HashTrieV34");
}

Date EngineHash::getPublishedTime() {
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	Date date = Date(&dataSet->header.published);
	DataSetHashRelease(dataSet);
	return date;
}

Date EngineHash::getUpdateAvailableTime() {
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	Date date = Date(&dataSet->header.nextUpdate);
	DataSetHashRelease(dataSet);
	return date;
}

string EngineHash::getDataFilePath() {
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	string path = string(dataSet->b.b.masterFileName);
	DataSetHashRelease(dataSet);
	return path;
}

string EngineHash::getDataFileTempPath() {
	string path;
	DataSetHash *dataSet = DataSetHashGet(manager.get());
	if (strcmp(
		dataSet->b.b.masterFileName,
		dataSet->b.b.fileName) == 0) {
		path = string("");
	}
	else {
		path = string(dataSet->b.b.fileName);
	}
	DataSetHashRelease(dataSet);
	return path;
}

void EngineHash::refreshData() {
	EXCEPTION_CREATE;
	StatusCode status = HashReloadManagerFromOriginalFile(
		manager.get(),
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
		return;
	}
	EXCEPTION_THROW;
}

void EngineHash::refreshData(const char *fileName) {
	EXCEPTION_CREATE;
	StatusCode status = HashReloadManagerFromFile(
		manager.get(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
		return;
	}
	EXCEPTION_THROW;
}

void EngineHash::refreshData(void *data, long length) {
	EXCEPTION_CREATE;
	StatusCode status = HashReloadManagerFromMemory(
		manager.get(),
		data,
		length,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
		return;
	}
	EXCEPTION_THROW;
}

void EngineHash::refreshData(
	unsigned char data[], 
	long length) {
	refreshData((void*)data, length);
}

DeviceDetection::Hash::ResultsHash* EngineHash::process(
	DeviceDetection::EvidenceDeviceDetection *evidence,
	int drift,
	int difference) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsHash *results = ResultsHashCreate(
		manager.get(),
		size,
		size);
	results->drift = drift;
	results->difference = difference;
	ResultsHashFromEvidence(
		results,
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;
	// TODO override properties/profiles.
	return new ResultsHash(results, manager);
}

DeviceDetection::Hash::ResultsHash* EngineHash::process(
	const char *userAgent,
	int drift,
	int difference) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsHash *results = ResultsHashCreate(
		manager.get(),
		1,
		0);
	results->drift = drift;
	results->difference = difference;
	ResultsHashFromUserAgent(
		results, 
		userAgent, 
		userAgent == nullptr ? 0 : (int)strlen(userAgent),
		exception);
	EXCEPTION_THROW;
	// TODO override properties/profiles.
	return new ResultsHash(results, manager);
}

DeviceDetection::Hash::ResultsHash* EngineHash::process(
	DeviceDetection::EvidenceDeviceDetection *evidence) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsHash *results = ResultsHashCreate(
		manager.get(),
		size,
		size);
	ResultsHashFromEvidence(
		results,
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;
	// TODO override properties/profiles.
	return new ResultsHash(results, manager);
}

DeviceDetection::Hash::ResultsHash* EngineHash::process(
	const char *userAgent) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsHash *results = ResultsHashCreate(
		manager.get(),
		1,
		0);
	ResultsHashFromUserAgent(
		results,
		userAgent,
		userAgent == nullptr ? 0 : (int)strlen(userAgent),
		exception);
	EXCEPTION_THROW;
	// TODO override properties/profiles.
	return new ResultsHash(results, manager);
}

ResultsBase* EngineHash::processBase(
	EvidenceBase *evidence) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsHash *results = ResultsHashCreate(
		manager.get(),
		size,
		size);
	ResultsHashFromEvidence(
		results,
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;
	return new ResultsHash(results, manager);
}

DeviceDetection::ResultsDeviceDetection* EngineHash::processDeviceDetection(
	DeviceDetection::EvidenceDeviceDetection *evidence) {
	return process(evidence);
}

DeviceDetection::ResultsDeviceDetection* EngineHash::processDeviceDetection(
	const char *userAgent) {
	return process(userAgent);
}