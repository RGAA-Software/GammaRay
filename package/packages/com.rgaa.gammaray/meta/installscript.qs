/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

     component.addOperation("CreateShortcut", "@TargetDir@/GammaRay.exe", "@StartMenuDir@/GammaRay.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/gr_icon.ico",
            "description=Open GammaRay");

     component.addOperation("CreateShortcut", "@TargetDir@/GammaRay.exe", "@DesktopDir@/GammaRay.lnk", "iconPath=@TargetDir@/gr_icon.ico",
        "workingDirectory=@TargetDir@");

     component.addOperation("CreateShortcut", "@TargetDir@/GammaRayClient.exe", "@StartMenuDir@/GammaRayClient.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/gr_client_icon.ico",
            "description=Open GammaRayClient");

     component.addOperation("CreateShortcut", "@TargetDir@/GammaRayClient.exe", "@DesktopDir@/GammaRayClient.lnk", "iconPath=@TargetDir@/gr_client_icon.ico",
        "workingDirectory=@TargetDir@");
}
