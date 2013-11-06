#import <Foundation/Foundation.h>

@interface NSString (MyAdditions)
- (NSUInteger)lengthh;
@end

@interface Observer
+ (instancetype)observerWithTarget:(id)target action:(SEL)selector;
@end

@interface Test : NSObject
@property (nonatomic,strong) id keyValueObserver;
@end

@implementation Test

- (void)test 
{
  NSString* string = @"";
  self.keyValueObserver = [Observer observerWithTarget:string action:@selector(lengthh)];
}
@end

void do_math(int *x) {
  *x += 5;
}

int main(void) {
  int result = -1, val = 4;
  do_math(&val);
  return result;
}
