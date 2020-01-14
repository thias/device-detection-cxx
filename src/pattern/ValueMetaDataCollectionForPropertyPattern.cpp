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

#include "ValueMetaDataCollectionForPropertyPattern.hpp"
#include "../common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Pattern;

ValueMetaDataCollectionForPropertyPattern::ValueMetaDataCollectionForPropertyPattern(
	fiftyoneDegreesResourceManager *manager,
	PropertyMetaData *property) : ValueMetaDataCollectionBasePattern(manager) {
	EXCEPTION_CREATE;
	DataReset(&propertyItem.data);
	PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		property->getName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForPropertyPattern::~ValueMetaDataCollectionForPropertyPattern() {
	COLLECTION_RELEASE(dataSet->properties, &propertyItem);
}

ValueMetaData* ValueMetaDataCollectionForPropertyPattern::getByIndex(
	uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	Value *value;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	value = ValueGet(
		dataSet->values, 
		getProperty()->firstValueIndex + index, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderPattern::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

ValueMetaData* ValueMetaDataCollectionForPropertyPattern::getByKey(
	ValueMetaDataKey key) {
	EXCEPTION_CREATE;
	Item item;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	String *name = PropertyGetName(
		dataSet->strings, 
		getProperty(), 
		&item, 
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		result = ValueMetaDataCollectionBasePattern::getByKey(key);
		if (strcmp(
			result->getKey().getPropertyName().c_str(), 
			&name->value) != 0) {
			delete result;
			result = nullptr;
		}
		dataSet->strings->release(&item);
	}
	return result;
}

uint32_t ValueMetaDataCollectionForPropertyPattern::getSize() {
	return getProperty()->lastValueIndex - getProperty()->firstValueIndex + 1;
}

fiftyoneDegreesProperty* ValueMetaDataCollectionForPropertyPattern::getProperty() {
	return (Property*)propertyItem.data.ptr;
}