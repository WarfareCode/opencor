/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://gnu.org/licenses>.

*******************************************************************************/

//==============================================================================
// Tree view widget
//==============================================================================

#pragma once

//==============================================================================

#include "commonwidget.h"
#include "coreglobal.h"

//==============================================================================

#include <QTreeView>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT TreeViewWidget : public QTreeView, public CommonWidget
{
    Q_OBJECT

public:
    explicit TreeViewWidget(QWidget *pParent);

    void resizeColumnsToContents();

    void selectItem(int pRow = 0, int pColumn = 0);
    void selectFirstItem();

    bool isEditing() const;

protected:
    void keyPressEvent(QKeyEvent *pEvent) override;
    void mousePressEvent(QMouseEvent *pEvent) override;

    void startDrag(Qt::DropActions pSupportedActions) override;
};

//==============================================================================

} // namespace Core
} // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
