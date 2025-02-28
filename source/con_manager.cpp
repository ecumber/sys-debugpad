#include "con_manager.hpp"
#include "udp_manager.hpp"
#include <mutex>
#include <array>

// Some of the code comes from hid-mitm

static std::string log_buffer; 

int FakeController::initialize(u16 conDeviceType)
{
    if (isInitialized) return 0;
    //printToFile("Controller initializing...");

    // Set the controller type to Pro-Controller, and set the npadInterfaceType.
    

    switch(conDeviceType)
    {
        case 1:
            controllerDevice.deviceType = HidDeviceType_DebugPad;
            break;

    }
    
    char* devtypemessage = (char*)malloc(controllerDevice.deviceType + 20);
    sprintf(devtypemessage, "device type is %d", controllerDevice.deviceType);
    log_buffer = devtypemessage;

    // Set the controller colors. The grip colors are for Pro-Controller on [9.0.0+].
    controllerDevice.singleColorBody = RGBA8_MAXALPHA(255,0,220);
    controllerDevice.singleColorButtons = RGBA8_MAXALPHA(0,0,0);
    if (conDeviceType == 1)
    {
        controllerDevice.colorLeftGrip = RGBA8_MAXALPHA(0,0,0);
        controllerDevice.colorRightGrip = RGBA8_MAXALPHA(0,0,0);
    }
    
    controllerDevice.npadInterfaceType = HidNpadInterfaceType_USB;

    // Setup example controller state.
    //controllerState.battery_level = 4; // Set battery charge to full.

    if (conDeviceType == 1 || conDeviceType == 2)
    {
        controllerState.analog_stick_l.x = 0x0;
        controllerState.analog_stick_l.y = -0x0;
    }

    if (conDeviceType == 1 || conDeviceType == 3)
    {
        controllerState.analog_stick_r.x = 0x0;
        controllerState.analog_stick_r.y = -0x0;
    }
    
    //myResult = hiddbgAttachHdlsVirtualDevice(&controllerHandle, &controllerDevice);
    //if (R_FAILED(myResult)) {
    //    log_buffer = "Failed connecting controller...";
    //    return -1;
    //}

    log_buffer = "Controller initialized!";
    isInitialized = true;
    return 0;
}

int FakeController::deInitialize()
{
    if (!isInitialized) return 0;
    Result myResult;

    controllerState = {0};
    
    myResult = hiddbgSetDebugPadAutoPilotState(&controllerState);
    if (R_FAILED(myResult)) {
        log_buffer = "Fatal Error while detaching controller.";
    }
    controllerHandle = {0};
    controllerDevice = {0};

    isInitialized = false;

    return 0;
}

std::array<FakeController, 4> fakeControllerList;
u64 buttonPresses;

void apply_fake_con_state(struct input_message message)
{
    // Check if the magic is correct
    if(message.magic != INPUT_MSG_MAGIC)
        return;

    u16 conType;
    u64 keys;
    s32 joylx;
    s32 joyly;
    s32 joyrx;
    s32 joyry;

    for(s32 i = 0; i < message.con_count; i++)
    {
        switch(i)
        {
            case 0:
                conType = message.con_type;
                keys = message.keys;
                joylx = message.joy_l_x;
                joyly = message.joy_l_y;
                joyrx = message.joy_r_x;
                joyry = message.joy_r_y;
                break;
            case 1:
                conType = message.con_type2;
                keys = message.keys2;
                joylx = message.joy_l_x2;
                joyly = message.joy_l_y2;
                joyrx = message.joy_r_x2;
                joyry = message.joy_r_y2;
                break;
            case 2:
                conType = message.con_type3;
                keys = message.keys3;
                joylx = message.joy_l_x3;
                joyly = message.joy_l_y3;
                joyrx = message.joy_r_x3;
                joyry = message.joy_r_y3;
                break;
            case 3:
                conType = message.con_type4;
                keys = message.keys4;
                joylx = message.joy_l_x4;
                joyly = message.joy_l_y4;
                joyrx = message.joy_r_x4;
                joyry = message.joy_r_y4;
                break;
        }

        // If there is no controller connected, we have to initialize one
        if (!fakeControllerList[i].isInitialized && (conType > 0 && conType < 4))
        {
            fakeControllerList[i].initialize(conType);
        } 
        // If there is a controller connected, but we changed the controller type to a non-existant one, we'll disconnect it
        else if (fakeControllerList[i].isInitialized && (conType < 1 || conType > 3))
        {
            fakeControllerList[i].deInitialize();
        }

        if (fakeControllerList[i].isInitialized)
        {
            fakeControllerList[i].controllerState.attributes = 1;
            fakeControllerList[i].controllerState.buttons = keys;
            fakeControllerList[i].controllerState.analog_stick_l.x = joylx;
            fakeControllerList[i].controllerState.analog_stick_l.y = joyly;
            fakeControllerList[i].controllerState.analog_stick_r.x = joyrx;
            fakeControllerList[i].controllerState.analog_stick_r.y = joyry;
            if (fakeControllerList[i].controllerState.buttons != 0) {
                char *out = new char[32];
                snprintf(out, 32, "Received input %i", fakeControllerList[i].controllerState.buttons);
                log_buffer = out;
            }

            const HiddbgDebugPadAutoPilotState* apstate = &fakeControllerList[i].controllerState;

            Result myResult;
            myResult = hiddbgSetDebugPadAutoPilotState(apstate);

            if (R_FAILED(myResult)) {
                log_buffer = "Fatal Error while updating Controller State.";
                char* out = new char[50];
                sprintf(out, "hiddbgSetHdlsState(): 0x%x\n", myResult);
                log_buffer = out;
            }   
        }
    }

    return;
}

static Mutex pkgMutex;
static struct input_message fakeConsState;

void networkThread(void* _)
{
    struct input_message temporal_pkg;
    log_buffer = "Starting Network Loop Thread!";
    while (true)
    {
        int poll_res = poll_udp_input(&temporal_pkg);
        mutexLock(&pkgMutex);

        if (poll_res == 0)
        {
            fakeConsState = temporal_pkg;
            apply_fake_con_state(fakeConsState);
        }
        else
        {
            fakeConsState.magic = 0;
            svcSleepThread(1e+7l);
        }
        mutexUnlock(&pkgMutex);

        svcSleepThread(-1);
    }
}

void loggingThread(void* _) {
    while (true) 
    {
        if (log_buffer != "") 
        {
            const char* output = log_buffer.c_str();
            printToFile(output);
            log_buffer = "";
        }
    }
}