#include "camera.h"
#include "math.h"
#include "renderer.h"
#import <Cocoa/Cocoa.h>
#include <mach/mach_time.h>

static double frameTimes[60];
static int frameIndex = 0;
// Call once at startup to get the conversion factor
static mach_timebase_info_data_t timebaseInfo;
static void InitTiming() { mach_timebase_info(&timebaseInfo); }

// Returns elapsed time in milliseconds
static double GetElapsedMs(uint64_t start, uint64_t end) {
  uint64_t elapsed = end - start;
  return (double)elapsed * timebaseInfo.numer / timebaseInfo.denom / 1e6;
}

static Renderer gRenderer;

static struct {
  BOOL w, a, s, d;
  BOOL space, shift;
} keyState = {0};

@interface PixelView : NSView
@end

@implementation PixelView

- (BOOL)acceptsFirstResponder {
  return YES; // Required to receive keyboard events
}

- (void)keyDown:(NSEvent *)event {
  // Prevent beeping on key press
  if (event.isARepeat)
    return;

  NSString *chars = [event charactersIgnoringModifiers];
  if (chars.length == 0)
    return;

  unichar key = [chars characterAtIndex:0];
  switch (key) {
  case 'w':
  case 'W':
    keyState.w = YES;
    break;
  case 'a':
  case 'A':
    keyState.a = YES;
    break;
  case 's':
  case 'S':
    keyState.s = YES;
    break;
  case 'd':
  case 'D':
    keyState.d = YES;
    break;
  case ' ':
    keyState.space = YES;
    break;
  }

  // Check for shift
  if (event.modifierFlags & NSEventModifierFlagShift) {
    keyState.shift = YES;
  }
}

- (void)keyUp:(NSEvent *)event {
  NSString *chars = [event charactersIgnoringModifiers];
  if (chars.length == 0)
    return;

  unichar key = [chars characterAtIndex:0];
  switch (key) {
  case 'w':
  case 'W':
    keyState.w = NO;
    break;
  case 'a':
  case 'A':
    keyState.a = NO;
    break;
  case 's':
  case 'S':
    keyState.s = NO;
    break;
  case 'd':
  case 'D':
    keyState.d = NO;
    break;
  case ' ':
    keyState.space = NO;
    break;
  }

  if (!(event.modifierFlags & NSEventModifierFlagShift)) {
    keyState.shift = NO;
  }
}

- (void)flagsChanged:(NSEvent *)event {
  keyState.shift = (event.modifierFlags & NSEventModifierFlagShift) != 0;
}

- (void)drawRect:(NSRect)dirtyRect {
  if (!gRenderer.ready)
    return;

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
      gRenderer.pixels, gRenderer.width, gRenderer.height, 8,
      gRenderer.width * 4, colorSpace,
      kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

  CGImageRef image = CGBitmapContextCreateImage(ctx);
  CGContextDrawImage([[NSGraphicsContext currentContext] CGContext], dirtyRect,
                     image);

  CGImageRelease(image);
  CGContextRelease(ctx);
  CGColorSpaceRelease(colorSpace);
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  NSWindow *window;
  PixelView *view;
  NSTimer *timer;
}
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  InitTiming();

  // Internal Resolution: scaled 4x to 800x600
  // const int w = 800, h = 600, pixelSize = 1;
  const int w = 200, h = 150, pixelSize = 4;

  gRenderer = Renderer_Create(w, h, pixelSize);
  gRenderer.camera =
      Camera_Create(Vec3{0.0f, 0.0f, 2.0f}, Vec3{0.0f, 1.0f, 0.0f}, YAW, PITCH);

  NSRect frame =
      NSMakeRect(100, 100, gRenderer.windowWidth, gRenderer.windowHeight);
  window = [[NSWindow alloc]
      initWithContentRect:frame
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                  backing:NSBackingStoreBuffered
                    defer:NO];

  view = [[PixelView alloc] initWithFrame:frame];
  [window setContentView:view];
  [window setTitle:@"CPU Rasterizer"];
  [window makeKeyAndOrderFront:nil];

  // Start render loop
  timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
                                           target:self
                                         selector:@selector(tick)
                                         userInfo:nil
                                          repeats:YES];
}

- (void)tick {
  float deltaTime = 0.016f;
  // static float t = 0;
  // t += deltaTime;

  // Process camera movement
  if (keyState.w) {
    Camera_ProcessKeyboard(&gRenderer.camera, FORWARD, deltaTime);
  }
  if (keyState.s) {
    Camera_ProcessKeyboard(&gRenderer.camera, BACKWARD, deltaTime);
  }
  if (keyState.a) {
    Camera_ProcessKeyboard(&gRenderer.camera, LEFT, deltaTime);
  }
  if (keyState.d) {
    Camera_ProcessKeyboard(&gRenderer.camera, RIGHT, deltaTime);
  }
  if (keyState.space) {
    gRenderer.camera.position.y += deltaTime * 2.5f; // Move up
  }
  if (keyState.shift) {
    gRenderer.camera.position.y -= deltaTime * 2.5f; // Move down
  }

  uint64_t start = mach_absolute_time();

  // Renderer
  Renderer_ClearBackground(&gRenderer, 0x101010);

  Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
  Vec3 rotation = Vec3{0.0f, 0.0f, 0.0f};
  // Vec3 rotation = Vec3{0.0f, t * 40.f, t * 20.0f};
  Vec3 scale = Vec3{1.0f, 1.0f, 1.0f};
  ColorRGBA color = ColorRGBA{1.0f, 0.5f, 0.31f};
  Renderer_DrawCube(&gRenderer, position, rotation, scale, color);

  uint64_t end = mach_absolute_time();

  frameTimes[frameIndex] = GetElapsedMs(start, end);
  frameIndex = (frameIndex + 1) % 60;

  if (frameIndex == 0) {
    double avg = 0;
    for (int i = 0; i < 60; i++)
      avg += frameTimes[i];
    avg /= 60.0;
    printf("Avg render: %.3f ms (%.1f FPS)\n", avg, 1000.0 / avg);
  }

  [view setNeedsDisplay:YES];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
  Renderer_Destroy(&gRenderer);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}

@end

int main() {
  @autoreleasepool {
    NSApplication *app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    AppDelegate *delegate = [[AppDelegate alloc] init];
    [app setDelegate:delegate];
    [app activateIgnoringOtherApps:YES];
    [app run];
  }
  return 0;
}
