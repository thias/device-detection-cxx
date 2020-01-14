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

#ifdef _DEBUG
#define PASSES 1
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#else
#include "dmalloc.h"
#endif
#else
#define PASSES 5
#endif

#include "../../../src/pattern/pattern.h"
#include "../../../src/pattern/fiftyone.h"
#include "../.././../src/common-cxx/textfile.h"

// Size of the character buffers
#define BUFFER 1000

// Number of marks to make when showing progress.
#define PROGRESS_MARKS 40

// Number of threads to start for performance analysis.
#define THREAD_COUNT 4

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV3.2.dat";

static const char *userAgentFileName = "20000 User Agents.csv";

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

// #define CONFIG fiftyoneDegreesPatternInMemoryConfig
// #define CONFIG fiftyoneDegreesPatternHighPerformanceConfig
#define CONFIG fiftyoneDegreesPatternLowMemoryConfig
// #define CONFIG fiftyoneDegreesPatternBalancedConfig
// #define CONFIG fiftyoneDegreesPatternBalancedTempConfig

/**
 * Shared test state. All members are immutable except runningThreads
 * which is updated via an interlocked compare and exchange.
 */
typedef struct t_memory_state {
	int userAgentsCount; // Total number of User-Agents
	int progress; // Number of User-Agents to process for each = marker
	const char *userAgentFilePath; // Filename for the User-Agent file
	int numberOfThreads; // Number of parallel threads
	fiftyoneDegreesResourceManager *manager; // Manager resource for detection
	volatile long runningThreads; // Number of active running threads
	FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT]; // Running threads
} memoryState;

/**
 * Individual thread state where all members are exclusively accessed by a 
 * single test thread.
 */
typedef struct t_thread_memory_state {
	memoryState *main; // Reference to the main threads shared state
	long count; // Number of User-Agents the thread has processed
	bool reportProgress; // True if this thread should report progress
	fiftyoneDegreesResultsPattern *results; // The results used by the thread
} memoryThreadState;

/**
 * Prints the progress bar based on the state provided.
 * @param state information about the overall performance test
 */
void printLoadBar(memoryThreadState *state) {
	int i;
	int full = state->count / state->main->progress;
	int empty = (state->main->userAgentsCount - state->count) /
		state->main->progress;
	printf("\r\t[");
	for (i = 0; i < full; i++) {
		printf("=");
	}

	for (i = 0; i < empty; i++) {
		printf(" ");
	}
	printf("]");
}

/**
 * Reports progress using the required property index specified.
 * @param state of the performance test
 */
void reportProgress(memoryThreadState *state) {
	EXCEPTION_CREATE;
	char deviceId[40] = "";

	// Update the user interface.
	printLoadBar(state);

	// If in real detection mode then print the id of the device found
	// to prove it's actually doing something!
	if (state->results != NULL) {
		printf(" ");
		PatternGetDeviceIdFromResults(
			state->results,
			deviceId,
			sizeof(deviceId),
			exception);
		EXCEPTION_THROW;
		printf("%s", deviceId);
	}
}

/**
 * Runs the test for the User-Agent provided. Called from the text file 
 * iterator.
 * @param userAgent to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char *userAgent, void *state) {
	EXCEPTION_CREATE;
	memoryThreadState *threadState = (memoryThreadState*)state;

	// If not calibrating the test environment perform device detection.
	ResultsPatternFromUserAgent(
		threadState->results,
		userAgent,
		strlen(userAgent),
		exception);

	// Increase the count for this performance thread and update the user
	// interface if a progress mark should be written.
	threadState->count++;
	if (threadState->reportProgress == true &&
		threadState->count % threadState->main->progress == 0) {
		reportProgress(threadState);
	}
}

/**
 * A single threaded memory test. Many of these will run in parallel to
 * ensure the single managed resource is being used.
 * @param mainState state information about the main test
 */
static void runMemoryThread(void* mainState) {
	const char userAgent[BUFFER] = "";
	memoryThreadState threadState;
	threadState.main = (memoryState*)mainState;

	// Ensure that only one thread reports progress. Avoids keeping a running
	// total and synchronising performance test threads.
	long initialRunning = threadState.main->runningThreads;
	if (ThreadingGetIsThreadSafe()) {
		threadState.reportProgress = INTERLOCK_EXCHANGE(
			threadState.main->runningThreads,
			initialRunning + 1,
			initialRunning) == threadState.main->numberOfThreads - 1;
	}
	else {
		threadState.reportProgress = 1;
	}
	threadState.count = 0;

	// Create an instance of results to access the returned values.
	threadState.results = ResultsPatternCreate(
		threadState.main->manager,
		1,
		0);

	// Execute the device detection test.
	TextFileIterate(
		threadState.main->userAgentFilePath,
		userAgent,
		BUFFER,
		&threadState,
		executeTest);

	// Free the memory used by the results instance.
	ResultsPatternFree(threadState.results);

	if (ThreadingGetIsThreadSafe()) {
		THREAD_EXIT;
	}
}

/**
 * Execute memory tests in parallel using a file of null terminated
 * User-Agent strings as input. If calibrate is true then the file is read but
 * no detections are performed.
 */
static void runMemoryTests(memoryState *state) {
	int thread;
	state->runningThreads = 0;
	if (ThreadingGetIsThreadSafe()) {

		// Create and start the threads.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_CREATE(
				state->threads[thread],
				(THREAD_ROUTINE)&runMemoryThread,
				state);
		}

		// Wait for them to finish.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_JOIN(state->threads[thread]);
			THREAD_CLOSE(state->threads[thread]);
		}
	}
	else {
		runMemoryThread(state);
	}

	printf("\n\n");
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100) 
#endif
/**
 * Increments the state counter to work out the number of User-Agents.
 */
static void userAgentCount(const char *userAgent, void *state) {
	(*(int*)state)++;
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

/**
 * Iterate the User-Agent source file and count the number of lines it
 * contains. Used to determine progress during the test.
 * @param userAgentFilePath
 * @return number of lines the file contains
 */
static int getUserAgentsCount(const char *userAgentFilePath) {
	int count = 0;
	char userAgent[BUFFER];
	TextFileIterate(
		userAgentFilePath,
		userAgent,
		BUFFER,
		&count,
		userAgentCount);
	return count;
}

/**
 * Sets the test state and then runs calibration, and device detection.
 * @param manager initialised manager to use for the tests
 * @param userAgentFilePath path to the User-Agents file to use for testing.
 */
void run(
	fiftyoneDegreesResourceManager *manager,
	const char *userAgentFilePath) {
	memoryState state;

	// Set the file name and manager and number of threads.
	state.userAgentFilePath = userAgentFilePath;
	state.manager = manager;
	state.numberOfThreads = THREAD_COUNT;

	// Count the number of User-Agents in the source file.
	state.userAgentsCount = getUserAgentsCount(userAgentFilePath);

	// Run the test once as the amount of memory used won't vary.
	// Set the progress indicator.
	state.progress = state.userAgentsCount / PROGRESS_MARKS;
	runMemoryTests(&state);
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	fiftyoneDegreesStatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

/**
 * Run the memory test from either the tests or the main method.
 * @param dataFilePath full file path to the Pattern device data file
 * @param userAgentFilePath full file path to the User-Agent test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesMemPatternRun(
	const char *dataFilePath,
	const char *userAgentFilePath,
	fiftyoneDegreesConfigPattern config) {
	EXCEPTION_CREATE;

	// Ensure the tracking malloc and free methods are used and the counters
	// reset.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	Free = MemoryTrackingFree;

	// Set concurrency to ensure sufficient shared resources available.
	config.components.concurrency =
		config.maps.concurrency =
		config.nodeRankedSignatureIndexes.concurrency =
		config.nodes.concurrency =
		config.profileOffsets.concurrency =
		config.profiles.concurrency =
		config.properties.concurrency =
		config.rankedSignatureIndexes.concurrency =
		config.rootNodes.concurrency =
		config.signatureNodeOffsets.concurrency =
		config.signatures.concurrency = 
		config.strings.concurrency =
		config.values.concurrency = THREAD_COUNT;

	config.userAgentCacheCapacity = 0;
	config.userAgentCacheConcurrency = 0;

	ResourceManager manager;
	StatusCode status = PatternInitManagerFromFile(
		&manager,
		&config,
		NULL,
		dataFilePath,
		exception);
	
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Run the performance tests.
		run(&manager, userAgentFilePath);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
		
		// Verify that the currently allocated memory is zero.
		assert(MemoryTrackingGetAllocated() == 0);

		// Report the maximum memory usage.
		printf("Maximum allocated memory %.2fMBs",
			(double)MemoryTrackingGetMax() / 
			(double)(1024 * 1024));
	}

	// Ensure the standard malloc and free methods are reinstated now the
	// tracking has finished.
	Malloc = MemoryStandardMalloc;
	Free = MemoryStandardFree;
	MemoryTrackingReset();
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 User-Agent file path
 * @arg3 required properties
 */
int main(int argc, char* argv[]) {
	
	printf("\n");
	printf("\t#############################################################\n");
	printf("\t#                                                           #\n");
	printf("\t#   This program can be used to test the memory usage of    #\n");
	printf("\t#              the 51Degrees 'Pattern' C API.               #\n");
	printf("\t#                                                           #\n");
	printf("\t#   The test will process a list of User Agents and output  #\n");
	printf("\t# the peak memory usage as measured by the 51Degrees memory #\n");
	printf("\t#    allocation tracker. As the tracker slows down memory   #\n");
	printf("\t#     allocation and free operations this example is not    #\n");
	printf("\t#  representative of real world performance. Use PerfPat to #\n");
	printf("\t#                   measure performance.                    #\n");
	printf("\t#                                                           #\n");
	printf("\t#  Command line arguments should be a Pattern format data   #\n");
	printf("\t#   file and a CSV file containing a list of User-Agents.   #\n");
	printf("\t#      A test file of 1 million can be downloaded from      #\n");
	printf("\t#            http://51degrees.com/million.zip               #\n");
	printf("\t#                                                           #\n");
	printf("\t#############################################################\n");

	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char userAgentFilePath[FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}
	if (argc > 2) {
		strcpy(userAgentFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			userAgentFileName,
			userAgentFilePath,
			sizeof(userAgentFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, userAgentFilePath);
		fgetc(stdin);
		return 1;
	}

	// Report the files that are being used with the performance test.
	printf("\n\nUser-Agents file is: %s\n\nData file is: %s\n\n",
		FileGetFileName(userAgentFilePath),
		FileGetFileName(dataFilePath));

	// Wait for a character to be pressed.
	printf("\nPress enter to start memory test.\n");
	fgetc(stdin);

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	// Run the performance test.
	fiftyoneDegreesMemPatternRun(
		dataFilePath,
		userAgentFilePath,
		CONFIG);
	
#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif