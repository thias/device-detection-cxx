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

#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_BUILDER_PATTERN_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_BUILDER_PATTERN_HPP

#include <vector>
#include "../common-cxx/ValueMetaData.hpp"
#include "../common-cxx/EntityMetaDataBuilder.hpp"
#include "pattern.h"

using namespace std;

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Pattern {
			/**
			 * Meta data builder class contains static helper methods used when
			 * building value meta data instances from a Pattern data set.
			 */
			class ValueMetaDataBuilderPattern : EntityMetaDataBuilder {
			public:
				/**
				 * Build a new instance of ValueMetaData from the underlying
				 * value provided. The instance returned does not hold a
				 * reference to the data set or value, and contains a copy of
				 * all meta data.
				 * @param dataSet pointer to the data set containing the
				 * component
				 * @param value pointer to the underlying value to create the
				 * meta data from
				 */
				static ValueMetaData* build(
					fiftyoneDegreesDataSetPattern *dataSet,
					fiftyoneDegreesValue *value);
			};
		}
	}
}

#endif