// Import the package
const utils = require('./index.js');

// Test function to run all checks
async function runTests() {
    console.log('Running on platform:', process.platform);
    
    try {
        // Test getRunningInputAudioProcesses
        console.log('\nTesting getRunningInputAudioProcesses:');
        const processes = utils.getRunningInputAudioProcesses();
        console.log('Audio processes:', processes);

        // Test platform-specific functions
        if (process.platform === 'darwin') {
            console.log('\nTesting Mac-specific functions:');
            console.log('makeKeyAndOrderFront available:', !!utils.makeKeyAndOrderFront);
        } else if (process.platform === 'win32') {
            console.log('\nTesting Windows-specific functions:');
            // Add any Windows-specific function tests here
        } else {
            console.log('Unsupported platform');
        }

        // Log all available exports
        console.log('\nAll available exports:');
        console.log(Object.keys(utils));

    } catch (error) {
        console.error('Test failed:', error);
        process.exit(1);
    }
}

// Run the tests
runTests(); 