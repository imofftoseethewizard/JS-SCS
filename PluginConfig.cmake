#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for HostServices
#
#\**********************************************************/

set(PLUGIN_NAME "HostServices")
set(PLUGIN_PREFIX "HS")
set(COMPANY_NAME "Blackwhistle")

# ActiveX constants:
set(FBTYPELIB_NAME HostServicesLib)
set(FBTYPELIB_DESC "HostServices 1.0 Type Library")
set(IFBControl_DESC "HostServices Control Interface")
set(FBControl_DESC "HostServices Control Class")
set(IFBComJavascriptObject_DESC "HostServices IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "HostServices ComJavascriptObject Class")
set(IFBComEventSource_DESC "HostServices IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 19416b69-529f-519f-8deb-ce280da33ae9)
set(IFBControl_GUID fe1829c7-3749-5014-a5d8-d0c740da678f)
set(FBControl_GUID 4bd4c347-31e4-52bd-9e42-3be531c1afb0)
set(IFBComJavascriptObject_GUID a95a2e50-68a3-5e33-af35-43dca0484e38)
set(FBComJavascriptObject_GUID 1f4c0342-cb8c-5e15-b756-d4d48114506a)
set(IFBComEventSource_GUID ba91a968-35c4-5ad7-912b-c9e47481311f)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "Blackwhistle.HostServices")
set(MOZILLA_PLUGINID "blackwhistle.com/HostServices")

# strings
set(FBSTRING_CompanyName "Blackwhistle")
set(FBSTRING_FileDescription "SSL-based services for Javascript.")
set(FBSTRING_PLUGIN_VERSION "1.0.0")
set(FBSTRING_LegalCopyright "Copyright 2011 Blackwhistle")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "HostServices")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "HostServices")
set(FBSTRING_MIMEType "application/x-hostservices")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

# set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 1)
set(FBMAC_USE_COCOA 1)
set(FBMAC_USE_COREGRAPHICS 1)
set(FBMAC_USE_COREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)
