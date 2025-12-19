#import <Cocoa/Cocoa.h>
#include "renderer.h"

static Renderer gRenderer;

@interface PixelView : NSView
@end

@implementation PixelView

- (void)drawRect:(NSRect)dirtyRect {
    if (!gRenderer.ready) return;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
        gRenderer.pixels, gRenderer.width, gRenderer.height, 8,
        gRenderer.width * 4, colorSpace,
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
    // Internal Resolution: scaled 4x to 800x600
    const int w = 200, h = 150, pixelSize = 4;
    gRenderer = Renderer_Create(w, h, pixelSize);

    NSRect frame = NSMakeRect(
        100,
        100,
        gRenderer.windowWidth,
        gRenderer.windowHeight
    );
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

    Renderer_ClearBackground(&gRenderer, 0xFF1a1a2e);
    // int cx = 400 + sin(t) * 150;
    // int cy = 300 + cos(t * 0.8f) * 100;
    // gRenderer->drawCircle(cx, cy, 40, 0xFFe94560);
    // gRenderer->drawCircle(400, 300, 20, 0xFF0f4c75);

    // float vertices[] = {100.0f, 40.0f, 60.0f, 80.0f, 150.0f, 120.0f};
    // for (int i = 0; i < 6; i+=2) {
    //   vertices[i] = vertices[i]*cos(t) - vertices[i+1]*sin(t);
    //   vertices[i+1] = vertices[i]*sin(t) + vertices[i+1]*cos(t);
    // }
    float vertices[] = {60.0f, 40.0f, 120.0f, 40.0f, 60.0f, 100.0f, 120.0f, 100.0f};
    Renderer_DrawTriangles(&gRenderer, vertices, 8, 0xFF0000);

    [view setNeedsDisplay:YES];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    Renderer_Destroy(&gRenderer);
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
