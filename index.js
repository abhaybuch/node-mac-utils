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
  
  // Mac-specific exports
  ...(process.platform === 'darwin' ? {
    makeKeyAndOrderFront: platform_utils.makeKeyAndOrderFront
  } : {})
};
