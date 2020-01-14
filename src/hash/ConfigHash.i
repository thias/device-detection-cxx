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

%include stdint.i

%include "../ConfigDeviceDetection.i"
%include "../common-cxx/CollectionConfig.i"

%rename(ConfigHashSwig) ConfigHash;

class ConfigHash : public ConfigDeviceDetection {
public:
	ConfigHash();
	void setHighPerformance();
	void setBalanced();
	void setBalancedTemp();
	void setLowMemory();
	void setMaxPerformance();
	void setDrift(int drift);
	void setDifference(int difference);
	void setConcurrency(uint16_t concurrency);
	CollectionConfig getStrings();
	CollectionConfig getProfiles();
	CollectionConfig getDevices();
	CollectionConfig getNodes();
	int getDrift();
	int getDifference();
	uint16_t getConcurrency();
};