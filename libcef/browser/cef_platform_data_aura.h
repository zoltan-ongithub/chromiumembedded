// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef CEF_LIBCEF_PLATFORM_DATA_AURA_H_
#define CEF_LIBCEF_PLATFORM_DATA_AURA_H_

#include "base/memory/scoped_ptr.h"
#include "ui/aura/window_tree_host.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/client/window_tree_client.h"

/* cef_platform_data_aura.cc
 *
 * Based on content shell's shell_platform_data_aura.h
 *
 * Not all ozone plugins support view and need to be built
 * without toolkit views. We use the create a custom platform
 * data class to handle the aura windows.
 */
namespace cef {

class PlatformDataAura {
 public:
  explicit PlatformDataAura(const gfx::Size& initial_size);
  ~PlatformDataAura();

  void ShowWindow();
  void ResizeWindow(const gfx::Size& size);

  aura::WindowTreeHost* host() { return host_.get(); }

 private:
  scoped_ptr<aura::WindowTreeHost> host_;
  scoped_ptr<aura::client::FocusClient> focus_client_;
  scoped_ptr<aura::client::DefaultCaptureClient> capture_client_;
  scoped_ptr<aura::client::WindowTreeClient> window_tree_client_;
  scoped_ptr<ui::EventHandler> ime_filter_;

  DISALLOW_COPY_AND_ASSIGN(PlatformDataAura);
};

} //namesapce

#endif

