let platform_utils;

if (process.platform === 'darwin') {
  platform_utils = require("bindings")("mac_utils.node");
} else if (process.platform === 'win32') {
  platform_utils = require("bindings")("win_utils.node");
} else {
  throw new Error('Unsupported platform');
}

module.exports = {
  // Common exports that work on all platforms
  getRunningInputAudioProcesses: platform_utils.getRunningInputAudioProcesses,
  INFO_ERROR_CODE: 1,
  ERROR_DOMAIN: "com.MicrophoneUsageMonitor",
  
  // Mac-specific exports
  ...(process.platform === 'darwin' ? {
    makeKeyAndOrderFront: platform_utils.makeKeyAndOrderFront,
    startMonitoringMic: platform_utils.startMonitoringMic,
    stopMonitoringMic: platform_utils.stopMonitoringMic,
  } : {})
};
