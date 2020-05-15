//
//  AppDelegate.m
//  BlueWindow
//
//  Created by sugat mankar on 21/02/20.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "GLESView.h"

@implementation AppDelegate
{
@private
    UIWindow *mainWindow;
    ViewController *mainViewController;
    GLESView *myView;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    CGRect screenBounds = [[UIScreen mainScreen]bounds];
    mainWindow = [[UIWindow alloc]initWithFrame:screenBounds];
    mainViewController = [[ViewController alloc]init];
    [mainWindow setRootViewController:mainViewController];
    myView = [[GLESView alloc]initWithFrame:screenBounds];
    [mainViewController setView:myView];
    [myView release];
    
    //[mainWindow addSubview:[mainViewController view]];
    [mainWindow makeKeyAndVisible];
    [myView startAnimation];

    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    //[myView stopAnimation];
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
 
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
   
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    //[myView startAnimation];
}


- (void)applicationWillTerminate:(UIApplication *)application {
    //[myView stopAnimation];
}

- (void)dealloc
{
    [myView release];
    [mainViewController release];
    [mainWindow release];
    
    [super dealloc];
}

@end
