//Importing all of the neccessary dependencies
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

//MACROS
#define global_variable static

//it is assigned to funcitons only to make the funciton native and usable to the specific file only
#define internal static

#define local_persist static


//STRUCTS

//Stucture of a Bitmap
struct Ish_32_offscreen_buffer {
            
    //intializing a bitmapinfo structure with null values
    BITMAPINFO Info;

    //void pointer to the BitmapMemory
    void * Memory;

    //width of the bitmap
    int Width;

    //height of the bitmap
    int Height;

    //bytes per pixel 
    int BytesPerPixel;

    //intializing a pitch
    int Pitch;

};


//strcuture for storing the dimensions of any window
struct Ish_32_window_dimension {
    int Width;
    int Height;
};


//CONSTANTS

// (Temp Solution) creating a global variable that indicates whether the application is operating or not
global_variable bool Operating;

//intializing a backbuffer strcture
global_variable Ish_32_offscreen_buffer GlobalBackBuffer;

//intializing an i direct sound buffer object
global_variable IDirectSoundBuffer* GlobalSecondaryBuffer;

//Some custom definition integer values
//unsigned
typedef uint8_t u8; 
typedef uint16_t u16; 
typedef uint32_t u32; 
typedef uint64_t u64; 
//signed
typedef int8_t s8; 
typedef int16_t s16; 
typedef int32_t s32; 
typedef int64_t s64; 
typedef s32 b32;


//FUNCTIONS
//function that initializes or resizes a device independent Bitmap (DIB) each time the OS sends an WM_SIZE Message.
internal void Ish_32_ResizeDIBSection(Ish_32_offscreen_buffer *Buffer, int Width, int Height);

//function that updates or repaints the window when WM_PAINT message is called.
internal void Ish_32_DisplayBufferInWindow(Ish_32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight);

//This is the callback function that handles the window messages
 LRESULT CALLBACK Ish_32_MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

//a function defined for filling up the pixel information of the bitmap
internal void RenderWeirdGradient(Ish_32_offscreen_buffer *Buffer, int XOffset, int YOffset);

//a helper function for extracting the window dimensions from the given window handle
internal Ish_32_window_dimension Ish_32_GetWindowDimension(HWND Window);

// a funciton for loading x input library in the application
internal void Ish_32_LoadXInput();

//a function for loading sound function from the sound library in the application
internal void Ish_32_InitDSound(HWND Window, s32 SamplesPerSecond, s32 BufferSize);


//MACRO FUNCTIONS

//X Input get state functions 
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_


// X Input set state functions
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

//For Sound 
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);