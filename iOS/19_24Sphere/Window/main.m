//
//  main.m
//  Window
//
//  Created by sugat mankar on 21/02/20.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc]init];
    int ret = UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    [pPool release];
    return ret;
}
