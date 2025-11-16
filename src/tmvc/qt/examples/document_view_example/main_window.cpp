// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file main_window.cpp
/// Contains implementation of the main_window class.

#include "main_window.hpp"
#include "../../document_view.hpp"
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <memory>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>


main_window::main_window() {
    auto initial_text = L"This is\nsimple\ntext\nbla\nbla\nbla\n\nlast line with very long width";
    assign(text_, initial_text);

    auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setPointSize(14);

    auto doc_view = new tmvc::qt::document_view{doc_};
    setCentralWidget(doc_view);
    doc_view->setFont(fnt);

    auto file_menu = this->menuBar()->addMenu("File");

    auto open_menu = file_menu->addAction("Open...");
    open_menu->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(open_menu, &QAction::triggered, [this] {
        open_file();
    });

    file_menu->addSeparator();

    auto exit_menu = file_menu->addAction("Exit");
    connect(exit_menu, &QAction::triggered, [this] {
        close();
    });
}


void main_window::open_file() {
    auto file_name = QFileDialog::getOpenFileName(this, tr("Select file to open"));
    if (file_name.isEmpty()) {
        return;
    }

    try {
        std::wifstream ifstr{file_name.toStdString(), std::ios::in | std::ios::binary};
        if (!ifstr.is_open()) {
            std::ostringstream msg;
            msg << "Can't open file " << file_name.toStdString() << " for reading";
            throw std::runtime_error{msg.str()};
        }

        // use UTF-8 character converter for system C locale
        std::locale loc("");
        if (loc.name() == "C") {
            loc = std::locale{loc, new std::codecvt_utf8<wchar_t>};
        }

        ifstr.imbue(loc);
        std::wstring wstr;
        while(true) {
            wchar_t ch;
            ifstr.read(&ch, 1);
            if (!ifstr) {
                break;
            }

            wstr.push_back(ch);
        }

        tmvc::assign(text_, wstr);
    }
    catch (std::exception & err) {
        QMessageBox msg(QMessageBox::Critical,
                        QObject::tr("Error"),
                        QObject::tr("Error: %1").arg(QString::fromStdString(err.what())),
                        QMessageBox::Ok);
        msg.exec();
    }
}
