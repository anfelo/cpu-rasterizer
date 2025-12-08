#import <Cocoa/Cocoa.h>
#include "renderer.h"

static Renderer *gRenderer = nullptr;

@interface PixelView : NSView
@end

@implementation PixelView

- (void)drawRect:(NSRect)dirtyRect {
    if (!gRenderer) return;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
        gRenderer->pixels, gRenderer->width, gRenderer->height, 8,
        gRenderer->width * 4, colorSpace,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little
    );

    CGImageRef image = CGBitmapContextCreateImage(ctx);
    CGContextDrawImage([[NSGraphicsContext currentContext] CGContext],
                       dirtyRect, image);

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
    const int W = 800, H = 600;
    gRenderer = new Renderer(W, H);

    NSRect frame = NSMakeRect(100, 100, W, H);
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
    timer = [NSTimer scheduledTimerWithTimeInterval:1.0/60.0
                                             target:self
                                           selector:@selector(tick)
                                           userInfo:nil
                                            repeats:YES];
}

- (void)tick {
    static float t = 0;
    t += 0.016f;

    // All rendering in pure C++
    gRenderer->clear(0xFF1a1a2e);
    gRenderer->fillRect(50, 50, 200, 100, 0xFF16213e);

    int cx = 400 + sin(t) * 150;
    int cy = 300 + cos(t * 0.8f) * 100;
    gRenderer->drawCircle(cx, cy, 40, 0xFFe94560);
    gRenderer->drawCircle(400, 300, 20, 0xFF0f4c75);

    [view setNeedsDisplay:YES];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    delete gRenderer;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
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
