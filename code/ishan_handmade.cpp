#include "ishan_source.h"

//This is the entry point of the window application
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {

//function that loads the necessary libraries for x input controllers
Ish_32_LoadXInput();

//Initializing an window class with null values
    WNDCLASS WindowClass = {};

//Assigning necessary values in our newly intialized window class
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Ish_32_MainWindowCallBack; 
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "Ishan_Handmade_Hero_Class";

//Registering the created window class
    if(RegisterClass(&WindowClass)){
        
        //Creating a window by using CreateWindowEx function that returns a handle of the newly created window
       HWND WindowHandle = CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Ishan's Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,                
                CW_USEDEFAULT,                
                CW_USEDEFAULT,                
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0
                );

        //If the window is created and handle is returned successfully
        if(WindowHandle){

           //Bitmap 
            int XOffset = 0;
            int YOffset = 0;
            //Sound
            int SamplesPerSecond = 48000;
            int BytesPerSample = sizeof(s16) * 2;
            int SecondaryBufferSize = 2 * SamplesPerSecond * BytesPerSample;
            int ToneHz = 256;
            int SquareWavePeriod = SamplesPerSecond/ ToneHz;
            int HalfSquareWavePeriod = SquareWavePeriod / 2;
            int ToneVolume = 3000;
            u32 RunningSampleIndex = 0;

            //Main Window Loop
            Operating = true;

            
        //using the same device context for all purposes
        HDC DeviceContext = GetDC(WindowHandle); 
        // For blitting the bitmap in to the screen even though the wm paint message is not called by the application
        Ish_32_ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

        Ish_32_InitDSound(WindowHandle, SamplesPerSecond, SecondaryBufferSize);
        GlobalSecondaryBuffer->Play(0,0,DSBPLAY_LOOPING);       

            //while loop if the window is willing to operate
            while(Operating){
                MSG Message;
                //the peek message function will be more flexible to us as compared to Getmessage as the funtion does not wait for any messages in order to 
                // run the application 
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) 
                {
                    //double checking the message in order to tackle heavey load of messages
                    if(Message.message == WM_QUIT){
                        Operating = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message); 

                }      

                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT;++ControllerIndex)
                {
                    //initaializing an xinput state structure
                    XINPUT_STATE ControllerState; 

                    //incase of succesfully retrieving the controller state 
                    if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {

                       //getting an pointer to the gamepad structure for easy use 
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        //state for controller buttons
                        bool Up = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool Down = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool Left = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT; 
                        bool Right = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool Start = Pad->wButtons & XINPUT_GAMEPAD_START;
                        bool Back = Pad->wButtons & XINPUT_GAMEPAD_BACK; 
                        bool LeftShoulder = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool RightShoulder = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                        bool A = Pad->wButtons & XINPUT_GAMEPAD_A;
                        bool B = Pad->wButtons & XINPUT_GAMEPAD_B;
                        bool X = Pad->wButtons & XINPUT_GAMEPAD_X;
                        bool Y = Pad->wButtons & XINPUT_GAMEPAD_Y;

                        //state for controller sticks
                        s16 StickX = Pad->sThumbLX;
                        s16 StickY = Pad->sThumbLY;

                        //for A button controller  (moves upword)
                        if(A){
                            YOffset += 2;
                        }

                        //for sticks
                        XOffset += StickX >> 12;
                        XOffset += StickY >> 12;

                    }else{

                    }
                }

                //specfying vibration level of controller in the application, max for now
                XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 60000;
                // Vibration.wRightMotorSpeed = 60000;
                XInputSetState(0, &Vibration);

            //in order to render the bitmap in every single frame
                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

            //DirectSound Output test
            DWORD PlayCursor;
            DWORD WriteCursor;       
            if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
            {
                DWORD WritePointer = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;
                DWORD BytesToWrite; 

                if(WritePointer > PlayCursor){
                    BytesToWrite = SecondaryBufferSize - WritePointer;
                    BytesToWrite += PlayCursor;
                }else{
                    BytesToWrite = PlayCursor - WriteCursor;
                }


                void* Region1;
                DWORD Region1Size;
                void* Region2;
                DWORD Region2Size;

                //for loccking the sound buffer in order to write sound in the buffer               
               if(SUCCEEDED (GlobalSecondaryBuffer->Lock(WritePointer, BytesToWrite,&Region1, &Region1Size, &Region2, &Region2Size, 0))){
                    
                    //For Region1
                    //the pointer to the start of region1  
                    s16 * SampleOut = (s16*)Region1;
                    //returns the total number of samples in region1 
                    DWORD Region1SampleCount = Region1Size / BytesPerSample;
                    //loop for filling up both channels of a specific audio sample
                    for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex){
                        s16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
                        *SampleOut++ = SampleValue;
                        *SampleOut++ = SampleValue;
                    }
                    
                    //For Region2
                    //the pointer to the start of region1  
                    SampleOut = (s16*)Region2;
                    //returns the total number of samples in region1 
                    DWORD Region2SampleCount = Region2Size / BytesPerSample;
                    //loop for filling up both channels of a specific audio sample
                    for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex){
                        s16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
                        *SampleOut++ = SampleValue;
                        *SampleOut++ = SampleValue;
                    }

                    GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
               }

            }


            Ish_32_window_dimension Data = Ish_32_GetWindowDimension(WindowHandle);
            Ish_32_DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Data.Width, Data.Height);

            //incrementing the xoffset in order to create some horixontal movement animation in the application
                ++XOffset;
            
        }

        }else{

        }
    
    }else{

    }

    return (0);// Returns 0 as the function returns an integer value. Just for formality i guess.
}


//This is the callback function that handles the window messages
 LRESULT CALLBACK Ish_32_MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) { 
    
    LRESULT Result = 0;

    //This switch statement function handles every single message present in the window's message queue
    switch(Message){
        //This case handles the window resize message. It occurs when the user resizes the window.
        case(WM_SIZE):
        {
        }break;

        //This is case handles the destroy message. It occurs when an window is forcefully destroyed
        case(WM_DESTROY):
        {
            Operating = false;
        }break;

        //This case handles the close message.It occurs whenever the user wants to close the window.
        case(WM_CLOSE):
        {
            Operating = false;
        }break;
       
        //This case handles the Window is currently active message.It occurs whenever our window is set as the active window among other possible applications.
        case(WM_ACTIVATEAPP):
        {
            OutputDebugStringA("Activated\n");
        }break;
       
        //This case handles whether to paint using a specific device context. 
        case(WM_PAINT):
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            Ish_32_window_dimension Data = Ish_32_GetWindowDimension(Window);
            Ish_32_DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Data.Width, Data.Height);
            EndPaint(Window, &Paint);
        }break;

        //This case handles the keyboard input messages. All of the messages are handles insiede the same case
        case(WM_SYSKEYUP):
        case(WM_SYSKEYDOWN):
        case(WM_KEYDOWN):
        case(WM_KEYUP):
        {
            bool IsDown =  ((LParam & (1 << 31 )) == 0);
            bool WasDown =   ((LParam & (1 << 30 )) != 0);
            b32 AltKeywasDown = (LParam & (1 << 29)); 

            //making use of the wParam parameter
            u32 VKCode = WParam;

            //case that handles forceful exit i.e ALT + F4 Key
            if(VKCode == VK_F4 && AltKeywasDown) 
            {
                Operating = false;
            }

            //in order to maintain repetition of function calls
            if(IsDown != WasDown){
                //if else case for every single virtual key code possible user can input
                if(VKCode == 'Q')
                {
                    OutputDebugStringA("Q key: ");
                    if(IsDown) {
                        OutputDebugStringA("is Down");
                    }

                    if(WasDown) {
                        OutputDebugStringA("was Down");
                    }
                    OutputDebugStringA("\n");
                }

                else if(VKCode == 'E')
                {
                }
                
                else if(VKCode == 'W')
                {
                }

                else if(VKCode == 'A')
                {
                }

                else if(VKCode == 'S')
                {
                }

                else if(VKCode == 'D')
                {
                }

                else if(VKCode == VK_UP)
                {
                }

                else if(VKCode == VK_DOWN)
                {
                }

                else if(VKCode == VK_LEFT)
                {
                }

                else if(VKCode == VK_RIGHT)
                {
                }

                else if(VKCode == VK_ESCAPE)
                {
                }

                else if(VKCode == VK_SPACE)
                {
                }

            }
        }break;

       //This is an in build function of Windows that handles the other unhandled messages automatically by itself
        default:
        {
               Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;


        return Result;
    }

 }   



//function that initializes or resizes a device independent Bitmap (DIB) each time the OS sends an WM_SIZE Message.
internal void Ish_32_ResizeDIBSection(Ish_32_offscreen_buffer *Buffer, int Width, int Height) {

    //freeing the bitmapmemory in case it already exists
    if(Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    //passing dimension values for the Bitmap 
    Buffer->Width = Width;
    Buffer->Height = Height;
    //specifying the number of bytes a single pixel will occupy in the bitmap memory
    Buffer->BytesPerPixel = 4;    
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;

    //fillling up necessary information about the Bitmap structure
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount= 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;


    //calculating the total memory size for the bitmap
    int BitmapMemorySize = Buffer->BytesPerPixel * (Buffer->Width * Buffer->Height);
    
    //allocating the memory for the bitmap in the virtual memory
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

}


//function that updates or repaints the window when WM_PAINT message is called.
internal void Ish_32_DisplayBufferInWindow(Ish_32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight){

    //function that copies the DIB image from the source rectangle to destination rectangle
    //It can automatically set the image size accrding to the destination rectangle's attributes, both bigger or smaller then tha source itself.
    StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer->Width, Buffer->Height, Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}



//a function defined for filling up the pixel information of the bitmap
internal void RenderWeirdGradient(Ish_32_offscreen_buffer *Buffer,int XOffset, int YOffset) {
    
    // Typecasting the void memory pointer to an unsgined char pointer as we can
    // have control over every single bit of the memory.
    u8* Row = (u8 *) Buffer->Memory;

    for(int Y = 0; Y < Buffer->Height; ++Y){
        u32 * Pixel = (u32*) Row;
        for(int X = 0; X < Buffer->Width; ++X){
            
            u8 Red = 0;
            u8 Green = (u8)(Y + YOffset);
            u8 Blue = (u8)(X + XOffset);
            *Pixel++ = Red << 16 | Green  << 8 | Blue;

        }
  
        Row += Buffer->Pitch;
    }
}


internal Ish_32_window_dimension Ish_32_GetWindowDimension(HWND Window)
{
    Ish_32_window_dimension Data;

    RECT ClientRect; 
    GetClientRect(Window, &ClientRect);
    Data.Width = ClientRect.right - ClientRect.left;
    Data.Height = ClientRect.bottom - ClientRect.top;

    return(Data);
}


internal void Ish_32_LoadXInput()
{
    //for retriving a handle to the library
    //checking thr library in three different possible dll files
    HMODULE XInputLibrary = LoadLibraryA("Xinput1_4.dll");

    //dll file check
    if(!XInputLibrary){
        XInputLibrary = LoadLibraryA("Xinput1_3.dll");
    }

    if(!XInputLibrary){
        XInputLibrary = LoadLibraryA("Xinput9_1_0.dll");
    }


    //incase if we found the xinput library
    if(XInputLibrary)
    {
        //for get state function
        //getproc address helps to find the specfic function in a provided library.
        //it takes handle of the module and function name as its input parameters.
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");

        //in case we didnot find the get state function
        if(!XInputGetState){
            XInputGetState = XInputGetStateStub;
        }


        //for set state function
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");

        //in case we didnot find the get state function
        if(!XInputSetState){
            XInputSetState = XInputSetStateStub;
        }

    }else{
       //in case we did not find any of hte functions, assigning them to the stub funcitons for preventing game crashes. 
        XInputGetState = XInputGetStateStub;
        XInputSetState = XInputSetStateStub;
    }
}


internal void Ish_32_InitDSound(HWND Window, s32 SamplesPerSecond, s32 BufferSize) {

    //loading the sound library and storing its module handle for easier use    
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary)
    {
        //searching directsoundcreate function in the dsound library
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate"); 
        
        //intializing an idirectsound strcuture
        IDirectSound *DirectSound;

        //if statement if the function was succesfully retrieved and the directsound object was succesfully created
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))){

            //intializing an empty waveformat ex strcuture
            WAVEFORMATEX WaveFormat = {};
            //filling up the structure
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/ 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))){
                //creating a primary buffer

               //intializing an empty dsbufferdesc struture
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;


                IDirectSoundBuffer *PrimaryBuffer; 

                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        OutputDebugStringA("Primary Buffer Format is set");                   
                    }
                }

            }

            //creating a secondary buffer
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwBufferBytes = BufferSize;
                BufferDescription.lpwfxFormat = &WaveFormat;

                
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
                {
                        OutputDebugStringA("Secondary Buffer Format is set");                   
                }

        }else{
            
        }
    }
}