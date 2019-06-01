#include "pch.h"

#include <kinc/network/http.h>

#import <Foundation/Foundation.h>

@interface Connection : NSObject <NSURLConnectionDelegate> {
	NSMutableData* responseData;
	Kinc_HttpCallback callback;
	void* data;
	int statusCode;
}

@end

@implementation Connection

- (id)initWithCallback:(Kinc_HttpCallback)aCallback andData:(void*)someData {
	if (self = [super init]) {
		callback = aCallback;
		data = someData;
		statusCode = 0;
		return self;
	}
	else {
		return nil;
	}
}

- (void)connection:(NSURLConnection*)connection didReceiveResponse:(NSURLResponse*)response {
	responseData = [[NSMutableData alloc] init];
	NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
	statusCode = (int)[httpResponse statusCode];
}

- (void)connection:(NSURLConnection*)connection didReceiveData:(NSData*)moreData {
	[responseData appendData:moreData];
}

- (NSCachedURLResponse*)connection:(NSURLConnection*)connection willCacheResponse:(NSCachedURLResponse*)cachedResponse {
	return nil;
}

- (void)connectionDidFinishLoading:(NSURLConnection*)connection {
	[responseData appendBytes:"\0" length:1];
	// printf("Got %s\n\n", (const char*)[responseData bytes]);
	callback(0, statusCode, (const char*)[responseData bytes], data);
}

- (void)connection:(NSURLConnection*)connection didFailWithError:(NSError*)error {
	[responseData appendBytes:"\0" length:1];
	callback(1, statusCode, 0, data);
}

@end

using namespace Kore;

void Kinc_HttpRequest(const char* url, const char* path, const char* data, int port, bool secure, int method, const char* header, Kinc_HttpCallback callback,
                       void* callbackdata) {
	NSString* urlstring = secure ? @"https://" : @"http://";
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:url]];
	urlstring = [urlstring stringByAppendingString:@":"];
	urlstring = [urlstring stringByAppendingString:[[NSNumber numberWithInt:port] stringValue]];
	urlstring = [urlstring stringByAppendingString:@"/"];
	urlstring = [urlstring stringByAppendingString:[NSString stringWithUTF8String:path]];

	NSURL* aUrl = [NSURL URLWithString:urlstring];
	NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:aUrl cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:60.0];
	[request addValue:@"application/json" forHTTPHeaderField:@"Content-type"];

	switch (method) {
	case KINC_HTTP_GET:
		[request setHTTPMethod:@"GET"];
		break;
	case KINC_HTTP_POST:
		[request setHTTPMethod:@"POST"];
		break;
	case KINC_HTTP_PUT:
		[request setHTTPMethod:@"PUT"];
		break;
	case KINC_HTTP_DELETE:
		[request setHTTPMethod:@"DELETE"];
		break;
	}

	if (data != 0) {
		// printf("Sending %s\n\n", data);
		NSString* datastring = [NSString stringWithUTF8String:data];
		[request setHTTPBody:[datastring dataUsingEncoding:NSUTF8StringEncoding]];
	}

	Connection* connection = [[Connection alloc] initWithCallback:callback andData:callbackdata];
	[[NSURLConnection alloc] initWithRequest:request delegate:connection];
}
