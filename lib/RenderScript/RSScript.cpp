/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bcc/RenderScript/RSScript.h"

#include <cstring>

#include <llvm/ADT/STLExtras.h>

#include "bcc/RenderScript/RSInfo.h"
#include "bcc/Source.h"
#include "bcc/Support/Log.h"

using namespace bcc;

RSScript::SourceDependency::SourceDependency(const std::string &pSourceName,
                                             const uint8_t *pSHA1)
  : mSourceName(pSourceName) {
  ::memcpy(mSHA1, pSHA1, sizeof(mSHA1));
  return;
}

bool RSScript::LinkRuntime(RSScript &pScript) {
  // Using the same context with the source in pScript.
  BCCContext &context = pScript.getSource().getContext();
  Source *libclcore_source = Source::CreateFromFile(context,
                                                    RSInfo::LibCLCorePath);
  if (libclcore_source == NULL) {
    ALOGE("Failed to load Renderscript library '%s' to link!",
          RSInfo::LibCLCorePath);
    return false;
  }

  if (!pScript.getSource().merge(*libclcore_source,
                                 /* pPreserveSource */false)) {
    ALOGE("Failed to link RenderScript library '%s'!", RSInfo::LibCLCorePath);
    delete libclcore_source;
    return false;
  }

  return true;
}

RSScript::RSScript(Source &pSource)
  : Script(pSource), mInfo(NULL), mCompilerVersion(0),
    mOptimizationLevel(kOptLvl3) { }

RSScript::~RSScript() {
  llvm::DeleteContainerPointers(mSourceDependencies);
}

bool RSScript::doReset() {
  mInfo = NULL;
  mCompilerVersion = 0;
  mOptimizationLevel = kOptLvl3;
  llvm::DeleteContainerPointers(mSourceDependencies);
  return true;
}

bool RSScript::addSourceDependency(const std::string &pSourceName,
                                   const uint8_t *pSHA1) {
  SourceDependency *source_dep =
      new (std::nothrow) SourceDependency(pSourceName, pSHA1);
  if (source_dep == NULL) {
    ALOGE("Out of memory when record dependency information of `%s'!",
          pSourceName.c_str());
    return false;
  }

  mSourceDependencies.push_back(source_dep);
  return true;
}
