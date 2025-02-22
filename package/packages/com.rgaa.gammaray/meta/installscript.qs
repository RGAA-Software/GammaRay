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
    //installer.setDefaultPageVisible(QInstaller.Introduction, false);

    if (installer.isInstaller()) {


        var result = installer.execute("sc",["query", "GammaRayService"]);
        console.log("===>[sc query GammaRayService] result: ");

        var running = false;
        result.forEach(function(element) {
            console.log("line: ", element);
            if (typeof element !== 'string') {
                return;
            }

            if (element.includes("RUNNING")) {
                running = true;
            }
        });
        console.log("Is GammaRayService running? ", running);

        var targetDir = installer.value("TargetDir");
        console.log("Installation dir: ", targetDir);

        if (running) {
            var answer = QMessageBox.question("diglog_remove_exists", "Uninstall Service", "Do you want to uninstall Service?", QMessageBox.Yes | QMessageBox.No);
            if (answer === QMessageBox.Yes) {
                console.log("User chose to continue installation.");
                var result = installer.execute("sc",["stop", "GammaRayService"]);
                console.log("===> [sc stop GammaRayService] result: ", result);

                result = installer.execute("sc",["delete", "GammaRayService"]);
                console.log("===> [sc delete GammaRayService] result: ", result);


            } else {
                console.log("User chose to cancel installation.");
                answer = QMessageBox.question("diglog_quiting", "Exiting", "Installation will exit.", QMessageBox.Yes);
                gui.clickButton(buttons.CancelButton);
            }
        }

        console.log("===> folder exists, will delete it !", targetDir);
        installer.performOperation("Delete",targetDir + "/maintenancetool.exe");
        installer.performOperation("Delete",targetDir + "/maintenancetool.dat");
        installer.performOperation("Delete",targetDir + "/maintenancetool.ini");
        installer.performOperation("Delete", targetDir);

        installer.installationFinished.connect(function() {
            console.log("===> Install finished.");
            //installer.performOperation("Execute", targetDir + "/GammaRay.exe");
            installer.executeDetached(targetDir + "/GammaRay.exe");
        });

    }
    else if (installer.isUninstaller()) {

    }

    installer.uninstallationStarted.connect(function() {
        console.log("===> UnInstallation started.");
        console.log("User chose to continue installation.");
        var result = installer.execute("sc",["stop", "GammaRayService"]);
        console.log("===> [sc stop GammaRayService] result: ", result);

        result = installer.execute("sc",["delete", "GammaRayService"]);
        console.log("===> [sc delete GammaRayService] result: ", result);

    });
}

Component.prototype.beginInstallation = function()
{
    // call default implementation
    component.beginInstallation();

    console.log("===> Installation Started ===");

}


Component.prototype.createOperations = function()
{

     console.log("===> Create Operations...");

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
