#import "AudioProcessMonitor.h"

@interface AudioProcessMonitor ()
@end

@implementation AudioProcessMonitor

- (instancetype)init {
  self = [super init];
  if (self) {
  }
  return self;
}

+ (NSArray<NSString *> *)getRunningInputAudioProcesses:(NSError **)error {
  AudioObjectPropertyAddress address = {
      kAudioHardwarePropertyProcessObjectList,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMain
  };

  UInt32 dataSize = 0;
  OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                 &address,
                                                 0,
                                                 NULL,
                                                 &dataSize);

  if (status != noErr) {
      if (error) {
          *error = [NSError errorWithDomain:@"AudioProcessMonitor"
                                     code:status
                                 userInfo:@{NSLocalizedDescriptionKey: @"Failed to get process list size"}];
      }
      return @[];
  }

  NSInteger count = dataSize / sizeof(AudioObjectID);
  AudioObjectID *processIDs = malloc(dataSize);

  if (!processIDs) {
    if (error) {
        *error = [NSError errorWithDomain:@"AudioProcessMonitor"
                                   code:-1
                               userInfo:@{NSLocalizedDescriptionKey: @"Failed to allocate memory"}];
    }
    return @[];
  }

  status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                    &address,
                                    0,
                                    NULL,
                                    &dataSize,
                                    processIDs);

  if (status != noErr) {
    free(processIDs);
    if (error) {
        *error = [NSError errorWithDomain:@"AudioProcessMonitor"
                                   code:status
                               userInfo:@{NSLocalizedDescriptionKey: @"Failed to get process list"}];
    }
    return @[];
  }

  NSMutableArray<NSString *> *activeBundleIDs = [NSMutableArray array];

  for (NSInteger i = 0; i < count; i++) {
    AudioObjectID processID = processIDs[i];

    // Check if process is running
    address.mSelector = kAudioProcessPropertyIsRunningInput;
    UInt32 isRunning = 0;
    dataSize = sizeof(isRunning);

    status = AudioObjectGetPropertyData(processID,
                                      &address,
                                      0,
                                      NULL,
                                      &dataSize,
                                      &isRunning);

    if (status != noErr || !isRunning) {
        continue;
    }

    // Get bundle ID
    address.mSelector = kAudioProcessPropertyBundleID;
    CFStringRef bundleIDRef = NULL;
    dataSize = sizeof(bundleIDRef);

    status = AudioObjectGetPropertyData(processID,
                                      &address,
                                      0,
                                      NULL,
                                      &dataSize,
                                      &bundleIDRef);

    if (status == noErr && bundleIDRef) {
        NSString *bundleID = (NSString *)CFBridgingRelease(bundleIDRef);
        if (bundleID.length > 0) {
            [activeBundleIDs addObject:bundleID];
        }
    }
  }

  free(processIDs);
  return [activeBundleIDs copy];
}

@end 
