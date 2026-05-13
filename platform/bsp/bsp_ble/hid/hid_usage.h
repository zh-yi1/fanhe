/*
 * @File name    : hid_usage.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2023-11-28
 * @Description  : HID Usage ID Definition by HUT_1.4.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */


/**
 * HID Keyboard/Keypad Page Usage ID
 */

#define HID_KB_RELEASE                                                      0x00        // Release
#define HID_KB_ERROR_ROLL_OVER                                              0x01        // Keyboard ErrorRollOver
#define HID_KB_POST_FAIL                                                    0x02        // Keyboard POSTFail
#define HID_KB_ERROR_UNDEFINED                                              0x03        // Keyboard ErrorUndefined
#define HID_KB_A                                                            0x04        // Keyboard a and A
#define HID_KB_B                                                            0x05        // Keyboard b and B
#define HID_KB_C                                                            0x06        // Keyboard c and C
#define HID_KB_D                                                            0x07        // Keyboard d and D
#define HID_KB_E                                                            0x08        // Keyboard e and E
#define HID_KB_F                                                            0x09        // Keyboard f and F
#define HID_KB_G                                                            0x0A        // Keyboard g and G
#define HID_KB_H                                                            0x0B        // Keyboard h and H
#define HID_KB_I                                                            0x0C        // Keyboard i and I
#define HID_KB_J                                                            0x0D        // Keyboard j and J
#define HID_KB_K                                                            0x0E        // Keyboard k and K
#define HID_KB_L                                                            0x0F        // Keyboard l and L
#define HID_KB_M                                                            0x10        // Keyboard m and M
#define HID_KB_N                                                            0x11        // Keyboard n and N
#define HID_KB_O                                                            0x12        // Keyboard o and O
#define HID_KB_P                                                            0x13        // Keyboard p and P
#define HID_KB_Q                                                            0x14        // Keyboard q and Q
#define HID_KB_R                                                            0x15        // Keyboard r and R
#define HID_KB_S                                                            0x16        // Keyboard s and S
#define HID_KB_T                                                            0x17        // Keyboard t and T
#define HID_KB_U                                                            0x18        // Keyboard u and U
#define HID_KB_V                                                            0x19        // Keyboard v and V
#define HID_KB_W                                                            0x1A        // Keyboard w and W
#define HID_KB_X                                                            0x1B        // Keyboard x and X
#define HID_KB_Y                                                            0x1C        // Keyboard y and Y
#define HID_KB_Z                                                            0x1D        // Keyboard z and Z
#define HID_KB_1                                                            0x1E        // Keyboard 1 and !
#define HID_KB_2                                                            0x1F        // Keyboard 2 and @
#define HID_KB_3                                                            0x20        // Keyboard 3 and #
#define HID_KB_4                                                            0x21        // Keyboard 4 and $
#define HID_KB_5                                                            0x22        // Keyboard 5 and %
#define HID_KB_6                                                            0x23        // Keyboard 6 and ∧
#define HID_KB_7                                                            0x24        // Keyboard 7 and &
#define HID_KB_8                                                            0x25        // Keyboard 8 and *
#define HID_KB_9                                                            0x26        // Keyboard 9 and (
#define HID_KB_0                                                            0x27        // Keyboard 0 and )
#define HID_KB_ENTER                                                        0x28        // Keyboard Return (ENTER)
#define HID_KB_ESCAPE                                                       0x29        // Keyboard ESCAPE
#define HID_KB_BACKSPACE                                                    0x2A        // Keyboard DELETE (Backspace)
#define HID_KB_TAB                                                          0x2B        // Keyboard Tab
#define HID_KB_SPACEBAR                                                     0x2C        // Keyboard Spacebar
#define HID_KB_HYPHEN_AND_UNDERSCORE                                        0x2D        // Keyboard - and (underscore)
#define HID_KB_EQUAL_AND_ADD                                                0x2E        // Keyboard = and +
#define HID_KB_L_SQUARE_AND_ROUND_BRACKETS                                  0x2F        // Keyboard [ and {
#define HID_KB_R_SQUARE_AND_ROUND_BRACKETS                                  0x30        // Keyboard ] and }
#define HID_KB_BACKSLASH_AND_PIPE                                           0x31        // Keyboard \and |
#define HID_KB_POUND_AND_TILDE                                              0x32        // Keyboard Non-US # and ˜
#define HID_KB_SEMICOLON_AND_COLON                                          0x33        // Keyboard ; and :
#define HID_KB_MINUTE_AND_SECOND                                            0x34        // Keyboard ‘ and “
#define HID_KB_GRAVE_ACCENT_AND_TILDE                                       0x35        // Keyboard Grave Accent and Tilde
#define HID_KB_COMMA_AND_LESS_THAN                                          0x36        // Keyboard , and <
#define HID_KB_POINT_AND_GREATER_THAN                                       0x37        // Keyboard . and >
#define HID_KB_SLASH_AND_QUESTION_MARK                                      0x38        // Keyboard / and ?
#define HID_KB_CAPS_LOCK                                                    0x39        // Keyboard Caps Lock
#define HID_KB_F1                                                           0x3A        // Keyboard F1
#define HID_KB_F2                                                           0x3B        // Keyboard F2
#define HID_KB_F3                                                           0x3C        // Keyboard F3
#define HID_KB_F4                                                           0x3D        // Keyboard F4
#define HID_KB_F5                                                           0x3E        // Keyboard F5
#define HID_KB_F6                                                           0x3F        // Keyboard F6
#define HID_KB_F7                                                           0x40        // Keyboard F7
#define HID_KB_F8                                                           0x41        // Keyboard F8
#define HID_KB_F9                                                           0x42        // Keyboard F9
#define HID_KB_F10                                                          0x43        // Keyboard F10
#define HID_KB_F11                                                          0x44        // Keyboard F11
#define HID_KB_F12                                                          0x45        // Keyboard F12
#define HID_KB_PRINT_SCREEN                                                 0x46        // Keyboard PrintScreen
#define HID_KB_SCROLL_LOCK                                                  0x47        // Keyboard Scroll Lock
#define HID_KB_PAUSE                                                        0x48        // Keyboard Pause
#define HID_KB_INSERT                                                       0x49        // Keyboard Insert
#define HID_KB_HOME                                                         0x4A        // Keyboard Home
#define HID_KB_PAGEUP                                                       0x4B        // Keyboard PageUp
#define HID_KB_DELETE_FORWARD                                               0x4C        // Keyboard Delete Forward
#define HID_KB_END                                                          0x4D        // Keyboard End
#define HID_KB_PAGEDOWN                                                     0x4E        // Keyboard PageDown
#define HID_KB_RIGHTARROW                                                   0x4F        // Keyboard RightArrow
#define HID_KB_LEFTARROW                                                    0x50        // Keyboard LeftArrow
#define HID_KB_DOWNARROW                                                    0x51        // Keyboard DownArrow
#define HID_KB_UPARROW                                                      0x52        // Keyboard UpArrow
#define HID_KP_NUM_LOCK                                                     0x53        // Keypad Num Lock and Clear
#define HID_KP_SLASH                                                        0x54        // Keypad /
#define HID_KP_STAR                                                         0x55        // Keypad *
#define HID_KP_SUB                                                          0x56        // Keypad -
#define HID_KP_ADD                                                          0x57        // Keypad +
#define HID_KP_ENTER                                                        0x58        // Keypad ENTER
#define HID_KP_1                                                            0x59        // Keypad 1 and End
#define HID_KP_2                                                            0x5A        // Keypad 2 and Down Arrow
#define HID_KP_3                                                            0x5B        // Keypad 3 and PageDn
#define HID_KP_4                                                            0x5C        // Keypad 4 and Left Arrow
#define HID_KP_5                                                            0x5D        // Keypad 5
#define HID_KP_6                                                            0x5E        // Keypad 6 and Right Arrow
#define HID_KP_7                                                            0x5F        // Keypad 7 and Home
#define HID_KP_8                                                            0x60        // Keypad 8 and Up Arrow
#define HID_KP_9                                                            0x61        // Keypad 9 and PageUp
#define HID_KP_0                                                            0x62        // Keypad 0 and Insert
#define HID_KP_POINT                                                        0x63        // Keypad . and Delete
#define HID_KP_BACKSLASH_AND_PIPE2                                          0x64        // Keyboard Non-US \and |
#define HID_KP_APPLICATION                                                  0x65        // Keyboard Application


/**
 * HID Consumer Page Usage ID.
 */

#define HID_CCC_RELEASE                                                     0x0000      // Release
#define HID_CCC_CLOCK_ADD_10                                                0x0020      // +10
#define HID_CCC_CLOCK_ADD_100                                               0x0021      // +100
#define HID_CCC_AM_OR_PM                                                    0x0022      // AM/PM

#define HID_CCC_POWER                                                       0x0030      // Power
#define HID_CCC_RESET                                                       0x0031      // Reset
#define HID_CCC_SLEEP                                                       0x0032      // Sleep
#define HID_CCC_SLEEP_AFTER                                                 0x0033      // Sleep After
#define HID_CCC_SLEEP_MODE                                                  0x0034      // Sleep Mode
#define HID_CCC_ILLUMINATION                                                0x0035      // Illumination
#define HID_CCC_FUNCTION_BUTTONS                                            0x0036      // Function Buttons

#define HID_CCC_MENU                                                        0x0040      // Menu
#define HID_CCC_MENU_PICK                                                   0x0041      // Menu Pick
#define HID_CCC_MENU_UP                                                     0x0042      // Menu Up
#define HID_CCC_MENU_DOWN                                                   0x0043      // Menu Down
#define HID_CCC_MENU_LEFT                                                   0x0044      // Menu Left
#define HID_CCC_MENU_RIGHT                                                  0x0045      // Menu Right
#define HID_CCC_MENU_ESCAPE                                                 0x0046      // Menu Escape
#define HID_CCC_MENU_VALUE_INCREASE                                         0x0047      // Menu Value Increase
#define HID_CCC_MENU_VALUE_DECREASE                                         0x0048      // Menu Value Decrease

#define HID_CCC_DATA_ON_SCREEN                                              0x0060      // Data On Screen
#define HID_CCC_CLOSED_CAPTION                                              0x0061      // Closed Caption
#define HID_CCC_CLOSED_CAPTION_SELECT                                       0x0062      // Closed Caption Select
#define HID_CCC_VCR_OR_TV                                                   0x0063      // VCR/TV
#define HID_CCC_BROADCAST_MODE                                              0x0064      // Broadcast Mode
#define HID_CCC_SNAPSHOT                                                    0x0065      // Snapshot
#define HID_CCC_STILL                                                       0x0066      // Still
#define HID_CCC_PICTURE_IN_PICTURE_TOGGLE                                   0x0067      // Picture-in-Picture Toggle
#define HID_CCC_PICTURE_IN_PICTURE_SWAP                                     0x0068      // Picture-in-Picture Swap
#define HID_CCC_RED_MENU_BUTTON                                             0x0069      // Red Menu Button
#define HID_CCC_GREEN_MENU_BUTTON                                           0x006A      // Green Menu Button
#define HID_CCC_BLUE_MENU_BUTTON                                            0x006B      // Blue Menu Button
#define HID_CCC_YELLOW_MENU_BUTTON                                          0x006C      // Yellow Menu Button
#define HID_CCC_ASPECT                                                      0x006D      // Aspect
#define HID_CCC_MODE_SELECT                                                 0x006E      // 3D Mode Select
#define HID_CCC_DISPLAY_BRIGHTNESS_INCREMENT                                0x006F      // Display Brightness Increment
#define HID_CCC_DISPLAY_BRIGHTNESS_DECREMENT                                0x0070      // Display Brightness Decrement
#define HID_CCC_DISPLAY_BRIGHTNESS                                          0x0071      // Display Brightness
#define HID_CCC_DISPLAY_BACKLIGHT_TOGGLE                                    0x0072      // Display Backlight Toggle
#define HID_CCC_DISPLAY_SET_BRIGHTNESS_TO_MINIMUM                           0x0073      // Display Set Brightness to Minimum
#define HID_CCC_DISPLAY_SET_BRIGHTNESS_TO_MAXIMUM                           0x0074      // Display Set Brightness to Maximum
#define HID_CCC_DISPLAY_SET_AUTO_BRIGHTNESS                                 0x0075      // Display Set Auto Brightness
#define HID_CCC_CAMERA_ACCESS_ENABLED                                       0x0076      // Camera Access Enabled
#define HID_CCC_CAMERA_ACCESS_DISABLED                                      0x0077      // Camera Access Disabled
#define HID_CCC_CAMERA_ACCESS_TOGGLE                                        0x0078      // Camera Access Toggle
#define HID_CCC_KEYBOARD_BRIGHTNESS_INCREMENT                               0x0079      // Keyboard Brightness Increment
#define HID_CCC_KEYBOARD_BRIGHTNESS_DECREMENT                               0x007A      // Keyboard Brightness Decrement
#define HID_CCC_KEYBOARD_BACKLIGHT_SET_LEVEL                                0x007B      // Keyboard Backlight Set Level
#define HID_CCC_KEYBOARD_BACKLIGHT_OOC                                      0x007C      // Keyboard Backlight OOC
#define HID_CCC_KEYBOARD_BACKLIGHT_SET_MINIMUM                              0x007D      // Keyboard Backlight Set Minimum
#define HID_CCC_KEYBOARD_BACKLIGHT_SET_MAXIMUM                              0x007E      // Keyboard Backlight Set Maximum
#define HID_CCC_KEYBOARD_BACKLIGHT_AUTO                                     0x007F      // Keyboard Backlight Auto
#define HID_CCC_SELECTION                                                   0x0080      // Selection
#define HID_CCC_ASSIGN_SELECTION                                            0x0081      // Assign Selection
#define HID_CCC_MODE_STEP                                                   0x0082      // Mode Step
#define HID_CCC_RECALL_LAST                                                 0x0083      // Recall Last
#define HID_CCC_ENTER_CHANNEL                                               0x0084      // Enter Channel
#define HID_CCC_ORDER_MOVIE                                                 0x0085      // Order Movie
#define HID_CCC_CHANNEL                                                     0x0086      // Channel
#define HID_CCC_MEDIA_SELECTION                                             0x0087      // Media Selection
#define HID_CCC_MEDIA_SELECT_COMPUTER                                       0x0088      // Media Select Computer
#define HID_CCC_MEDIA_SELECT_TV                                             0x0089      // Media Select TV
#define HID_CCC_MEDIA_SELECT_WWW                                            0x008A      // Media Select WWW
#define HID_CCC_MEDIA_SELECT_DVD                                            0x008B      // Media Select DVD
#define HID_CCC_MEDIA_SELECT_TELEPHONE                                      0x008C      // Media Select Telephone
#define HID_CCC_MEDIA_SELECT_PROGRAM_GUIDE                                  0x008D      // Media Select Program Guide
#define HID_CCC_MEDIA_SELECT_VIDEO_PHONE                                    0x008E      // Media Select Video Phone
#define HID_CCC_MEDIA_SELECT_GAMES                                          0x008F      // Media Select Games
#define HID_CCC_MEDIA_SELECT_MESSAGES                                       0x0090      // Media Select Messages
#define HID_CCC_MEDIA_SELECT_CD                                             0x0091      // Media Select CD
#define HID_CCC_MEDIA_SELECT_VCR                                            0x0092      // Media Select VCR
#define HID_CCC_MEDIA_SELECT_TUNER                                          0x0093      // Media Select Tuner
#define HID_CCC_QUIT                                                        0x0094      // Quit
#define HID_CCC_HELP                                                        0x0095      // Help
#define HID_CCC_MEDIA_SELECT_TAPE                                           0x0096      // Media Select Tape
#define HID_CCC_MEDIA_SELECT_CABLE                                          0x0097      // Media Select Cable
#define HID_CCC_MEDIA_SELECT_SATELLITE                                      0x0098      // Media Select Satellite
#define HID_CCC_MEDIA_SELECT_SECURITY                                       0x0099      // Media Select Security
#define HID_CCC_MEDIA_SELECT_HOME                                           0x009A      // Media Select Home
#define HID_CCC_MEDIA_SELECT_CALL                                           0x009B      // Media Select Call
#define HID_CCC_CHANNEL_INCREMENT                                           0x009C      // Channel Increment
#define HID_CCC_CHANNEL_DECREMENT                                           0x009D      // Channel Decrement
#define HID_CCC_MEDIA_SELECT_SAP                                            0x009E      // Media Select SAP

#define HID_CCC_VCR_PLUS                                                    0x00A0      // VCR Plus
#define HID_CCC_ONCE                                                        0x00A1      // Once
#define HID_CCC_DAILY                                                       0x00A2      // Daily
#define HID_CCC_WEEKLY                                                      0x00A3      // Weekly
#define HID_CCC_MONTHLY                                                     0x00A4      // Monthly

#define HID_CCC_PLAY                                                        0x00B0      // Play
#define HID_CCC_PAUSE                                                       0x00B1      // Pause
#define HID_CCC_RECORD                                                      0x00B2      // Record
#define HID_CCC_FAST_FORWARD                                                0x00B3      // Fast Forward
#define HID_CCC_REWIND                                                      0x00B4      // Rewind
#define HID_CCC_SCAN_NEXT_TRACK                                             0x00B5      // Scan Next Track
#define HID_CCC_SCAN_PREVIOUS_TRACK                                         0x00B6      // Scan Previous Track
#define HID_CCC_STOP                                                        0x00B7      // Stop
#define HID_CCC_EJECT                                                       0x00B8      // Eject
#define HID_CCC_RANDOM_PLAY                                                 0x00B9      // Random Play
#define HID_CCC_SELECT_DISC                                                 0x00BA      // Select Disc
#define HID_CCC_ENTER_DISC                                                  0x00BB      // Enter Disc
#define HID_CCC_REPEAT                                                      0x00BC      // Repeat
#define HID_CCC_TRACKING                                                    0x00BD      // Tracking
#define HID_CCC_TRACK_NORMAL                                                0x00BE      // Track Normal
#define HID_CCC_SLOW_TRACKING                                               0x00BF      // Slow Tracking
#define HID_CCC_FRAME_FORWARD                                               0x00C0      // Frame Forward
#define HID_CCC_FRAME_BACK                                                  0x00C1      // Frame Back
#define HID_CCC_MARK                                                        0x00C2      // Mark
#define HID_CCC_CLEAR_MARK                                                  0x00C3      // Clear Mark
#define HID_CCC_REPEAT_FROM_MARK                                            0x00C4      // Repeat From Mark
#define HID_CCC_RETURN_TO_MARK                                              0x00C5      // Return To Mark
#define HID_CCC_SEARCH_MARK_FORWARD                                         0x00C6      // Search Mark Forward
#define HID_CCC_SEARCH_MARK_BACKWARDS                                       0x00C7      // Search Mark Backwards
#define HID_CCC_COUNTER_RESET                                               0x00C8      // Counter Reset
#define HID_CCC_SHOW_COUNTER                                                0x00C9      // Show Counter
#define HID_CCC_TRACKING_INCREMENT                                          0x00CA      // Tracking Increment
#define HID_CCC_TRACKING_DECREMENT                                          0x00CB      // Tracking Decrement
#define HID_CCC_STOP_OR_EJECT                                               0x00CC      // Stop/Eject
#define HID_CCC_PLAY_OR_PAUSE                                               0x00CD      // Play/Pause
#define HID_CCC_PLAY_OR_SKIP                                                0x00CE      // Play/Skip
#define HID_CCC_VOICE_COMMAND                                               0x00CF      // Voice Command
#define HID_CCC_INVOKE_CAPTURE_INTERFACE                                    0x00D0      // Invoke Capture Interface
#define HID_CCC_START_OR_STOP_GAME_RECORDING                                0x00D1      // Start or Stop Game Recording
#define HID_CCC_HISTORICAL_GAME_CAPTURE                                     0x00D2      // Historical Game Capture
#define HID_CCC_CAPTURE_GAME_SCREENSHOT                                     0x00D3      // Capture Game Screenshot
#define HID_CCC_SHOW_OR_HIDE_RECORDING_INDICATOR                            0x00D4      // Show or Hide Recording Indicator
#define HID_CCC_START_OR_STOP_MICROPHONE_CAPTURE                            0x00D5      // Start or Stop Microphone Capture
#define HID_CCC_START_OR_STOP_CAMERA_CAPTURE                                0x00D6      // Start or Stop Camera Capture
#define HID_CCC_START_OR_STOP_GAME_BROADCAST                                0x00D7      // Start or Stop Game Broadcast
#define HID_CCC_START_OR_STOP_VOICE_DICTATION_SESSION                       0x00D8      // Start or Stop Voice Dictation Session
#define HID_CCC_INVOKE_OR_DISMISS_EMOJI_PICKER                              0x00D9      // Invoke/Dismiss Emoji Picker

#define HID_CCC_VOLUME                                                      0x00E0      // Volume
#define HID_CCC_BALANCE                                                     0x00E1      // Balance
#define HID_CCC_MUTE                                                        0x00E2      // Mute
#define HID_CCC_BASS                                                        0x00E3      // Bass
#define HID_CCC_TREBLE                                                      0x00E4      // Treble
#define HID_CCC_BASS_BOOST                                                  0x00E5      // Bass Boost
#define HID_CCC_SURROUND_MODE                                               0x00E6      // Surround Mode
#define HID_CCC_LOUDNESS                                                    0x00E7      // Loudness
#define HID_CCC_MPX                                                         0x00E8      // MPX
#define HID_CCC_VOLUME_INCREMENT                                            0x00E9      // Volume Increment
#define HID_CCC_VOLUME_DECREMENT                                            0x00EA      // Volume Decrement

#define HID_CCC_SPEED_SELECT                                                0x00F0      // Speed Select
#define HID_CCC_PLAYBACK_SPEED                                              0x00F1      // Playback Speed
#define HID_CCC_STANDARD_PLAY                                               0x00F2      // Standard Play
#define HID_CCC_LONG_PLAY                                                   0x00F3      // Long Play
#define HID_CCC_EXTENDED_PLAY                                               0x00F4      // Extended Play
#define HID_CCC_SLOW                                                        0x00F5      // Slow

#define HID_CCC_FAN_ENABLE                                                  0x0100      // Fan Enable
#define HID_CCC_FAN_SPEED                                                   0x0101      // Fan Speed
#define HID_CCC_LIGHT_ENABLE                                                0x0102      // Light Enable
#define HID_CCC_LIGHT_ILLUMINATION_LEVEL                                    0x0103      // Light Illumination Level
#define HID_CCC_CLIMATE_CONTROL_ENABLE                                      0x0104      // Climate Control Enable
#define HID_CCC_ROOM_TEMPERATURE                                            0x0105      // Room Temperature
#define HID_CCC_SECURITY_ENABLE                                             0x0106      // Security Enable
#define HID_CCC_FIRE_ALARM                                                  0x0107      // Fire Alarm
#define HID_CCC_POLICE_ALARM                                                0x0108      // Police Alarm
#define HID_CCC_PROXIMITY                                                   0x0109      // Proximity
#define HID_CCC_MOTION                                                      0x010A      // Motion
#define HID_CCC_DURESS_ALARM                                                0x010B      // Duress Alarm
#define HID_CCC_HOLDUP_ALARM                                                0x010C      // Holdup Alarm
#define HID_CCC_MEDICAL_ALARM                                               0x010D      // Medical Alarm

#define HID_CCC_BALANCE_RIGHT                                               0x0150      // Balance Right
#define HID_CCC_BALANCE_LEFT                                                0x0151      // Balance Left
#define HID_CCC_BASS_INCREMENT                                              0x0152      // Bass Increment
#define HID_CCC_BASS_DECREMENT                                              0x0153      // Bass Decrement
#define HID_CCC_TREBLE_INCREMENT                                            0x0154      // Treble Increment
#define HID_CCC_TREBLE_DECREMENT                                            0x0155      // Treble Decrement

#define HID_CCC_SPEAKER_SYSTEM                                              0x0160      // Speaker System
#define HID_CCC_CHANNEL_LEFT                                                0x0161      // Channel Left
#define HID_CCC_CHANNEL_RIGHT                                               0x0162      // Channel Right
#define HID_CCC_CHANNEL_CENTER                                              0x0163      // Channel Center
#define HID_CCC_CHANNEL_FRONT                                               0x0164      // Channel Front
#define HID_CCC_CHANNEL_CENTER_FRONT                                        0x0165      // Channel Center Front
#define HID_CCC_CHANNEL_SIDE                                                0x0166      // Channel Side
#define HID_CCC_CHANNEL_SURROUND                                            0x0167      // Channel Surround
#define HID_CCC_CHANNEL_LOW_FREQUENCY_ENHANCEMENT                           0x0168      // Channel Low Frequency Enhancement
#define HID_CCC_CHANNEL_TOP                                                 0x0169      // Channel Top
#define HID_CCC_CHANNEL_UNKNOWN                                             0x016A      // Channel Unknown

#define HID_CCC_SUB_CHANNEL                                                 0x0170      // Sub-channel
#define HID_CCC_SUB_CHANNEL_INCREMENT                                       0x0171      // Sub-channel Increment
#define HID_CCC_SUB_CHANNEL_DECREMENT                                       0x0172      // Sub-channel Decrement
#define HID_CCC_ALTERNATE_AUDIO_INCREMENT                                   0x0173      // Alternate Audio Increment
#define HID_CCC_ALTERNATE_AUDIO_DECREMENT                                   0x0174      // Alternate Audio Decrement

#define HID_CCC_APPLICATION_LAUNCH_BUTTONS                                  0x0180      // Application Launch Buttons
#define HID_CCC_AL_LAUNCH_BUTTON_CONFIGURATION_TOOL                         0x0181      // AL Launch Button Configuration Tool
#define HID_CCC_AL_PROGRAMMABLE_BUTTON_CONFIGURATION                        0x0182      // AL Programmable Button Configuration
#define HID_CCC_AL_CONSUMER_CONTROL_CONFIGURATION                           0x0183      // AL Consumer Control Configuration
#define HID_CCC_AL_WORD_PROCESSOR                                           0x0184      // AL Word Processor
#define HID_CCC_AL_TEXT_EDITOR                                              0x0185      // AL Text Editor
#define HID_CCC_AL_SPREADSHEET                                              0x0186      // AL Spreadsheet
#define HID_CCC_AL_GRAPHICS_EDITOR                                          0x0187      // AL Graphics Editor
#define HID_CCC_AL_PRESENTATION_APP                                         0x0188      // AL Presentation App
#define HID_CCC_AL_DATABASE_APP                                             0x0189      // AL Database App
#define HID_CCC_AL_EMAIL_READER                                             0x018A      // AL Email Reader
#define HID_CCC_AL_NEWSREADER                                               0x018B      // AL Newsreader
#define HID_CCC_AL_VOICEMAIL                                                0x018C      // AL Voicemail
#define HID_CCC_AL_CONTACTS_OR_ADDRESS_BOOK                                 0x018D      // AL Contacts/Address Book
#define HID_CCC_AL_CALENDAR_OR_SCHEDULE                                     0x018E      // AL Calendar/Schedule
#define HID_CCC_AL_TASK_OR_PROJECT_MANAGER                                  0x018F      // AL Task/Project Manager
#define HID_CCC_AL_LOG_OR_JOURNAL_OR_TIMECARD                               0x0190      // AL Log/Journal/Timecard
#define HID_CCC_AL_CHECKBOOK_OR_FINANCE                                     0x0191      // AL Checkbook/Finance
#define HID_CCC_AL_CALCULATOR                                               0x0192      // AL Calculator
#define HID_CCC_AL_A_OR_V_CAPTURE_OR_PLAYBACK                               0x0193      // AL A/V Capture/Playback
#define HID_CCC_AL_LOCAL_MACHINE_BROWSER                                    0x0194      // AL Local Machine Browser
#define HID_CCC_AL_LAN_OR_WAN_BROWSER                                       0x0195      // AL LAN/WAN Browser
#define HID_CCC_AL_INTERNET_BROWSER                                         0x0196      // AL Internet Browser
#define HID_CCC_AL_REMOTE_NETWORKING_OR_ISP_CONNECT                         0x0197      // AL Remote Networking/ISP Connect
#define HID_CCC_AL_NETWORK_CONFERENCE                                       0x0198      // AL Network Conference
#define HID_CCC_AL_NETWORK_CHAT                                             0x0199      // AL Network Chat
#define HID_CCC_AL_TELEPHONY_OR_DIALER                                      0x019A      // AL Telephony/Dialer
#define HID_CCC_AL_LOGON                                                    0x019B      // AL Logon
#define HID_CCC_AL_LOGOFF                                                   0x019C      // AL Logoff
#define HID_CCC_AL_LOGON_OR_LOGOFF                                          0x019D      // AL Logon/Logoff
#define HID_CCC_AL_TERMINAL_LOCK_OR_SCREENSAVER                             0x019E      // AL Terminal Lock/Screensaver
#define HID_CCC_AL_CONTROL_PANEL                                            0x019F      // AL Control Panel
#define HID_CCC_AL_COMMAND_LINE_PROCESSOR_OR_RUN                            0x01A0      // AL Command Line Processor/Run
#define HID_CCC_AL_PROCESS_OR_TASK_MANAGER                                  0x01A1      // AL Process/Task Manager
#define HID_CCC_AL_SELECT_TASK_OR_APPLICATION                               0x01A2      // AL Select Task/Application
#define HID_CCC_AL_NEXT_TASK_OR_APPLICATION                                 0x01A3      // AL Next Task/Application
#define HID_CCC_AL_PREVIOUS_TASK_OR_APPLICATION                             0x01A4      // AL Previous Task/Application
#define HID_CCC_AL_PREEMPTIVE_HALT_TASK_OR_APPLICATION                      0x01A5      // AL Preemptive Halt Task/Application
#define HID_CCC_AL_INTEGRATED_HELP_CENTER                                   0x01A6      // AL Integrated Help Center
#define HID_CCC_AL_DOCUMENTS                                                0x01A7      // AL Documents
#define HID_CCC_AL_THESAURUS                                                0x01A8      // AL Thesaurus
#define HID_CCC_AL_DICTIONARY                                               0x01A9      // AL Dictionary
#define HID_CCC_AL_DESKTOP                                                  0x01AA      // AL Desktop
#define HID_CCC_AL_SPELL_CHECK                                              0x01AB      // AL Spell Check
#define HID_CCC_AL_GRAMMAR_CHECK                                            0x01AC      // AL Grammar Check
#define HID_CCC_AL_WIRELESS_STATUS                                          0x01AD      // AL Wireless Status
#define HID_CCC_AL_KEYBOARD_LAYOUT                                          0x01AE      // AL Keyboard Layout
#define HID_CCC_AL_VIRUS_PROTECTION                                         0x01AF      // AL Virus Protection
#define HID_CCC_AL_ENCRYPTION                                               0x01B0      // AL Encryption
#define HID_CCC_AL_SCREEN_SAVER                                             0x01B1      // AL Screen Saver
#define HID_CCC_AL_ALARMS                                                   0x01B2      // AL Alarms
#define HID_CCC_AL_CLOCK                                                    0x01B3      // AL Clock
#define HID_CCC_AL_FILE_BROWSER                                             0x01B4      // AL File Browser
#define HID_CCC_AL_POWER_STATUS                                             0x01B5      // AL Power Status
#define HID_CCC_AL_IMAGE_BROWSER                                            0x01B6      // AL Image Browser
#define HID_CCC_AL_AUDIO_BROWSER                                            0x01B7      // AL Audio Browser
#define HID_CCC_AL_MOVIE_BROWSER                                            0x01B8      // AL Movie Browser
#define HID_CCC_AL_DIGITAL_RIGHTS_MANAGER                                   0x01B9      // AL Digital Rights Manager
#define HID_CCC_AL_DIGITAL_WALLET                                           0x01BA      // AL Digital Wallet

#define HID_CCC_AL_INSTANT_MESSAGING                                        0x01BC      // AL Instant Messaging
#define HID_CCC_AL_OEM_FEATURES_OR__TIPS_OR_TUTORIAL_BROWSER                0x01BD      // AL OEM Features/ Tips/Tutorial Browser
#define HID_CCC_AL_OEM_HELP                                                 0x01BE      // AL OEM Help
#define HID_CCC_AL_ONLINE_COMMUNITY                                         0x01BF      // AL Online Community
#define HID_CCC_AL_ENTERTAINMENT_CONTENT_BROWSER                            0x01C0      // AL Entertainment Content Browser
#define HID_CCC_AL_ONLINE_SHOPPING_BROWSER                                  0x01C1      // AL Online Shopping Browser
#define HID_CCC_AL_SMARTCARD_INFORMATION_OR_HELP                            0x01C2      // AL SmartCard Information/Help
#define HID_CCC_AL_MARKET_MONITOR_OR_FINANCE_BROWSER                        0x01C3      // AL Market Monitor/Finance Browser
#define HID_CCC_AL_CUSTOMIZED_CORPORATE_NEWS_BROWSER                        0x01C4      // AL Customized Corporate News Browser
#define HID_CCC_AL_ONLINE_ACTIVITY_BROWSER                                  0x01C5      // AL Online Activity Browser
#define HID_CCC_AL_RESEARCH_OR_SEARCH_BROWSER                               0x01C6      // AL Research/Search Browser
#define HID_CCC_AL_AUDIO_PLAYER                                             0x01C7      // AL Audio Player
#define HID_CCC_AL_MESSAGE_STATUS                                           0x01C8      // AL Message Status
#define HID_CCC_AL_CONTACT_SYNC                                             0x01C9      // AL Contact Sync
#define HID_CCC_AL_NAVIGATION                                               0x01CA      // AL Navigation
#define HID_CCC_AL_CONTEXT_AWARE_DESKTOP_ASSISTANT                          0x01CB      // AL Context-aware Desktop Assistant

#define HID_CCC_GENERIC_GUI_APPLICATION_CONTROLS                            0x0200      // Generic GUI Application Controls
#define HID_CCC_AC_NEW                                                      0x0201      // AC New
#define HID_CCC_AC_OPEN                                                     0x0202      // AC Open
#define HID_CCC_AC_CLOSE                                                    0x0203      // AC Close
#define HID_CCC_AC_EXIT                                                     0x0204      // AC Exit
#define HID_CCC_AC_MAXIMIZE                                                 0x0205      // AC Maximize
#define HID_CCC_AC_MINIMIZE                                                 0x0206      // AC Minimize
#define HID_CCC_AC_SAVE                                                     0x0207      // AC Save
#define HID_CCC_AC_PRINT                                                    0x0208      // AC Print
#define HID_CCC_AC_PROPERTIES                                               0x0209      // AC Properties

#define HID_CCC_AC_UNDO                                                     0x021A      // AC Undo
#define HID_CCC_AC_COPY                                                     0x021B      // AC Copy
#define HID_CCC_AC_CUT                                                      0x021C      // AC Cut
#define HID_CCC_AC_PASTE                                                    0x021D      // AC Paste
#define HID_CCC_AC_SELECT_ALL                                               0x021E      // AC Select All
#define HID_CCC_AC_FIND                                                     0x021F      // AC Find
#define HID_CCC_AC_FIND_AND_REPLACE                                         0x0220      // AC Find and Replace
#define HID_CCC_AC_SEARCH                                                   0x0221      // AC Search
#define HID_CCC_AC_GO_TO                                                    0x0222      // AC Go To
#define HID_CCC_AC_HOME                                                     0x0223      // AC Home
#define HID_CCC_AC_BACK                                                     0x0224      // AC Back
#define HID_CCC_AC_FORWARD                                                  0x0225      // AC Forward
#define HID_CCC_AC_STOP                                                     0x0226      // AC Stop
#define HID_CCC_AC_REFRESH                                                  0x0227      // AC Refresh
#define HID_CCC_AC_PREVIOUS_LINK                                            0x0228      // AC Previous Link
#define HID_CCC_AC_NEXT_LINK                                                0x0229      // AC Next Link
#define HID_CCC_AC_BOOKMARKS                                                0x022A      // AC Bookmarks
#define HID_CCC_AC_HISTORY                                                  0x022B      // AC History
#define HID_CCC_AC_SUBSCRIPTIONS                                            0x022C      // AC Subscriptions
#define HID_CCC_AC_ZOOM_IN                                                  0x022D      // AC Zoom In
#define HID_CCC_AC_ZOOM_OUT                                                 0x022E      // AC Zoom Out
#define HID_CCC_AC_ZOOM                                                     0x022F      // AC Zoom
#define HID_CCC_AC_FULL_SCREEN_VIEW                                         0x0230      // AC Full Screen View
#define HID_CCC_AC_NORMAL_VIEW                                              0x0231      // AC Normal View
#define HID_CCC_AC_VIEW_TOGGLE                                              0x0232      // AC View Toggle
#define HID_CCC_AC_SCROLL_UP                                                0x0233      // AC Scroll Up
#define HID_CCC_AC_SCROLL_DOWN                                              0x0234      // AC Scroll Down
#define HID_CCC_AC_SCROLL                                                   0x0235      // AC Scroll
#define HID_CCC_AC_PAN_LEFT                                                 0x0236      // AC Pan Left
#define HID_CCC_AC_PAN_RIGHT                                                0x0237      // AC Pan Right
#define HID_CCC_AC_PAN                                                      0x0238      // AC Pan
#define HID_CCC_AC_NEW_WINDOW                                               0x0239      // AC New Window
#define HID_CCC_AC_TILE_HORIZONTALLY                                        0x023A      // AC Tile Horizontally
#define HID_CCC_AC_TILE_VERTICALLY                                          0x023B      // AC Tile Vertically
#define HID_CCC_AC_FORMAT                                                   0x023C      // AC Format
#define HID_CCC_AC_EDIT                                                     0x023D      // AC Edit
#define HID_CCC_AC_BOLD                                                     0x023E      // AC Bold
#define HID_CCC_AC_ITALICS                                                  0x023F      // AC Italics
#define HID_CCC_AC_UNDERLINE                                                0x0240      // AC Underline
#define HID_CCC_AC_STRIKETHROUGH                                            0x0241      // AC Strikethrough
#define HID_CCC_AC_SUBSCRIPT                                                0x0242      // AC Subscript
#define HID_CCC_AC_SUPERSCRIPT                                              0x0243      // AC Superscript
#define HID_CCC_AC_ALL_CAPS                                                 0x0244      // AC All Caps
#define HID_CCC_AC_ROTATE                                                   0x0245      // AC Rotate
#define HID_CCC_AC_RESIZE                                                   0x0246      // AC Resize
#define HID_CCC_AC_FLIP_HORIZONTAL                                          0x0247      // AC Flip Horizontal
#define HID_CCC_AC_FLIP_VERTICAL                                            0x0248      // AC Flip Vertical
#define HID_CCC_AC_MIRROR_HORIZONTAL                                        0x0249      // AC Mirror Horizontal
#define HID_CCC_AC_MIRROR_VERTICAL                                          0x024A      // AC Mirror Vertical
#define HID_CCC_AC_FONT_SELECT                                              0x024B      // AC Font Select
#define HID_CCC_AC_FONT_COLOR                                               0x024C      // AC Font Color
#define HID_CCC_AC_FONT_SIZE                                                0x024D      // AC Font Size
#define HID_CCC_AC_JUSTIFY_LEFT                                             0x024E      // AC Justify Left
#define HID_CCC_AC_JUSTIFY_CENTER_H                                         0x024F      // AC Justify Center H
#define HID_CCC_AC_JUSTIFY_RIGHT                                            0x0250      // AC Justify Right
#define HID_CCC_AC_JUSTIFY_BLOCK_H                                          0x0251      // AC Justify Block H
#define HID_CCC_AC_JUSTIFY_TOP                                              0x0252      // AC Justify Top
#define HID_CCC_AC_JUSTIFY_CENTER_V                                         0x0253      // AC Justify Center V
#define HID_CCC_AC_JUSTIFY_BOTTOM                                           0x0254      // AC Justify Bottom
#define HID_CCC_AC_JUSTIFY_BLOCK_V                                          0x0255      // AC Justify Block V
#define HID_CCC_AC_INDENT_DECREASE                                          0x0256      // AC Indent Decrease
#define HID_CCC_AC_INDENT_INCREASE                                          0x0257      // AC Indent Increase
#define HID_CCC_AC_NUMBERED_LIST                                            0x0258      // AC Numbered List
#define HID_CCC_AC_RESTART_NUMBERING                                        0x0259      // AC Restart Numbering
#define HID_CCC_AC_BULLETED_LIST                                            0x025A      // AC Bulleted List
#define HID_CCC_AC_PROMOTE                                                  0x025B      // AC Promote
#define HID_CCC_AC_DEMOTE                                                   0x025C      // AC Demote
#define HID_CCC_AC_YES                                                      0x025D      // AC Yes
#define HID_CCC_AC_NO                                                       0x025E      // AC No
#define HID_CCC_AC_CANCEL                                                   0x025F      // AC Cancel
#define HID_CCC_AC_CATALOG                                                  0x0260      // AC Catalog
#define HID_CCC_AC_BUY_OR_CHECKOUT                                          0x0261      // AC Buy/Checkout
#define HID_CCC_AC_ADD_TO_CART                                              0x0262      // AC Add to Cart
#define HID_CCC_AC_EXPAND                                                   0x0263      // AC Expand
#define HID_CCC_AC_EXPAND_ALL                                               0x0264      // AC Expand All
#define HID_CCC_AC_COLLAPSE                                                 0x0265      // AC Collapse
#define HID_CCC_AC_COLLAPSE_ALL                                             0x0266      // AC Collapse All
#define HID_CCC_AC_PRINT_PREVIEW                                            0x0267      // AC Print Preview
#define HID_CCC_AC_PASTE_SPECIAL                                            0x0268      // AC Paste Special
#define HID_CCC_AC_INSERT_MODE                                              0x0269      // AC Insert Mode
#define HID_CCC_AC_DELETE                                                   0x026A      // AC Delete
#define HID_CCC_AC_LOCK                                                     0x026B      // AC Lock
#define HID_CCC_AC_UNLOCK                                                   0x026C      // AC Unlock
#define HID_CCC_AC_PROTECT                                                  0x026D      // AC Protect
#define HID_CCC_AC_UNPROTECT                                                0x026E      // AC Unprotect
#define HID_CCC_AC_ATTACH_COMMENT                                           0x026F      // AC Attach Comment
#define HID_CCC_AC_DELETE_COMMENT                                           0x0270      // AC Delete Comment
#define HID_CCC_AC_VIEW_COMMENT                                             0x0271      // AC View Comment
#define HID_CCC_AC_SELECT_WORD                                              0x0272      // AC Select Word
#define HID_CCC_AC_SELECT_SENTENCE                                          0x0273      // AC Select Sentence
#define HID_CCC_AC_SELECT_PARAGRAPH                                         0x0274      // AC Select Paragraph
#define HID_CCC_AC_SELECT_COLUMN                                            0x0275      // AC Select Column
#define HID_CCC_AC_SELECT_ROW                                               0x0276      // AC Select Row
#define HID_CCC_AC_SELECT_TABLE                                             0x0277      // AC Select Table
#define HID_CCC_AC_SELECT_OBJECT                                            0x0278      // AC Select Object
#define HID_CCC_AC_REDO_OR_REPEAT                                           0x0279      // AC Redo/Repeat
#define HID_CCC_AC_SORT                                                     0x027A      // AC Sort
#define HID_CCC_AC_SORT_ASCENDING                                           0x027B      // AC Sort Ascending
#define HID_CCC_AC_SORT_DESCENDING                                          0x027C      // AC Sort Descending
#define HID_CCC_AC_FILTER                                                   0x027D      // AC Filter
#define HID_CCC_AC_SET_CLOCK                                                0x027E      // AC Set Clock
#define HID_CCC_AC_VIEW_CLOCK                                               0x027F      // AC View Clock
#define HID_CCC_AC_SELECT_TIME_ZONE                                         0x0280      // AC Select Time Zone
#define HID_CCC_AC_EDIT_TIME_ZONES                                          0x0281      // AC Edit Time Zones
#define HID_CCC_AC_SET_ALARM                                                0x0282      // AC Set Alarm
#define HID_CCC_AC_CLEAR_ALARM                                              0x0283      // AC Clear Alarm
#define HID_CCC_AC_SNOOZE_ALARM                                             0x0284      // AC Snooze Alarm
#define HID_CCC_AC_RESET_ALARM                                              0x0285      // AC Reset Alarm
#define HID_CCC_AC_SYNCHRONIZE                                              0x0286      // AC Synchronize
#define HID_CCC_AC_SEND_OR_RECEIVE                                          0x0287      // AC Send/Receive
#define HID_CCC_AC_SEND_TO                                                  0x0288      // AC Send To
#define HID_CCC_AC_REPLY                                                    0x0289      // AC Reply
#define HID_CCC_AC_REPLY_ALL                                                0x028A      // AC Reply All
#define HID_CCC_AC_FORWARD_MSG                                              0x028B      // AC Forward Msg
#define HID_CCC_AC_SEND                                                     0x028C      // AC Send
#define HID_CCC_AC_ATTACH_FILE                                              0x028D      // AC Attach File
#define HID_CCC_AC_UPLOAD                                                   0x028E      // AC Upload
#define HID_CCC_AC_DOWNLOAD                                                 0x028F      // AC Download (Save Target As)
#define HID_CCC_AC_SET_BORDERS                                              0x0290      // AC Set Borders
#define HID_CCC_AC_INSERT_ROW                                               0x0291      // AC Insert Row
#define HID_CCC_AC_INSERT_COLUMN                                            0x0292      // AC Insert Column
#define HID_CCC_AC_INSERT_FILE                                              0x0293      // AC Insert File
#define HID_CCC_AC_INSERT_PICTURE                                           0x0294      // AC Insert Picture
#define HID_CCC_AC_INSERT_OBJECT                                            0x0295      // AC Insert Object
#define HID_CCC_AC_INSERT_SYMBOL                                            0x0296      // AC Insert Symbol
#define HID_CCC_AC_SAVE_AND_CLOSE                                           0x0297      // AC Save and Close
#define HID_CCC_AC_RENAME                                                   0x0298      // AC Rename
#define HID_CCC_AC_MERGE                                                    0x0299      // AC Merge
#define HID_CCC_AC_SPLIT                                                    0x029A      // AC Split
#define HID_CCC_AC_DISRIBUTE_HORIZONTALLY                                   0x029B      // AC Disribute Horizontally
#define HID_CCC_AC_DISTRIBUTE_VERTICALLY                                    0x029C      // AC Distribute Vertically
#define HID_CCC_AC_NEXT_KEYBOARD_LAYOUT_SELECT                              0x029D      // AC Next Keyboard Layout Select
#define HID_CCC_AC_NAVIGATION_GUIDANCE                                      0x029E      // AC Navigation Guidance
#define HID_CCC_AC_DESKTOP_SHOW_ALL_WINDOWS                                 0x029F      // AC Desktop Show All Windows
#define HID_CCC_AC_SOFT_KEY_LEFT                                            0x02A0      // AC Soft Key Left
#define HID_CCC_AC_SOFT_KEY_RIGHT                                           0x02A1      // AC Soft Key Right
#define HID_CCC_AC_DESKTOP_SHOW_ALL_APPLICATIONS                            0x02A2      // AC Desktop Show All Applications

#define HID_CCC_AC_IDLE_KEEP_ALIVE                                          0x02B0      // AC Idle Keep Alive

#define HID_CCC_EXTENDED_KEYBOARD_ATTRIBUTES_COLLECTION                     0x02C0      // Extended Keyboard Attributes Collection
#define HID_CCC_KEYBOARD_FORM_FACTOR                                        0x02C1      // Keyboard Form Factor
#define HID_CCC_KEYBOARD_KEY_TYPE                                           0x02C2      // Keyboard Key Type
#define HID_CCC_KEYBOARD_PHYSICAL_LAYOUT                                    0x02C3      // Keyboard Physical Layout
#define HID_CCC_VENDOR_SPECIFIC_KEYBOARD_PHYSICAL_LAYOUT                    0x02C4      // Vendor-Specific Keyboard Physical Layout
#define HID_CCC_KEYBOARD_IETF_LANGUAGE_TAG_INDEX                            0x02C5      // Keyboard IETF Language Tag Index
#define HID_CCC_IMPLEMENTED_KEYBOARD_INPUT_ASSIST_CONTROLS                  0x02C6      // Implemented Keyboard Input Assist Controls
#define HID_CCC_KEYBOARD_INPUT_ASSIST_PREVIOUS                              0x02C7      // Keyboard Input Assist Previous
#define HID_CCC_KEYBOARD_INPUT_ASSIST_NEXT                                  0x02C8      // Keyboard Input Assist Next
#define HID_CCC_KEYBOARD_INPUT_ASSIST_PREVIOUS_GROUP                        0x02C9      // Keyboard Input Assist Previous Group
#define HID_CCC_KEYBOARD_INPUT_ASSIST_NEXT_GROUP                            0x02CA      // Keyboard Input Assist Next Group
#define HID_CCC_KEYBOARD_INPUT_ASSIST_ACCEPT                                0x02CB      // Keyboard Input Assist Accept
#define HID_CCC_KEYBOARD_INPUT_ASSIST_CANCEL                                0x02CC      // Keyboard Input Assist Cancel

#define HID_CCC_PRIVACY_SCREEN_TOGGLE                                       0x02D0      // Privacy Screen Toggle
#define HID_CCC_PRIVACY_SCREEN_LEVEL_DECREMENT                              0x02D1      // Privacy Screen Level Decrement
#define HID_CCC_PRIVACY_SCREEN_LEVEL_INCREMENT                              0x02D2      // Privacy Screen Level Increment
#define HID_CCC_PRIVACY_SCREEN_LEVEL_MINIMUM                                0x02D3      // Privacy Screen Level Minimum
#define HID_CCC_PRIVACY_SCREEN_LEVEL_MAXIMUM                                0x02D4      // Privacy Screen Level Maximum

#define HID_CCC_CONTACT_EDITED                                              0x0500      // Contact Edited
#define HID_CCC_CONTACT_ADDED                                               0x0501      // Contact Added
#define HID_CCC_CONTACT_RECORD_ACTIVE                                       0x0502      // Contact Record Active
#define HID_CCC_CONTACT_INDEX                                               0x0503      // Contact Index
#define HID_CCC_CONTACT_NICKNAME                                            0x0504      // Contact Nickname
#define HID_CCC_CONTACT_FIRST_NAME                                          0x0505      // Contact First Name
#define HID_CCC_CONTACT_LAST_NAME                                           0x0506      // Contact Last Name
#define HID_CCC_CONTACT_FULL_NAME                                           0x0507      // Contact Full Name
#define HID_CCC_CONTACT_PHONE_NUMBER_PERSONAL                               0x0508      // Contact Phone Number Personal
#define HID_CCC_CONTACT_PHONE_NUMBER_BUSINESS                               0x0509      // Contact Phone Number Business
#define HID_CCC_CONTACT_PHONE_NUMBER_MOBILE                                 0x050A      // Contact Phone Number Mobile
#define HID_CCC_CONTACT_PHONE_NUMBER_PAGER                                  0x050B      // Contact Phone Number Pager
#define HID_CCC_CONTACT_PHONE_NUMBER_FAX                                    0x050C      // Contact Phone Number Fax
#define HID_CCC_CONTACT_PHONE_NUMBER_OTHER                                  0x050D      // Contact Phone Number Other
#define HID_CCC_CONTACT_EMAIL_PERSONAL                                      0x050E      // Contact Email Personal
#define HID_CCC_CONTACT_EMAIL_BUSINESS                                      0x050F      // Contact Email Business
#define HID_CCC_CONTACT_EMAIL_OTHER                                         0x0510      // Contact Email Other
#define HID_CCC_CONTACT_EMAIL_MAIN                                          0x0511      // Contact Email Main
#define HID_CCC_CONTACT_SPEED_DIAL_NUMBER                                   0x0512      // Contact Speed Dial Number
#define HID_CCC_CONTACT_STATUS_FLAG                                         0x0513      // Contact Status Flag
#define HID_CCC_CONTACT_MISC                                                0x0514      // Contact Misc.

