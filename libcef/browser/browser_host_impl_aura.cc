// Copyright (c) 2014 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_host_impl.h"

#include <sys/sysinfo.h>

#include "libcef/browser/context.h"
#include "libcef/browser/window_delegate_view.h"
#include "libcef/browser/thread_util.h"
#include "libcef/browser/cef_platform_data_aura.h"

#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "base/bind.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/renderer_preferences.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "content/public/browser/web_contents.h"

#include "ui/aura/test/test_screen.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/ozone/public/cursor_factory_ozone.h"

#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <linux/fb.h>
#include <sys/ioctl.h>

namespace {

// Returns the number of seconds since system boot.
long GetSystemUptime() {
  struct sysinfo info;
  if (sysinfo(&info) == 0)
    return info.uptime;
  return 0;
}

}  // namespace


ui::PlatformCursor CefBrowserHostImpl::GetPlatformCursor(
    blink::WebCursorInfo::Type type){

  return ui::CursorFactoryOzone::GetInstance()->GetDefaultCursor(type);
}

bool CefBrowserHostImpl::PlatformCreateWindow() {

  CHECK(!platform_);
  /* TODO: Input device enumartion in the DeviceManager fails if we
   * dont allow IO on this thread.
   */
  base::ThreadRestrictions::SetIOAllowed(true);

  struct fb_var_screeninfo fb_var;

  int widht, height;

  int fb_fd =  open("/dev/fb0", O_RDWR);
  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var)) {
    LOG(WARNING) << "failed to get fb var info ( " << errno << "). Using default 640x480 size";
    widht = 640;
    height = 480;
  }
  else {
    widht = fb_var.xres;
    height = fb_var.yres;
  }
  gfx::Size default_window_size(widht, height);

  close(fb_fd);

  aura::TestScreen* screen = aura::TestScreen::Create(gfx::Size());

  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, screen);

  platform_  = new cef::PlatformDataAura(default_window_size);

  window_ =  web_contents_->GetNativeView();
  aura::Window* parent = platform_->host()->window();
  if (!parent->Contains(window_))
    parent->AddChild(window_);

  window_->Show();

  return true;
}

void CefBrowserHostImpl::PlatformCloseWindow() {
    //Fixme: need to implement this
}

void CefBrowserHostImpl::PlatformSizeTo(int width, int height) {
}

void CefBrowserHostImpl::PlatformSetFocus(bool focus) {
  if (!focus)
    return;
  if (web_contents_) {
    // Give logical focus to the RenderWidgetHostViewAura in the views
    // hierarchy. This does not change the native keyboard focus.
    web_contents_->Focus();
  }
}

CefWindowHandle CefBrowserHostImpl::PlatformGetWindowHandle() {
  return window_info_.window;
}

bool CefBrowserHostImpl::PlatformViewText(const std::string& text) {
  CEF_REQUIRE_UIT();

  char buff[] = "/tmp/CEFSourceXXXXXX";
  int fd = mkstemp(buff);

  if (fd == -1)
    return false;

  FILE* srcOutput = fdopen(fd, "w+");
  if (!srcOutput)
    return false;

  if (fputs(text.c_str(), srcOutput) < 0) {
    fclose(srcOutput);
    return false;
  }

  fclose(srcOutput);

  std::string newName(buff);
  newName.append(".txt");
  if (rename(buff, newName.c_str()) != 0)
    return false;

  std::string openCommand("xdg-open ");
  openCommand += newName;

  if (system(openCommand.c_str()) != 0)
    return false;

  return true;
}

void CefBrowserHostImpl::PlatformHandleKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  // TODO(cef): Is something required here to handle shortcut keys?
}

void CefBrowserHostImpl::PlatformRunFileChooser(
    const content::FileChooserParams& params,
    RunFileChooserCallback callback) {
  NOTIMPLEMENTED();
  std::vector<base::FilePath> files;
  callback.Run(files);
}

void CefBrowserHostImpl::PlatformHandleExternalProtocol(const GURL& url) {
}

void CefBrowserHostImpl::PlatformTranslateKeyEvent(
    content::NativeWebKeyboardEvent& result,
    const CefKeyEvent& key_event) {
  NOTIMPLEMENTED();
}

void CefBrowserHostImpl::PlatformTranslateClickEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event,
    MouseButtonType type,
    bool mouseUp, int clickCount) {
  PlatformTranslateMouseEvent(result, mouse_event);

  switch (type) {
  case MBT_LEFT:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonLeft;
    break;
  case MBT_MIDDLE:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonMiddle;
    break;
  case MBT_RIGHT:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonRight;
    break;
  default:
    NOTREACHED();
  }

  result.clickCount = clickCount;
}

void CefBrowserHostImpl::PlatformTranslateMoveEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event,
    bool mouseLeave) {
  PlatformTranslateMouseEvent(result, mouse_event);

  if (!mouseLeave) {
    result.type = blink::WebInputEvent::MouseMove;
    if (mouse_event.modifiers & EVENTFLAG_LEFT_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonLeft;
    else if (mouse_event.modifiers & EVENTFLAG_MIDDLE_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonMiddle;
    else if (mouse_event.modifiers & EVENTFLAG_RIGHT_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonRight;
    else
      result.button = blink::WebMouseEvent::ButtonNone;
  } else {
    result.type = blink::WebInputEvent::MouseLeave;
    result.button = blink::WebMouseEvent::ButtonNone;
  }

  result.clickCount = 0;
}

void CefBrowserHostImpl::PlatformTranslateWheelEvent(
    blink::WebMouseWheelEvent& result,
    const CefMouseEvent& mouse_event,
    int deltaX, int deltaY) {
  result = blink::WebMouseWheelEvent();
  PlatformTranslateMouseEvent(result, mouse_event);

  result.type = blink::WebInputEvent::MouseWheel;

  static const double scrollbarPixelsPerGtkTick = 40.0;
  result.deltaX = deltaX;
  result.deltaY = deltaY;
  result.wheelTicksX = result.deltaX / scrollbarPixelsPerGtkTick;
  result.wheelTicksY = result.deltaY / scrollbarPixelsPerGtkTick;
  result.hasPreciseScrollingDeltas = true;

  // Unless the phase and momentumPhase are passed in as parameters to this
  // function, there is no way to know them
  result.phase = blink::WebMouseWheelEvent::PhaseNone;
  result.momentumPhase = blink::WebMouseWheelEvent::PhaseNone;

  if (mouse_event.modifiers & EVENTFLAG_LEFT_MOUSE_BUTTON)
    result.button = blink::WebMouseEvent::ButtonLeft;
  else if (mouse_event.modifiers & EVENTFLAG_MIDDLE_MOUSE_BUTTON)
    result.button = blink::WebMouseEvent::ButtonMiddle;
  else if (mouse_event.modifiers & EVENTFLAG_RIGHT_MOUSE_BUTTON)
    result.button = blink::WebMouseEvent::ButtonRight;
  else
    result.button = blink::WebMouseEvent::ButtonNone;
}

void CefBrowserHostImpl::PlatformTranslateMouseEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event) {
  // position
  result.x = mouse_event.x;
  result.y = mouse_event.y;
  result.windowX = result.x;
  result.windowY = result.y;
  result.globalX = result.x;
  result.globalY = result.y;

  // TODO(linux): Convert global{X,Y} to screen coordinates.

  // modifiers
  result.modifiers |= TranslateModifiers(mouse_event.modifiers);

  // timestamp
  result.timeStampSeconds = GetSystemUptime();
}
