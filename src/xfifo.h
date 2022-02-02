#ifndef _XFIFO_Header_
#define _XFIFO_Header_


#ifdef __cplusplus
extern "C" {
#endif

void*	XFIFOOpen(void);
void	XFIFOClose(void** instancePtr);
int		XFIFOWrite(void* instance, char* buffer, int requestSize);
void	XFIFOCancel(void* instance);


#ifdef __cplusplus
}
#endif


#endif // _XFIFO_Header_
