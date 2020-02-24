//
//  MyView.m
//  Window
//
//  Created by sugat mankar on 21/02/20.
//

#import "MyView.h"

@implementation MyView
{
    NSString *centralText;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if(self)
    {
        [self setBackgroundColor:[UIColor whiteColor]];
        centralText = @"Hello World!!!";
        
        UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];
        
        UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecognizer];
        
        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];
        
        UISwipeGestureRecognizer *swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        [self addGestureRecognizer:swipeGestureRecognizer];
        
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:longPressGestureRecognizer];
        
    }
    return(self);
}

- (void)drawRect:(CGRect)rect
{
    UIColor *fillColor = [UIColor blackColor];
    [fillColor set];
    UIRectFill(rect);
    
    NSDictionary *dictionaryForTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys: [UIFont fontWithName:@"Helvetica"  size:24], NSFontAttributeName, [UIColor greenColor],NSForegroundColorAttributeName, nil];
    
    CGSize textSize = [centralText sizeWithAttributes:dictionaryForTextAttributes];
    
    CGPoint point;
    point.x = (rect.size.width/2) - (textSize.width/2);
    point.y = (rect.size.height/2) - (textSize.height/2);
    
    [centralText drawAtPoint:point withAttributes:dictionaryForTextAttributes];
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //code
}

- (void)onSingleTap:(UITapGestureRecognizer *)gr
{
    centralText = @"'onSingleTap' Event Occured";
    [self setNeedsDisplay];
}


- (void)onDoubleTap:(UITapGestureRecognizer *)gr
{
    centralText = @"'onDoubleTap' Event Occured";
    [self setNeedsDisplay];
}


- (void)onSwipe:(UISwipeGestureRecognizer *)gr
{
    [self release];
    exit(0);
}

- (void)onLongPress:(UILongPressGestureRecognizer *)gr
{
    centralText = @"'onLongPress' Event Occured";
    [self setNeedsDisplay];
}

- (void)dealloc
{
    [super dealloc];
}

@end
