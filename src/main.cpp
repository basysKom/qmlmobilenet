/**
 * SPDX-FileCopyrightText: 2024 basysKom GmbH
 * SPDX-FileContributor: Berthold Krevert <berthold.krevert@basyskom.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "cocodetectionfilter.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    qmlRegisterType<CocoDetectionFilter>("machine.learning", 1, 0, "CocoDetectionFilter");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
