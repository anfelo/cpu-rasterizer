#include "renderer.h"
#import <Cocoa/Cocoa.h>

static Renderer gRenderer;

@interface PixelView : NSView
@end

@implementation PixelView

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
  // Internal Resolution: scaled 4x to 800x600
  const int w = 200, h = 150, pixelSize = 4;
  gRenderer = Renderer_Create(w, h, pixelSize);

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
  static float t = 0;
  t += 0.016f;

  Renderer_ClearBackground(&gRenderer, 0xFF1a1a2e);

  // Normalized Device Coordinates (NDC)
  float vertices[] = {
      // Geometry          // Normals
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
      0.5f,  -0.5f, 0.5f, 0.0f,  0.0f,   1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      -0.5f, 0.5f,  0.5f, 0.0f,  0.0f,   1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      -0.5f, -0.5f, 0.5f, -1.0f, 0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.5f,  -0.5f, 0.5f, 1.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
      0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f,  -1.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      -0.5f, 0.5f,  0.5f, 0.0f,  1.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
  };
  Vec3 position = Vec3{60.0f, 40.f, 0.0f};
  Vec3 rotation = Vec3{0.0f, t * 40.f, t * 20.0f};
  Renderer_DrawTriangles(&gRenderer, vertices, 36, 6, position, rotation);

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
