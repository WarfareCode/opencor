/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// PMR support preferences widget
//==============================================================================

#pragma once

//==============================================================================

#include "preferencesinterface.h"

//==============================================================================

namespace Ui {
    class PmrSupportPreferencesWidget;
}

//==============================================================================

namespace OpenCOR {
namespace PMRSupport {

//==============================================================================

class PmrSupportPreferencesWidget : public Preferences::PreferencesWidget
{
    Q_OBJECT

public:
    explicit PmrSupportPreferencesWidget(QObject *pPluginInstance,
                                         QWidget *pParent);
    ~PmrSupportPreferencesWidget();

    virtual void resetPreferences();
    virtual void savePreferences();

private:
    Ui::PmrSupportPreferencesWidget *mGui;
};

//==============================================================================

}   // namespace PMRSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
