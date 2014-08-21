// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefsimple/simple_handler.h"

#ifdef USE_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#endif
#include <string>
#include "include/wrapper/cef_helpers.h"
#include "include/cef_browser.h"

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();
}

